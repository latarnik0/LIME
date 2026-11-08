#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

struct MEMORY_INFO
{
	std::uint64_t totKb = 0;
	std::uint64_t avKb = 0;
	std::uint64_t buffersKb = 0;
	std::uint64_t cachedKb = 0;
	std::uint64_t dirtyKb = 0;
	std::uint64_t shmemKb = 0;
	std::uint64_t commitLimitKb = 0;
	std::uint64_t committedAsKb = 0;
	float usg = 0.0f;
	std::uint64_t swaptKb = 0;
	std::uint64_t swapfKb = 0;
	float swapusg = 0.0f;
};

struct CPU_STATIC_INFO
{
	int logicalCpus = 0;
	int cores = 0;
	std::string vendor;
	std::string cpufamily;
	std::string modelname;
	std::string mhz;
	std::string cachesize;
	std::string governor;
};

struct CPU_CORE_DYNAMIC
{
	unsigned long long prevTotal = 0;
	unsigned long long prevIdle = 0;
	float usage = 0.0f;
	int freqMhz = 0;
};

struct CPU_DYNAMIC_INFO
{
	unsigned long long user = 0, nice = 0, system = 0, idle = 0;
	unsigned long long iowait = 0, irq = 0, softirq = 0;
	float usage = 0.0f;
	float smoothUsage = 0.0f;
	unsigned long long prevTotal = 0, prevIdle = 0, deltaTotal = 0;
	std::vector<CPU_CORE_DYNAMIC> cores;
};

struct LOAD_AVG
{
	float one = 0.0f;
	float five = 0.0f;
	float fifteen = 0.0f;
};

struct PROCESS_INFO
{
	unsigned long long interrupts = 0;
	unsigned long long contextSwitches = 0;
	unsigned long long bootTime = 0;
	int allProcs = 0;
	int runningProcs = 0;
	int blockedProcs = 0;
	long uptimeRaw = 0;
	int uptimeH = 0, uptimeM = 0, uptimeS = 0;
};

struct PSAUX
{
	int kthrd = 0;
	int uthrd = 0;
};

struct NET_IFACE
{
	std::string name;
	unsigned long long rxBytes = 0;
	unsigned long long txBytes = 0;
	unsigned long long prevRx = 0;
	unsigned long long prevTx = 0;
	double rxBps = 0.0;
	double txBps = 0.0;
	int linkSpeedMbps = -1;
};

struct NETWORK
{
	std::vector<NET_IFACE> ifaces;
	double rxBps = 0.0;
	double txBps = 0.0;
};

struct SYSINFO
{
	std::string os;
	std::string kver;
	std::string hostname;
};

struct FileSystemInfo
{
	std::string device;
	std::string mountPoint;
	std::string fsType;
	unsigned long long total = 0;
	unsigned long long used = 0;
	unsigned long long available = 0;
	float usage = 0.0f;
};

struct DISK_DEVICE_IO
{
	std::string device;
	unsigned long long prevRead = 0;
	unsigned long long prevWrite = 0;
	unsigned long long readBytes = 0;
	unsigned long long writeBytes = 0;
	double readBps = 0.0;
	double writeBps = 0.0;
};

struct DISK_INFO
{
	std::vector<FileSystemInfo> filesystems;
	std::vector<DISK_DEVICE_IO> devices;
	double readBps = 0.0;
	double writeBps = 0.0;
};

struct PROCESS
{
	int pid = 0;
	std::string user = "N/A";
	std::string command;
	char RSZ = '?';
	unsigned long long utime = 0;
	unsigned long long stime = 0;
	int priority = 0;
	int nice = 0;
	unsigned long long memSize = 0;
	unsigned long long memResident = 0;
	unsigned long long memShared = 0;
	float cpupercent = 0.0f;
};

enum class SensorKind
{
	Temp,
	Fan
};

struct SENSOR_READING
{
	std::string chip;
	std::string label;
	SensorKind kind = SensorKind::Temp;
	float value = 0.0f;
	float crit = 0.0f;
	bool hasCrit = false;
};

struct GPU_INFO
{
	bool available = false;
	std::string driver;
	std::string pciId;
	std::string card;
	int freqMhz = 0;
	int actFreqMhz = 0;
	int minFreqMhz = 0;
	int maxFreqMhz = 0;
	float usage = 0.0f;
	float memUsedMiB = 0.0f;
	unsigned long long prevRc6Ms = 0;
	bool hasPrevRc6 = false;
};

struct POWER_DOMAIN
{
	std::string name;
	std::string path;
	unsigned long long prevEnergyUj = 0;
	bool hasPrev = false;
	float watts = 0.0f;
};

struct POWER_INFO
{
	bool available = false;
	float packageWatts = 0.0f;
	float coreWatts = 0.0f;
	float uncoreWatts = 0.0f;
	std::vector<POWER_DOMAIN> domains;
};

struct BATTERY_INFO
{
	bool available = false;
	std::string name;
	std::string status;
	std::string technology;
	int capacity = 0;
	float voltageV = 0.0f;
	float currentA = 0.0f;
	float powerW = 0.0f;
	float chargeNowAh = 0.0f;
	float chargeFullAh = 0.0f;
	bool acOnline = false;
};

struct HISTORY
{
	static constexpr std::size_t CAPACITY = 180;
	std::vector<float> cpu;
	std::vector<float> mem;
	std::vector<float> gpu;
	std::vector<float> packagePower;
};

struct CAPABILITIES
{
	bool hasGpu = false;
	bool hasRapl = false;
	bool hasBattery = false;
	bool hasFans = false;
};

struct STATE
{
	MEMORY_INFO mem;
	CPU_STATIC_INFO cpus;
	CPU_DYNAMIC_INFO cpud;
	PROCESS_INFO proc;
	LOAD_AVG load;
	DISK_INFO disks;
	PSAUX psaux;
	NETWORK net;
	SYSINFO sys;
	GPU_INFO gpu;
	POWER_INFO power;
	BATTERY_INFO battery;
	CAPABILITIES caps;
	HISTORY history;
	std::vector<PROCESS> pCurr;
	std::vector<PROCESS> pPrev;
	std::vector<SENSOR_READING> sensors;
	int cpuTemp = 0;
	double sampleDtSec = 1.0;
};
