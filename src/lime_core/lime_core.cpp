#include "lime_core.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <pwd.h>
#include <regex>
#include <set>
#include <sstream>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

namespace fs = std::filesystem;

namespace {

bool read_first_line(const fs::path &path, std::string &out)
{
	std::ifstream in(path);
	if (!in.is_open())
		return false;
	if (!std::getline(in, out))
		return false;
	return true;
}

bool read_ull(const fs::path &path, unsigned long long &out)
{
	std::ifstream in(path);
	if (!in.is_open())
		return false;
	in >> out;
	return static_cast<bool>(in) || in.eof();
}

bool read_int(const fs::path &path, int &out)
{
	std::ifstream in(path);
	if (!in.is_open())
		return false;
	in >> out;
	return static_cast<bool>(in) || in.eof();
}

bool read_double(const fs::path &path, double &out)
{
	std::ifstream in(path);
	if (!in.is_open())
		return false;
	in >> out;
	return static_cast<bool>(in) || in.eof();
}

std::string trim_copy(std::string s)
{
	while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r'))
		s.pop_back();
	const auto start = s.find_first_not_of(" \t");
	if (start == std::string::npos)
		return {};
	return s.substr(start);
}

template <typename T>
void push_history(std::vector<T> &hist, T value, std::size_t cap)
{
	hist.push_back(value);
	if (hist.size() > cap)
		hist.erase(hist.begin(), hist.begin() + static_cast<std::ptrdiff_t>(hist.size() - cap));
}

fs::path find_intel_drm_card()
{
	const fs::path drmRoot = "/sys/class/drm";
	if (!fs::exists(drmRoot))
		return {};

	for (const auto &entry : fs::directory_iterator(drmRoot))
	{
		const std::string name = entry.path().filename().string();
		if (name.rfind("card", 0) != 0)
			continue;
		if (name.find('-') != std::string::npos)
			continue;

		const fs::path vendorPath = entry.path() / "device" / "vendor";
		std::string vendor;
		if (!read_first_line(vendorPath, vendor))
			continue;
		vendor = trim_copy(vendor);
		if (vendor != "0x8086")
			continue;

		const fs::path driverLink = entry.path() / "device" / "driver";
		if (fs::exists(driverLink))
		{
			std::error_code ec;
			const auto target = fs::read_symlink(driverLink, ec);
			if (!ec)
			{
				const std::string driver = target.filename().string();
				if (driver == "i915" || driver == "xe")
					return entry.path();
			}
		}

		if (fs::exists(entry.path() / "gt_cur_freq_mhz") || fs::exists(entry.path() / "gt"))
			return entry.path();
	}
	return {};
}

} // namespace

bool is_number(const std::string &s)
{
	if (s.empty())
		return false;
	return std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
}

bool is_physical_disk(const std::string &s)
{
	static const std::regex sdCheck("^sd[a-z]+$");
	static const std::regex nvmeCheck("^nvme[0-9]+n[0-9]+$");
	static const std::regex vdCheck("^vd[a-z]+$");
	static const std::regex mmcCheck("^mmcblk[0-9]+$");

	return std::regex_match(s, sdCheck) || std::regex_match(s, nvmeCheck) ||
	       std::regex_match(s, vdCheck) || std::regex_match(s, mmcCheck);
}

void read_cpu_temp(STATE &state)
{
	state.cpuTemp = 0;

	for (const auto &reading : state.sensors)
	{
		if (reading.kind != SensorKind::Temp)
			continue;
		if (reading.chip == "coretemp" &&
		    (reading.label.find("Package") != std::string::npos || reading.label == "temp1"))
		{
			state.cpuTemp = static_cast<int>(std::lround(reading.value));
			return;
		}
	}

	for (const auto &reading : state.sensors)
	{
		if (reading.kind == SensorKind::Temp && reading.chip == "coretemp")
		{
			state.cpuTemp = static_cast<int>(std::lround(reading.value));
			return;
		}
	}

	const fs::path thermalRoot = "/sys/class/thermal";
	if (!fs::exists(thermalRoot))
		return;

	for (const auto &zone : fs::directory_iterator(thermalRoot))
	{
		if (!zone.is_directory())
			continue;
		std::string type;
		if (!read_first_line(zone.path() / "type", type))
			continue;
		if (trim_copy(type) != "x86_pkg_temp")
			continue;
		int milli = 0;
		if (read_int(zone.path() / "temp", milli) && milli > 0)
		{
			state.cpuTemp = milli / 1000;
			return;
		}
	}
}

void read_cpu_freqs(STATE &state)
{
	if (state.cpud.cores.empty() && state.cpus.logicalCpus > 0)
		state.cpud.cores.resize(static_cast<std::size_t>(state.cpus.logicalCpus));

	for (std::size_t i = 0; i < state.cpud.cores.size(); ++i)
	{
		const fs::path path =
		    "/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/scaling_cur_freq";
		int freqKhz = 0;
		if (read_int(path, freqKhz))
			state.cpud.cores[i].freqMhz = freqKhz / 1000;
	}

	if (state.cpus.governor.empty())
	{
		std::string gov;
		if (read_first_line("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", gov))
			state.cpus.governor = trim_copy(gov);
	}
}

void read_sensors(STATE &state)
{
	state.sensors.clear();
	state.caps.hasFans = false;

	const fs::path hwmonBase = "/sys/class/hwmon";
	if (!fs::exists(hwmonBase))
		return;

	for (const auto &entry : fs::directory_iterator(hwmonBase))
	{
		if (!entry.is_directory())
			continue;

		std::string chip;
		if (!read_first_line(entry.path() / "name", chip))
			chip = entry.path().filename().string();
		chip = trim_copy(chip);

		for (const auto &file : fs::directory_iterator(entry.path()))
		{
			const std::string fname = file.path().filename().string();
			std::smatch match;

			static const std::regex tempRe("^temp([0-9]+)_input$");
			static const std::regex fanRe("^fan([0-9]+)_input$");

			if (std::regex_match(fname, match, tempRe))
			{
				int milli = 0;
				if (!read_int(file.path(), milli))
					continue;
				if (milli == 0)
					continue;

				SENSOR_READING reading;
				reading.chip = chip;
				reading.kind = SensorKind::Temp;
				reading.value = milli / 1000.0f;

				const std::string idx = match[1].str();
				std::string label;
				if (read_first_line(entry.path() / ("temp" + idx + "_label"), label))
					reading.label = trim_copy(label);
				else
					reading.label = "temp" + idx;

				int crit = 0;
				if (read_int(entry.path() / ("temp" + idx + "_crit"), crit) && crit > 0)
				{
					reading.crit = crit / 1000.0f;
					reading.hasCrit = true;
				}
				else if (read_int(entry.path() / ("temp" + idx + "_max"), crit) && crit > 0)
				{
					reading.crit = crit / 1000.0f;
					reading.hasCrit = true;
				}

				state.sensors.push_back(reading);
			}
			else if (std::regex_match(fname, match, fanRe))
			{
				int rpm = 0;
				if (!read_int(file.path(), rpm))
					continue;

				SENSOR_READING reading;
				reading.chip = chip;
				reading.kind = SensorKind::Fan;
				reading.value = static_cast<float>(rpm);

				const std::string idx = match[1].str();
				std::string label;
				if (read_first_line(entry.path() / ("fan" + idx + "_label"), label))
					reading.label = trim_copy(label);
				else
					reading.label = "fan" + idx;

				state.sensors.push_back(reading);
				state.caps.hasFans = true;
			}
		}
	}
}

void read_mem(STATE &state)
{
	std::ifstream infoMem("/proc/meminfo");
	if (!infoMem.is_open())
	{
		std::cerr << "Cannot open file (meminfo)\n";
		return;
	}

	std::map<std::string, std::uint64_t> config;
	std::string line;
	while (std::getline(infoMem, line))
	{
		std::stringstream ss(line);
		std::string key;
		std::uint64_t val = 0;
		ss >> key >> val;
		if (!key.empty())
			config[key] = val;
	}

	auto get = [&](const char *k) -> std::uint64_t {
		const auto it = config.find(k);
		return it == config.end() ? 0ULL : it->second;
	};

	state.mem.totKb = get("MemTotal:");
	state.mem.avKb = get("MemAvailable:");
	state.mem.buffersKb = get("Buffers:");
	state.mem.cachedKb = get("Cached:");
	state.mem.dirtyKb = get("Dirty:");
	state.mem.shmemKb = get("Shmem:");
	state.mem.commitLimitKb = get("CommitLimit:");
	state.mem.committedAsKb = get("Committed_AS:");
	state.mem.swaptKb = get("SwapTotal:");
	state.mem.swapfKb = get("SwapFree:");

	if (state.mem.totKb > 0)
		state.mem.usg = static_cast<float>((state.mem.totKb - state.mem.avKb) * 100.0) /
		                static_cast<float>(state.mem.totKb);
	else
		state.mem.usg = 0.0f;

	if (state.mem.swaptKb > 0)
		state.mem.swapusg = static_cast<float>((state.mem.swaptKb - state.mem.swapfKb) * 100.0) /
		                    static_cast<float>(state.mem.swaptKb);
	else
		state.mem.swapusg = 0.0f;
}

void read_cpus(STATE &state)
{
	std::ifstream infoCpu("/proc/cpuinfo");
	if (!infoCpu.is_open())
	{
		std::cerr << "Cannot open file cpuinfo\n";
		return;
	}

	std::map<std::string, std::string> firstCpu;
	int processors = 0;
	std::string line;
	bool inFirst = true;

	while (std::getline(infoCpu, line))
	{
		if (line.empty())
		{
			inFirst = false;
			continue;
		}

		std::stringstream ss(line);
		std::string key, val;
		if (!std::getline(ss, key, ':'))
			continue;
		std::getline(ss, val);
		key = trim_copy(key);
		val = trim_copy(val);

		if (key == "processor")
			++processors;

		if (inFirst && !key.empty())
			firstCpu[key] = val;
	}

	state.cpus.logicalCpus = processors > 0 ? processors : static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
	if (firstCpu.count("cpu cores"))
	{
		try
		{
			state.cpus.cores = std::stoi(firstCpu["cpu cores"]);
		}
		catch (...)
		{
			state.cpus.cores = state.cpus.logicalCpus;
		}
	}
	else
	{
		state.cpus.cores = state.cpus.logicalCpus;
	}

	if (firstCpu.count("vendor_id"))
		state.cpus.vendor = firstCpu["vendor_id"];
	if (firstCpu.count("cpu family"))
		state.cpus.cpufamily = firstCpu["cpu family"];
	if (firstCpu.count("model name"))
		state.cpus.modelname = firstCpu["model name"];
	if (firstCpu.count("cpu MHz"))
		state.cpus.mhz = firstCpu["cpu MHz"];
	if (firstCpu.count("cache size"))
		state.cpus.cachesize = firstCpu["cache size"];

	state.cpud.cores.assign(static_cast<std::size_t>(state.cpus.logicalCpus), CPU_CORE_DYNAMIC{});
}

void read_cpud(STATE &state, double dtSec)
{
	(void)dtSec;
	std::ifstream data("/proc/stat");
	if (!data.is_open())
	{
		std::cerr << "Cannot open file stat\n";
		return;
	}

	std::string line;
	while (std::getline(data, line))
	{
		std::stringstream ss(line);
		std::string label;
		ss >> label;

		auto parseUsage = [&](unsigned long long &prevTotal, unsigned long long &prevIdle,
		                      float &usageOut, unsigned long long *deltaTotalOut) {
			unsigned long long user = 0, nice = 0, system = 0, idle = 0;
			unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0;
			ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

			const unsigned long long currActive = user + nice + system + irq + softirq + steal;
			const unsigned long long currIdle = idle + iowait;
			const unsigned long long currTotal = currActive + currIdle;

			const unsigned long long totalDiff = currTotal - prevTotal;
			const unsigned long long idleDiff = currIdle - prevIdle;
			if (totalDiff > 0)
				usageOut = static_cast<float>(totalDiff - idleDiff) * 100.0f /
				           static_cast<float>(totalDiff);

			if (deltaTotalOut)
				*deltaTotalOut = totalDiff;

			prevTotal = currTotal;
			prevIdle = currIdle;

			if (label == "cpu")
			{
				state.cpud.user = user;
				state.cpud.nice = nice;
				state.cpud.system = system;
				state.cpud.idle = idle;
				state.cpud.iowait = iowait;
				state.cpud.irq = irq;
				state.cpud.softirq = softirq;
			}
		};

		if (label == "cpu")
		{
			parseUsage(state.cpud.prevTotal, state.cpud.prevIdle, state.cpud.usage,
			           &state.cpud.deltaTotal);
			state.cpud.smoothUsage = 0.7f * state.cpud.smoothUsage + 0.3f * state.cpud.usage;
		}
		else if (label.rfind("cpu", 0) == 0 && label.size() > 3 && std::isdigit(label[3]))
		{
			const int idx = std::stoi(label.substr(3));
			if (idx < 0)
				continue;
			if (static_cast<std::size_t>(idx) >= state.cpud.cores.size())
				state.cpud.cores.resize(static_cast<std::size_t>(idx) + 1);
			auto &core = state.cpud.cores[static_cast<std::size_t>(idx)];
			parseUsage(core.prevTotal, core.prevIdle, core.usage, nullptr);
		}
	}
}

void read_loadavg(STATE &state)
{
	std::ifstream in("/proc/loadavg");
	if (!in.is_open())
		return;
	in >> state.load.one >> state.load.five >> state.load.fifteen;
}

void read_stat_extras(STATE &state)
{
	std::ifstream data("/proc/stat");
	if (!data.is_open())
		return;

	std::string line;
	while (std::getline(data, line))
	{
		std::stringstream ss(line);
		std::string label;
		ss >> label;
		if (label == "intr")
		{
			ss >> state.proc.interrupts;
		}
		else if (label == "ctxt")
		{
			ss >> state.proc.contextSwitches;
		}
		else if (label == "btime")
		{
			ss >> state.proc.bootTime;
		}
		else if (label == "processes")
		{
			ss >> state.proc.allProcs;
		}
		else if (label == "procs_running")
		{
			ss >> state.proc.runningProcs;
		}
		else if (label == "procs_blocked")
		{
			ss >> state.proc.blockedProcs;
		}
	}
}

void read_uptime(STATE &state)
{
	std::ifstream infoUptime("/proc/uptime");
	if (!infoUptime.is_open())
	{
		std::cerr << "Cannot open file uptime\n";
		return;
	}

	double up = 0.0;
	infoUptime >> up;
	state.proc.uptimeRaw = static_cast<long>(up);

	long raw = state.proc.uptimeRaw;
	state.proc.uptimeH = static_cast<int>(raw / 3600);
	raw %= 3600;
	state.proc.uptimeM = static_cast<int>(raw / 60);
	state.proc.uptimeS = static_cast<int>(raw % 60);
}

void count_threads(STATE &state)
{
	state.psaux.kthrd = 0;
	state.psaux.uthrd = 0;

	try
	{
		for (const auto &entryP : fs::directory_iterator("/proc"))
		{
			if (!entryP.is_directory())
				continue;

			const std::string nameP = entryP.path().filename().string();
			if (!is_number(nameP))
				continue;

			const fs::path taskPath = entryP.path() / "task";
			if (!fs::exists(taskPath))
				continue;

			try
			{
				for (const auto &entryT : fs::directory_iterator(taskPath))
				{
					std::ifstream cmd(entryT.path() / "cmdline");
					if (!cmd.is_open())
						continue;

					if (cmd.peek() == std::ifstream::traits_type::eof())
						state.psaux.kthrd++;
					else
						state.psaux.uthrd++;
				}
			}
			catch (const fs::filesystem_error &)
			{
				continue;
			}
		}
	}
	catch (const fs::filesystem_error &)
	{
	}
}

void read_network(STATE &state, double dtSec)
{
	if (dtSec <= 0.0)
		dtSec = 1.0;

	std::ifstream data("/proc/net/dev");
	if (!data.is_open())
		return;

	std::unordered_map<std::string, NET_IFACE> prev;
	prev.reserve(state.net.ifaces.size() * 2 + 1);
	for (const auto &iface : state.net.ifaces)
		prev[iface.name] = iface;

	state.net.ifaces.clear();
	state.net.rxBps = 0.0;
	state.net.txBps = 0.0;

	std::string line;
	std::getline(data, line);
	std::getline(data, line);

	while (std::getline(data, line))
	{
		std::stringstream ss(line);
		std::string name;
		ss >> name;
		if (!name.empty() && name.back() == ':')
			name.pop_back();
		if (name.empty() || name == "lo")
			continue;

		unsigned long long rx = 0, tx = 0, ignore = 0;
		ss >> rx;
		for (int i = 0; i < 7; ++i)
			ss >> ignore;
		ss >> tx;

		NET_IFACE iface;
		iface.name = name;
		iface.rxBytes = rx;
		iface.txBytes = tx;

		const auto it = prev.find(name);
		if (it != prev.end())
		{
			iface.prevRx = it->second.rxBytes;
			iface.prevTx = it->second.txBytes;
			iface.rxBps = static_cast<double>(rx - it->second.rxBytes) / dtSec;
			iface.txBps = static_cast<double>(tx - it->second.txBytes) / dtSec;
		}

		int speed = -1;
		if (read_int("/sys/class/net/" + name + "/speed", speed) && speed > 0)
			iface.linkSpeedMbps = speed;

		state.net.rxBps += iface.rxBps;
		state.net.txBps += iface.txBps;
		state.net.ifaces.push_back(iface);
	}
}

void read_sysinfo(STATE &state)
{
	read_first_line("/proc/sys/kernel/ostype", state.sys.os);
	read_first_line("/proc/sys/kernel/osrelease", state.sys.kver);
	read_first_line("/proc/sys/kernel/hostname", state.sys.hostname);
	state.sys.os = trim_copy(state.sys.os);
	state.sys.kver = trim_copy(state.sys.kver);
	state.sys.hostname = trim_copy(state.sys.hostname);
}

void read_disks(STATE &state)
{
	state.disks.filesystems.clear();
	std::ifstream data("/proc/mounts");
	if (!data.is_open())
	{
		std::cerr << "Cannot open file mounts\n";
		return;
	}

	static const std::regex devCheck("^/dev/(sd|nvme|vd|mmcblk|hd).+");
	std::string line;
	std::set<std::string> seenMounts;

	while (std::getline(data, line))
	{
		std::stringstream ss(line);
		FileSystemInfo fsInfo;
		ss >> fsInfo.device >> fsInfo.mountPoint >> fsInfo.fsType;
		if (!std::regex_match(fsInfo.device, devCheck))
			continue;
		if (!seenMounts.insert(fsInfo.mountPoint).second)
			continue;

		struct statvfs stat
		{
		};
		if (statvfs(fsInfo.mountPoint.c_str(), &stat) != 0)
			continue;

		fsInfo.total = static_cast<unsigned long long>(stat.f_blocks) * stat.f_frsize;
		fsInfo.available = static_cast<unsigned long long>(stat.f_bavail) * stat.f_frsize;
		fsInfo.used = fsInfo.total - static_cast<unsigned long long>(stat.f_bfree) * stat.f_frsize;
		if (fsInfo.total > 0)
			fsInfo.usage = static_cast<float>(fsInfo.used) * 100.0f / static_cast<float>(fsInfo.total);
		state.disks.filesystems.push_back(fsInfo);
	}
}

void read_disk_rw(STATE &state, double dtSec)
{
	if (dtSec <= 0.0)
		dtSec = 1.0;

	std::unordered_map<std::string, DISK_DEVICE_IO> prev;
	for (const auto &d : state.disks.devices)
		prev[d.device] = d;

	std::ifstream data("/proc/diskstats");
	if (!data.is_open())
	{
		std::cerr << "Cannot open file diskstats\n";
		return;
	}

	state.disks.devices.clear();
	state.disks.readBps = 0.0;
	state.disks.writeBps = 0.0;

	std::string line;
	while (std::getline(data, line))
	{
		std::stringstream ss(line);
		std::string ignore, device;
		unsigned long long sectorsRead = 0, sectorsWritten = 0;
		ss >> ignore >> ignore >> device;
		if (!is_physical_disk(device))
			continue;

		ss >> ignore >> ignore >> sectorsRead >> ignore >> ignore >> ignore >> sectorsWritten;
		DISK_DEVICE_IO io;
		io.device = device;
		io.readBytes = sectorsRead * 512ULL;
		io.writeBytes = sectorsWritten * 512ULL;

		const auto it = prev.find(device);
		if (it != prev.end())
		{
			io.prevRead = it->second.readBytes;
			io.prevWrite = it->second.writeBytes;
			io.readBps = static_cast<double>(io.readBytes - it->second.readBytes) / dtSec;
			io.writeBps = static_cast<double>(io.writeBytes - it->second.writeBytes) / dtSec;
		}

		state.disks.readBps += io.readBps;
		state.disks.writeBps += io.writeBps;
		state.disks.devices.push_back(io);
	}
}

void read_gpu(STATE &state, double dtSec)
{
	if (dtSec <= 0.0)
		dtSec = 1.0;

	const bool hadPrev = state.gpu.hasPrevRc6;
	const unsigned long long prevRc6 = state.gpu.prevRc6Ms;
	state.gpu = GPU_INFO{};

	const fs::path card = find_intel_drm_card();
	if (card.empty())
	{
		state.caps.hasGpu = false;
		return;
	}

	state.gpu.available = true;
	state.caps.hasGpu = true;
	state.gpu.card = card.filename().string();

	std::string vendor, device;
	read_first_line(card / "device" / "vendor", vendor);
	read_first_line(card / "device" / "device", device);
	state.gpu.pciId = trim_copy(vendor) + ":" + trim_copy(device);

	const fs::path driverLink = card / "device" / "driver";
	if (fs::exists(driverLink))
	{
		std::error_code ec;
		const auto target = fs::read_symlink(driverLink, ec);
		if (!ec)
			state.gpu.driver = target.filename().string();
	}
	if (state.gpu.driver.empty())
		state.gpu.driver = "i915";

	auto readFreq = [&](const fs::path &p, int &out) {
		int v = 0;
		if (read_int(p, v))
			out = v;
	};

	readFreq(card / "gt_cur_freq_mhz", state.gpu.freqMhz);
	readFreq(card / "gt_act_freq_mhz", state.gpu.actFreqMhz);
	readFreq(card / "gt_min_freq_mhz", state.gpu.minFreqMhz);
	readFreq(card / "gt_max_freq_mhz", state.gpu.maxFreqMhz);

	if (state.gpu.freqMhz == 0)
	{
		readFreq(card / "gt" / "gt0" / "rps_cur_freq_mhz", state.gpu.freqMhz);
		readFreq(card / "gt" / "gt0" / "rps_act_freq_mhz", state.gpu.actFreqMhz);
		readFreq(card / "gt" / "gt0" / "rps_min_freq_mhz", state.gpu.minFreqMhz);
		readFreq(card / "gt" / "gt0" / "rps_max_freq_mhz", state.gpu.maxFreqMhz);
	}

	unsigned long long rc6Ms = 0;
	bool gotRc6 = read_ull(card / "power" / "rc6_residency_ms", rc6Ms);
	if (!gotRc6)
		gotRc6 = read_ull(card / "gt" / "gt0" / "rc6_residency_ms", rc6Ms);

	if (gotRc6)
	{
		state.gpu.prevRc6Ms = rc6Ms;
		state.gpu.hasPrevRc6 = true;
		if (hadPrev)
		{
			const double deltaRc6 = static_cast<double>(rc6Ms - prevRc6);
			const double wallMs = dtSec * 1000.0;
			double busy = 100.0 * (1.0 - deltaRc6 / wallMs);
			busy = std::clamp(busy, 0.0, 100.0);
			state.gpu.usage = static_cast<float>(busy);
		}
	}

	// Approximate graphics memory in use from unique DRM clients.
	std::set<int> seenClients;
	double memKiB = 0.0;
	try
	{
		for (const auto &procEntry : fs::directory_iterator("/proc"))
		{
			if (!procEntry.is_directory())
				continue;
			const std::string pid = procEntry.path().filename().string();
			if (!is_number(pid))
				continue;

			const fs::path fdinfoDir = procEntry.path() / "fdinfo";
			if (!fs::exists(fdinfoDir))
				continue;

			try
			{
				for (const auto &fdEntry : fs::directory_iterator(fdinfoDir))
				{
					std::ifstream in(fdEntry.path());
					if (!in.is_open())
						continue;

					bool isDrm = false;
					int clientId = -1;
					double resident = 0.0;
					std::string fline;
					while (std::getline(in, fline))
					{
						if (fline.rfind("drm-driver:", 0) == 0)
							isDrm = true;
						else if (fline.rfind("drm-client-id:", 0) == 0)
						{
							std::stringstream ss(fline.substr(14));
							ss >> clientId;
						}
						else if (fline.rfind("drm-resident-system0:", 0) == 0 ||
						         fline.rfind("drm-resident-memory:", 0) == 0)
						{
							std::stringstream ss(fline.substr(fline.find(':') + 1));
							ss >> resident;
						}
					}

					if (!isDrm || clientId < 0)
						continue;
					if (!seenClients.insert(clientId).second)
						continue;
					memKiB += resident;
				}
			}
			catch (const fs::filesystem_error &)
			{
			}
		}
	}
	catch (const fs::filesystem_error &)
	{
	}

	state.gpu.memUsedMiB = static_cast<float>(memKiB / 1024.0);
}

void read_power(STATE &state, double dtSec)
{
	if (dtSec <= 0.0)
		dtSec = 1.0;

	std::unordered_map<std::string, POWER_DOMAIN> prev;
	for (const auto &d : state.power.domains)
		prev[d.path] = d;

	state.power.domains.clear();
	state.power.available = false;
	state.power.packageWatts = 0.0f;
	state.power.coreWatts = 0.0f;
	state.power.uncoreWatts = 0.0f;
	state.caps.hasRapl = false;

	const fs::path raplRoot = "/sys/class/powercap";
	if (!fs::exists(raplRoot))
		return;

	for (const auto &entry : fs::directory_iterator(raplRoot))
	{
		const std::string name = entry.path().filename().string();
		if (name.rfind("intel-rapl", 0) != 0)
			continue;

		const fs::path energyPath = entry.path() / "energy_uj";
		if (!fs::exists(energyPath))
			continue;

		std::string domainName;
		read_first_line(entry.path() / "name", domainName);
		domainName = trim_copy(domainName);

		unsigned long long energy = 0;
		std::ifstream in(energyPath);
		if (!in.is_open())
			continue;
		in >> energy;
		if (!in && !in.eof())
			continue;

		POWER_DOMAIN domain;
		domain.name = domainName.empty() ? name : domainName;
		domain.path = entry.path().string();
		domain.prevEnergyUj = energy;

		const auto it = prev.find(domain.path);
		if (it != prev.end() && it->second.hasPrev)
		{
			const double deltaUj = static_cast<double>(energy - it->second.prevEnergyUj);
			domain.watts = static_cast<float>((deltaUj / 1'000'000.0) / dtSec);
			domain.hasPrev = true;
			state.power.available = true;
			state.caps.hasRapl = true;
		}
		else
		{
			domain.hasPrev = true; // seed for next sample
		}

		if (domain.name == "package-0" || domain.name.find("package") != std::string::npos)
			state.power.packageWatts = domain.watts;
		else if (domain.name == "core")
			state.power.coreWatts = domain.watts;
		else if (domain.name == "uncore")
			state.power.uncoreWatts = domain.watts;

		state.power.domains.push_back(domain);
	}
}

void read_battery(STATE &state)
{
	state.battery = BATTERY_INFO{};
	state.caps.hasBattery = false;

	const fs::path psy = "/sys/class/power_supply";
	if (!fs::exists(psy))
		return;

	for (const auto &entry : fs::directory_iterator(psy))
	{
		std::string type;
		if (!read_first_line(entry.path() / "type", type))
			continue;
		type = trim_copy(type);

		if (type == "Mains")
		{
			int online = 0;
			if (read_int(entry.path() / "online", online))
				state.battery.acOnline = online != 0;
			continue;
		}

		if (type != "Battery" || state.battery.available)
			continue;

		state.battery.available = true;
		state.caps.hasBattery = true;
		state.battery.name = entry.path().filename().string();

		std::string status, tech;
		if (read_first_line(entry.path() / "status", status))
			state.battery.status = trim_copy(status);
		if (read_first_line(entry.path() / "technology", tech))
			state.battery.technology = trim_copy(tech);

		int capacity = 0;
		if (read_int(entry.path() / "capacity", capacity))
			state.battery.capacity = capacity;

		long long voltage = 0, current = 0, power = 0;
		long long chargeNow = 0, chargeFull = 0;
		std::ifstream(entry.path() / "voltage_now") >> voltage;
		std::ifstream(entry.path() / "current_now") >> current;
		std::ifstream(entry.path() / "power_now") >> power;
		std::ifstream(entry.path() / "charge_now") >> chargeNow;
		std::ifstream(entry.path() / "charge_full") >> chargeFull;

		state.battery.voltageV = static_cast<float>(voltage) / 1'000'000.0f;
		state.battery.currentA = static_cast<float>(std::llabs(current)) / 1'000'000.0f;
		if (power > 0)
			state.battery.powerW = static_cast<float>(power) / 1'000'000.0f;
		else
			state.battery.powerW = state.battery.voltageV * state.battery.currentA;

		state.battery.chargeNowAh = static_cast<float>(chargeNow) / 1'000'000.0f;
		state.battery.chargeFullAh = static_cast<float>(chargeFull) / 1'000'000.0f;
	}
}

void update_history(STATE &state)
{
	push_history(state.history.cpu, state.cpud.smoothUsage, HISTORY::CAPACITY);
	push_history(state.history.mem, state.mem.usg, HISTORY::CAPACITY);
	push_history(state.history.gpu, state.gpu.usage, HISTORY::CAPACITY);
	push_history(state.history.packagePower, state.power.packageWatts, HISTORY::CAPACITY);
}

bool parse_status(const std::string &pidStr, PROCESS &p)
{
	std::ifstream statusData("/proc/" + pidStr + "/status");
	if (!statusData.is_open())
		return false;

	std::string line;
	int ruid = 0;
	while (std::getline(statusData, line))
	{
		std::stringstream ss(line);
		std::string key;
		ss >> key;
		if (key != "Uid:")
			continue;
		ss >> ruid;
		break;
	}

	struct passwd *userInfo = getpwuid(static_cast<uid_t>(ruid));
	if (userInfo != nullptr)
		p.user = userInfo->pw_name;
	else
		p.user = std::to_string(ruid);
	return true;
}

bool parse_stat(const std::string &pidStr, PROCESS &p)
{
	std::ifstream statData("/proc/" + pidStr + "/stat");
	if (!statData.is_open())
		return false;

	std::string line;
	if (!std::getline(statData, line))
		return false;

	const auto commStart = line.find('(');
	const auto commEnd = line.rfind(')');
	if (commStart == std::string::npos || commEnd == std::string::npos || commEnd <= commStart)
		return false;

	std::stringstream ss(line.substr(commEnd + 2));
	std::string ignore;
	ss >> p.RSZ;
	for (int i = 0; i < 10; ++i)
		ss >> ignore;
	ss >> p.utime >> p.stime >> ignore >> ignore >> p.priority >> p.nice;
	return true;
}

bool parse_statm(const std::string &pidStr, PROCESS &p)
{
	std::ifstream statmData("/proc/" + pidStr + "/statm");
	if (!statmData.is_open())
		return false;

	statmData >> p.memSize >> p.memResident >> p.memShared;
	const unsigned long long pageSize = static_cast<unsigned long long>(sysconf(_SC_PAGESIZE));
	p.memSize = (p.memSize * pageSize) / 1024ULL;
	p.memResident = (p.memResident * pageSize) / 1024ULL;
	p.memShared = (p.memShared * pageSize) / 1024ULL;
	return true;
}

bool parse_cmdline(const std::string &pidStr, PROCESS &p)
{
	std::ifstream cmdlineData("/proc/" + pidStr + "/cmdline");
	if (!cmdlineData.is_open())
		return false;

	std::string line;
	if (!std::getline(cmdlineData, line))
	{
		p.command.clear();
		return true;
	}

	for (char &c : line)
	{
		if (c == '\0')
			c = ' ';
	}
	p.command = trim_copy(line);
	return true;
}

void calc_proc_cpu(STATE &state)
{
	if (state.cpud.deltaTotal == 0)
		return;

	const int cores = std::max(1, state.cpus.cores);
	std::unordered_map<int, unsigned long long> prevRuntime;
	prevRuntime.reserve(state.pPrev.size() * 2 + 1);
	for (const auto &prev : state.pPrev)
		prevRuntime[prev.pid] = prev.utime + prev.stime;

	for (auto &curr : state.pCurr)
	{
		const auto it = prevRuntime.find(curr.pid);
		if (it == prevRuntime.end())
			continue;
		const unsigned long long currRuntime = curr.utime + curr.stime;
		const unsigned long long diff = currRuntime - it->second;
		curr.cpupercent = static_cast<float>(diff) * 100.0f * static_cast<float>(cores) /
		                  static_cast<float>(state.cpud.deltaTotal);
	}
}

void update_proc(STATE &state)
{
	state.pPrev = state.pCurr;
	state.pCurr.clear();

	for (const auto &entryP : fs::directory_iterator("/proc"))
	{
		if (!entryP.is_directory())
			continue;
		const std::string pidStr = entryP.path().filename().string();
		if (!is_number(pidStr))
			continue;

		PROCESS p;
		p.pid = std::stoi(pidStr);
		parse_cmdline(pidStr, p);
		parse_stat(pidStr, p);
		parse_status(pidStr, p);
		parse_statm(pidStr, p);
		state.pCurr.push_back(std::move(p));
	}
}

void gather_data(STATE &state, std::mutex &m, std::atomic<bool> &run, int intervalMs)
{
	STATE temp;
	intervalMs = std::max(200, intervalMs);

	read_cpus(temp);
	read_sysinfo(temp);
	read_sensors(temp);
	read_cpu_temp(temp);
	read_cpu_freqs(temp);
	read_battery(temp);

	auto tPrev = std::chrono::steady_clock::now();
	read_cpud(temp, 1.0);
	read_network(temp, 1.0);
	read_disk_rw(temp, 1.0);
	read_gpu(temp, 1.0);
	read_power(temp, 1.0);
	update_proc(temp);

	while (run.load())
	{
		const auto tNow = std::chrono::steady_clock::now();
		const double dt = std::chrono::duration<double>(tNow - tPrev).count();
		tPrev = tNow;
		temp.sampleDtSec = dt > 0.0 ? dt : intervalMs / 1000.0;

		read_uptime(temp);
		read_mem(temp);
		read_loadavg(temp);
		read_stat_extras(temp);
		count_threads(temp);
		read_disks(temp);
		read_sensors(temp);
		read_cpu_temp(temp);
		read_cpu_freqs(temp);
		read_battery(temp);

		update_proc(temp);
		read_cpud(temp, temp.sampleDtSec);
		read_network(temp, temp.sampleDtSec);
		read_disk_rw(temp, temp.sampleDtSec);
		read_gpu(temp, temp.sampleDtSec);
		read_power(temp, temp.sampleDtSec);
		calc_proc_cpu(temp);
		update_history(temp);

		{
			std::lock_guard<std::mutex> lock(m);
			state = temp;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
	}
}
