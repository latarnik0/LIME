#include "lime_core.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>
#include <regex>
#include <sys/statvfs.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>


// READ MEMORY INFO
void read_mem(STATE &state){

	std::string line;
	std::map<std::string, int> config; 
	std::ifstream infoMem("/proc/meminfo");

	if(!infoMem.is_open()){
		std::cerr<<"Cannot open file (meminfo)"<<std::endl;
	}

	while(std::getline(infoMem, line)){
		
		std::stringstream ss(line);
		std::string key;
		int val;
		ss >> key >> val;
		
		if(!key.empty()){ 
			config[key] = val; 
		}	
	}
	if(config.count("MemTotal:")) { state.mem.tot = config["MemTotal:"]; }
	if(config.count("MemAvailable:")) { state.mem.av = config["MemAvailable:"]; }
	if(config.count("SwapTotal:")) { state.mem.swapt = config["SwapTotal:"]; }
	if(config.count("SwapFree:")) { state.mem.swapf = config["SwapFree:"]; }
	
	state.mem.usg = ((state.mem.tot - state.mem.av)*100.0f)  / state.mem.tot;
       	state.mem.swapusg = ((state.mem.swapt - state.mem.swapf)*100.0f)  / state.mem.swapt;	
}


// READ CPU STATIC INFO
void read_cpus(STATE &state){

	std::string line;
	std::map<std::string, std::string> config;
	std::ifstream infoCpu("/proc/cpuinfo");

	if(!infoCpu.is_open()){
		std::cerr<<"Cannot open file cpuinfo"<<std::endl;
	}

	while(std::getline(infoCpu, line)){
		
		std::stringstream ss(line);
		std::string key, val;
		if(!std::getline(ss, key, ':')) continue;

		std::getline(ss, val);

		while(!key.empty() && (key.back() == '\t' || key.back() == ' ')) {
            		key.pop_back();
        }

		size_t first_char = val.find_first_not_of(" \t");

        if(first_char != std::string::npos) {
            	val = val.substr(first_char);
        }
		else{
            	val = "";
        }

		if(!key.empty()){
			config[key] = val;
		}

		if(line.empty() && !config.empty()) break;
	}


	if(config.count("cpu cores")) { state.cpus.cores = config["cpu cores"]; }
    if(config.count("vendor_id")) { state.cpus.vendor = config["vendor_id"]; }
	if(config.count("cpu family")) { state.cpus.cpufamily = config["cpu family"]; }
	if(config.count("model name")) { state.cpus.modelname = config["model name"]; }
    if(config.count("cpu MHz")) { state.cpus.mhz = config["cpu MHz"]; }
	if(config.count("cache size")) { state.cpus.cachesize = config["cache size"]; }
}



// READ CPU USAGE
void read_cpud(STATE &state){
    std::ifstream data("/proc/stat");
    
    if(!data.is_open()){
        std::cerr << "Cannot open file stat" << std::endl;
        return; 
    }

    std::string line, label;

    unsigned long long currActive = 0, currIdle = 0, currTotal = 0;
    unsigned long long user=0, nice=0, system=0, idle=0, iowait=0, irq=0, softirq=0, steal=0;

    while(std::getline(data, line)){
        std::stringstream ss(line);
        ss >> label;
        
        if(label == "cpu"){
            ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
            
            currActive = user + nice + system + irq + softirq + steal;
            currIdle = idle + iowait;
            currTotal = currActive + currIdle;

            break; 
        }
    }
    data.close();

    unsigned long long totalDiff = currTotal - state.cpud.prevTotal;
    unsigned long long idleDiff = currIdle - state.cpud.prevIdle;

    if(totalDiff > 0){
        state.cpud.usage = (static_cast<float>(totalDiff - idleDiff) / static_cast<float>(totalDiff)) * 100.0f;
	state.cpud.smoothUsage = 0.7f * state.cpud.smoothUsage + 0.3f * state.cpud.usage;
    }
    
    state.cpud.prevIdle = currIdle;
    state.cpud.prevTotal = currTotal;
}


// READ UPTIME
void read_uptime(STATE &state){
	std::string line;
	int hours, minutes, seconds;
	std::ifstream infoUptime("/proc/uptime");

	if(!infoUptime.is_open()){
		std::cerr<<"Cannot open file uptime"<<std::endl;
	}

	infoUptime >> line;
	state.proc.uptimeRaw = std::stoi(line);
	
	state.proc.uptimeH = state.proc.uptimeRaw / 3600;
    state.proc.uptimeRaw %= 3600;

    state.proc.uptimeM = state.proc.uptimeRaw / 60;
    state.proc.uptimeS = state.proc.uptimeRaw % 60;
}



// PROCS INFO 1
void read_procs(STATE &state){
	std::string line;
    int hours, minutes, seconds;
	std::map<std::string, int> config;
    std::ifstream infoProcs("/proc/stat");

    if(!infoProcs.is_open()){
            std::cerr<<"Cannot open file stat (processes)"<<std::endl;
    }

    while(std::getline(infoProcs, line)){
        	std::stringstream ss(line);
            std::string key;
            int val;
            ss >> key >> val;

            if(!key.empty()){
                    config[key] = val;
            }
    }
    if(config.count("intr")) { state.proc.interr = config["intr"]; }
    if(config.count("ctxt")) { state.proc.contextSwitches = config["ctxt"]; }
	if(config.count("btime")) { state.proc.bootTime = config["btime"]; }
    if(config.count("processes")) { state.proc.allProcs = config["processes"]; }
	if(config.count("procs_running")) { state.proc.runningProcs = config["procs_running"]; }
    if(config.count("procs_blocked")) { state.proc.blockedProcs = config["procs_blocked"]; }
	
}


bool is_number(const std::string& s){
	if(isdigit(s[0])){
		return true;
	}
	else return false;
}

// COUNT THREADS
void count_threads(STATE &state){
	
	state.psaux.kthrd = 0;
	state.psaux.uthrd = 0;
	try {
                for (const auto& entryP : std::filesystem::directory_iterator("/proc")) {
                        if (!entryP.is_directory()) continue;

                        std::string nameP = entryP.path().filename().string();
                        if(!is_number(nameP)) continue;

                        std::filesystem::path taskPath = std::filesystem::path("/proc") / nameP / "task";

                        if(!std::filesystem::exists(taskPath)) continue;

                        try {

                                for (const auto& entryT : std::filesystem::directory_iterator(taskPath)) {

                                        std::filesystem::path cmdPath = entryT.path() / "cmdline";
                                        std::ifstream cmd(cmdPath);

                                        if(!cmd.is_open()) continue;

                                        if(cmd.peek() == std::ifstream::traits_type::eof()){
                                                state.psaux.kthrd++;
                                        }
                                        else{
                                                state.psaux.uthrd++;
                                        }
                                        cmd.close();
                                }
                        } catch (const std::filesystem::filesystem_error& e) { continue; }
                }
        } catch (const std::filesystem::filesystem_error& e) {
		std::cerr<<"Permission denied: cannot access file"<<std::endl;
        }
}


// READ NETWORK
void read_network(STATE &state){
    std::ifstream data("/proc/net/dev");
    std::string line;
    unsigned long long ignoreThis;
    unsigned long long rxTemp=0, txTemp=0;
    unsigned long long rxCurr=0, txCurr=0;

    std::getline(data, line);
    std::getline(data, line);

    while(std::getline(data, line)){
            std::stringstream ss(line);
            std::string interfaceName;

            ss >> interfaceName;

            if(interfaceName == "lo:") { continue; }
            else{
		    ss >> rxTemp;
		    rxCurr += rxTemp;

                for(int i=0; i<7; i++){
			ss >> ignoreThis;
                }

                ss >> txTemp;
                txCurr += txTemp;
            }
    }
    data.close();
    if(state.net.rxPrev > 0){
    	state.net.rxDiff = rxCurr - state.net.rxPrev;
    	state.net.txDiff = txCurr - state.net.txPrev;
    }

    state.net.rxPrev = rxCurr;
    state.net.txPrev = txCurr;
}


// READ SYSTEM INFO
void read_sysinfo(STATE &state){
	std::ifstream dataOS("/proc/sys/kernel/ostype");
	std::ifstream dataVer("/proc/sys/kernel/osrelease");
	std::ifstream dataHost("/proc/sys/kernel/hostname");
	
	if(!dataOS.is_open()) { std::cerr<<"Cannot open file: ostype"<<std::endl; }
	if(!dataVer.is_open()) { std::cerr<<"Cannot open file: osrelease"<<std::endl; }
	if(!dataHost.is_open()) { std::cerr<<"Cannot open file: hostname"<<std::endl; }
	
	std::getline(dataOS, state.sys.os);
	std::getline(dataVer, state.sys.kver);
	std::getline(dataHost, state.sys.hostname);

}


bool is_physical_disk(std::string &s){
        static const std::regex sdCheck("^sd[a-z]+$");
        static const std::regex nvmeCheck("^nvme[0-9]+n[0-9]+$");
        static const std::regex vdCheck("^vd[a-z]+$");
        static const std::regex mmcCheck("^mmcblk[0-9]+$");

        if(std::regex_match(s, sdCheck) || std::regex_match(s, nvmeCheck) || std::regex_match(s, vdCheck) || std::regex_match(s, mmcCheck)){
                return true;
        }
        else return false;
}



// READ DISKS SPACE INFO
void read_disks(STATE &state){
	state.disks.filesystems.clear();
	std::ifstream data("/proc/mounts");
	std::string line;
	static const std::regex sdCheck("(/dev/sd)(.*)");
        static const std::regex nvmeCheck("(/dev/nvme)(.*)");
        static const std::regex vdCheck("(/dev/vd)(.*)");
        static const std::regex mmcCheck("(/dev/mmcblk)(.*)");
	static const std::regex hdCheck("(/dev/hd)(.*)");

	if(!data.is_open()){
                std::cerr<<"Cannot open file diskstats"<<std::endl;
        }
	
	while(std::getline(data, line)){
		std::stringstream ss(line);
		FileSystemInfo fs;
		ss >> fs.device;
		
		if(std::regex_match(fs.device, sdCheck) || std::regex_match(fs.device, nvmeCheck) || std::regex_match(fs.device, vdCheck) ||
			std::regex_match(fs.device, mmcCheck) || std::regex_match(fs.device, hdCheck)) {

			ss >> fs.mountPoint >> fs.fsType;
			struct statvfs stat;
			
			if(statvfs(fs.mountPoint.c_str(), &stat) == 0) {
            			fs.total = stat.f_blocks * stat.f_frsize;
            			fs.available = stat.f_bavail * stat.f_frsize;
            			fs.used = fs.total - (stat.f_bfree * stat.f_frsize);

            			if(fs.total > 0) {
                			fs.usage = (static_cast<float>(fs.used) / fs.total) * 100.0f;
            			}

            			state.disks.filesystems.push_back(fs);
        		}
		}	

	}	
}


// READ ALL DISKS READ/WRITE INFO
void read_disk_rw(STATE &state){
	std::ifstream data("/proc/diskstats");
	std::string line, device, ignoreThis;
	unsigned long long currWritten=0, currRead=0, totalCurrRead=0, totalCurrWritten=0;
	
	if(!data.is_open()){
		std::cerr<<"Cannot open file diskstats"<<std::endl;
	}

	while(std::getline(data, line)){
		std::stringstream ss(line);
		ss >> ignoreThis >> ignoreThis >> device;
		
		if(is_physical_disk(device)){
			ss >> ignoreThis >> ignoreThis >> currRead >> ignoreThis >> ignoreThis >> ignoreThis >> currWritten;
			totalCurrRead += currRead;
			totalCurrWritten += currWritten;
		}
	}
	totalCurrRead *= 512;
	totalCurrWritten *= 512;
	
	if(state.disks.prevRead > 0){
		state.disks.readDiff = totalCurrRead - state.disks.prevRead;
		state.disks.writeDiff = totalCurrWritten - state.disks.prevWrite;
	}
	state.disks.prevRead = totalCurrRead;
	state.disks.prevWrite = totalCurrWritten;
}


bool parse_status(const std::string& pidStr, PROCESS& p){
	std::ifstream statusData("/proc/" + pidStr + "/status");
	if(!statusData.is_open()) return false;
	
	int ruid;
	std::string uid;
	std::string line;
			
	while(std::getline(statusData, line)){
		std::stringstream ss(line);
		ss >> uid;
				
		if(uid != "Uid:") continue; 
		ss >> ruid;
		break;
	}	
	struct passwd *userInfo = getpwuid(ruid);
			
	if(userInfo != NULL){
		p.user = userInfo->pw_name;
	}
	else{
		p.user = std::to_string(ruid);
	}
	
	return true;
}


bool parse_stat(const std::string& pidStr, PROCESS& p){
	std::ifstream statData("/proc/" + pidStr + "/stat");
	if(!statData.is_open()) return false;

        std::string line;

	while(std::getline(statData, line)){
		std::stringstream ss(line);
		std::string ignorethis;
		ss >> ignorethis >> ignorethis >> p.RSZ >> 
			ignorethis >> ignorethis >> ignorethis >> ignorethis >> ignorethis >> ignorethis >> ignorethis >> ignorethis >> ignorethis >> ignorethis >> 
			p.utime>> p.stime >> ignorethis >> ignorethis >> 
			p.priority >> p.nice;

	}
	return true;
}


bool parse_statm(const std::string& pidStr, PROCESS& p){
 	std::ifstream statmData("/proc/" + pidStr + "/statm");
	if(!statmData.is_open()) return false;

	std::string line;
	
	while(std::getline(statmData, line)){
		std::stringstream ss(line);
		ss >> p.memSize >> p.memResident >> p.memShared;
	}
	unsigned long long pageSize = sysconf(_SC_PAGESIZE);
        p.memSize = (p.memSize * pageSize) / 1024;
        p.memResident = (p.memResident * pageSize) / 1024;
        p.memShared = (p.memShared * pageSize) / 1024;
	
	return true;
}


bool parse_cmdline(const std::string& pidStr, PROCESS& p) {
    std::ifstream cmdlineData("/proc/" + pidStr + "/cmdline");
    if (!cmdlineData.is_open()) return false;

    std::string line;
    if (std::getline(cmdlineData, line)) {
        for (char& c : line) {
            if (c == '\0') {
                c = ' ';
            }
        }
        p.command = line;
    }
    return true;
}


void update_proc(STATE &state) {
	state.pPrev = state.pCurr;
        state.pCurr.clear();
	for (const auto& entryP : std::filesystem::directory_iterator("/proc")) {
        	if (!entryP.is_directory()) continue;

        	std::string pidStr = entryP.path().filename().string();
        	if (!is_number(pidStr)) continue;

       		PROCESS p;
        	p.pid = std::stoi(pidStr);

        	parse_cmdline(pidStr, p);
        	parse_stat(pidStr, p);   
        	parse_status(pidStr, p);
		parse_statm(pidStr, p);

        	state.pCurr.push_back(p);
    }
}


// THREAD
void gather_data(STATE &state, std::mutex &m, std::atomic<bool> &run){
	while(run){
		STATE temp_state;
		
		read_cpud(temp_state); // first call of cpud (so that "prev variables" are not equal to 0)
                read_network(temp_state); // first call of network
		read_disk_rw(temp_state); // first call of read/write 

		read_cpus(temp_state);
		read_sysinfo(temp_state);
                read_procs(temp_state);
                read_uptime(temp_state);
                read_mem(temp_state);
		count_threads(temp_state);
		read_disks(temp_state);
		update_proc(temp_state);

                read_cpud(temp_state); // second call of cpud
                read_network(temp_state); // second call of network
		read_disk_rw(temp_state); // second call of read/write
		
		{
			std::lock_guard<std::mutex> lock(m);
			state = temp_state;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

