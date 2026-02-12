#include "lime_core.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>

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
	
	state.mem.usg = ((state.mem.tot - state.mem.av)*100)  / state.mem.tot;
       state.mem.swapusg = ((state.mem.swapt - state.mem.swapf)*100)  / state.mem.swapt;	
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

    std::string line, label;
    int prevActive, prevIdle, currActive, currIdle;
    int activeDiff, totalDiff;
    std::ifstream data("/proc/stat");

    if(!data.is_open()){
	    std::cerr<<"Cannot open file stat (time interval 1)"<<std::endl;
    }

    while(std::getline(data, line)){
	    std::stringstream ss(line);
	    ss >> label;
	    
	    if(label == "cpu"){
		    ss >> state.prev.user >> state.prev.nice>>state.prev.system>>state.prev.idle>>state.prev.iowait>>state.prev.irq>>state.prev.softirq;
		    
		    prevActive = state.prev.user + state.prev.nice +state.prev.system+state.prev.irq+state.prev.softirq;
		    prevIdle = state.prev.idle+state.prev.iowait;
		    
		    break;
	    }
    }
    data.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::ifstream data2("/proc/stat");
    std::string line2, label2;

    if(!data2.is_open()){
    	std::cerr<<"Cannot open file stat (time interval 2)"<<std::endl;
    }
    while(std::getline(data2, line2)){
	    std::stringstream ss2(line2);
	    ss2 >> label2;
	    
	    if(label2 == "cpu"){
		    ss2 >> state.curr.user >> state.curr.nice>>state.curr.system>>state.curr.idle>>state.curr.iowait>>state.curr.irq>>state.curr.softirq;
		    
		    currActive = state.curr.user + state.curr.nice + state.curr.system + state.curr.irq+state.curr.softirq;
		    currIdle = state.curr.idle + state.curr.iowait;
		    break;
	    }
    }
    data2.close();
    
    activeDiff = currActive - prevActive;
    totalDiff = (currIdle - prevIdle)+(currActive - prevActive);
    
    state.curr.usage = ((static_cast<float>(activeDiff)) / (static_cast<float> (totalDiff*100.0)));
	state.curr.usageInt = static_cast<int>(state.curr.usage);
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

// PROCS INFO ps aux
void count_active_ps(STATE &state){
	
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

void read_network(STATE &state){
    std::ifstream data("/proc/net/dev");
    std::string line;
    int ignoreThis;
    unsigned long rxPrev=0, txPrev=0, rxCurr=0, txCurr=0;
    unsigned long rxDiff, txDiff;

    std::getline(data, line);
    std::getline(data, line);

    while(std::getline(data, line)){
            std::stringstream ss(line);
            std::string interfaceName;

            ss >> interfaceName;

            if(interfaceName == "lo:") { continue; }
            else{
		    ss >> rxPrev;
		    state.netprev.rx += rxPrev;

                for(int i=0; i<7; i++){
			ss >> ignoreThis;
                }

                ss >> txPrev;
                state.netprev.tx += txPrev;
            }
    }
  
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::ifstream data2("/proc/net/dev");
    std::string line2;
    int ignoreThis2;

    std::getline(data2, line2);
    std::getline(data2, line2);

    while(std::getline(data2, line2)){
            std::stringstream ss2(line2);
            std::string interfaceName2;

            ss2 >> interfaceName2;

            if(interfaceName2 == "lo:") { continue; }
            else{
		    ss2 >> rxCurr;
		    state.netcur.rx += rxCurr;

                for(int i=0; i<7; i++){
			ss2 >> ignoreThis2;
                }

                ss2 >> txCurr;
                state.netcur.tx += txCurr;
            }
    }
    rxDiff = state.netcur.rx - state.netprev.rx;
    txDiff = state.netcur.tx - state.netprev.tx;

    state.netcur.rxDynamic = rxDiff;
    state.netcur.txDynamic = txDiff;

}



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

void gather_data(STATE &state, std::mutex &m, std::atomic<bool> &run){
	while(run){
		STATE temp_state;
		
		read_cpus(temp_state);
		read_sysinfo(temp_state);
                read_procs(temp_state);
                read_uptime(temp_state);
                read_mem(temp_state);
                read_cpud(temp_state);
                count_active_ps(temp_state);
                read_network(temp_state);
		
		{
			std::lock_guard<std::mutex> lock(m);
			state = temp_state;
		}
	}
}

