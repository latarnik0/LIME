#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <ncurses.h>
#include <map>
#include <vector>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>

// DATA STRUCTS
struct MEMORY_INFO {
	int tot;
	int av;
};

struct CPU_STATIC_INFO {
	std::string cores;
	std::string vendor;
	std::string cpufamily;
	std::string modelname;
	std::string mhz;
	std::string cachesize;
};



struct CPU_DYNAMIC_INFO {
	long user, nice, system, idle;
        long iowait, irq, softirq;
	float usage;
};

struct PROCESS_INFO {
	int interr;
        int contextSwitches;
	int bootTime;
        int allProcs;
        int runningProcs;
        int blockedProcs;
	long uptimeRaw;
	int uptimeH, uptimeM, uptimeS;
};

struct DISK_INFO {
	std::vector<std::string> disks;
};



// MAIN DATA STRUCT
struct STATE {
	MEMORY_INFO mem;
	CPU_STATIC_INFO cpus;
	CPU_DYNAMIC_INFO prev;
	CPU_DYNAMIC_INFO curr;
	PROCESS_INFO proc;
	DISK_INFO disk;
};



// READ RAM INFO
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
    
    state.curr.usage = ((float)activeDiff / (float)totalDiff)*100.0;
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



// READ DISKS USAGE
void read_disk(STATE &state){
	// to do  
}



// PRINT STATE
void print_res(STATE &state){

	int max_x, max_y;
	getmaxyx(stdscr, max_y, max_x);

	start_color();
        use_default_colors();

        init_pair(1, COLOR_RED, COLOR_WHITE);
        init_pair(2, COLOR_BLUE, COLOR_WHITE);
        init_pair(3, COLOR_WHITE, -1);


//----------------------------------STATIC INFO----------------------------------------------------------
    	attron(COLOR_PAIR(2));
	attron(A_BOLD);
	attron(A_STANDOUT);
	mvprintw(0, 20, "STATIC INFO");
	attroff(COLOR_PAIR(2));
	attroff(A_BOLD);
	attroff(A_STANDOUT);

	attron(COLOR_PAIR(3));
	attron(A_BOLD);
	attron(A_STANDOUT);
	mvprintw(3, 24, "CPU");
	attroff(COLOR_PAIR(3));
	attroff(A_BOLD);
	attroff(A_STANDOUT);


	mvprintw(4, 0, "Vendor"); 
	printw(" : %s ", (state.cpus.vendor).c_str());
        mvprintw(5, 0, "Family");
        printw(" : %s ", (state.cpus.cpufamily).c_str());
        mvprintw(6, 0, "Model Name");
        printw(" : %s ", (state.cpus.modelname).c_str());
        mvprintw(7, 0, "Cores");
        printw(" : %s ", (state.cpus.cores).c_str());
        mvprintw(8, 0, "Clock speed:"); 
        printw(" : %s ", (state.cpus.mhz).c_str());
	printw(" MHz");



	attron(COLOR_PAIR(3));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(10, 20, "Disks Usage");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

	

	
//----------------------------------RESOURCES----------------------------------------------------------------	
	attron(COLOR_PAIR(1));
	attron(A_BOLD);
	attron(A_STANDOUT);
	mvprintw(0, 80, "RESOURCES");
	attroff(COLOR_PAIR(1));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

	attron(COLOR_PAIR(3));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(3, 83, "RAM");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        attroff(A_STANDOUT);


	
    	mvprintw(4, 70, "Total RAM : %.2f", static_cast<float> (state.mem.tot / 1000.0));
	printw(" MB");
    	mvprintw(5, 70, "Available RAM : %.2f", static_cast<float> (state.mem.av / 1000.0));
	printw(" MB");
	mvprintw(6, 70, "Usage : %.2f", static_cast<float> (((state.mem.tot/1000.0) - (state.mem.av/1000.0) ) / (state.mem.tot/1000.0) * 100.0));
	printw("%%");


	attron(COLOR_PAIR(3));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(10, 80, "CPU Usage");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

        mvprintw(11, 70, "All cores: %.2f", state.curr.usage);
        printw(" %%");
	

	mvprintw(0, 170, "Uptime: %02d:%02d:%02d", state.proc.uptimeH, state.proc.uptimeM, state.proc.uptimeS);
	
}


// MAIN	
int main(){
	
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	nodelay(stdscr, TRUE);
	

	STATE state;

	timeout(500);

	while(true){
		read_procs(state);
		read_uptime(state);
		read_mem(state);
		read_cpus(state);
		read_cpud(state);
		read_disk(state);
			
		print_res(state);
		refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		int quit = getch();
		
		if(quit == 'q' || quit == 'Q'){
			break;
		}
	}
	endwin();
	return 0;
}
