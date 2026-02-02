#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <ncurses.h>
#include <map>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>

#define PATH "/home/user/monitorRes/getdata.sh"

// Struktury danych
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

struct PROCESS_INFO {
        int interr;
        int contextSwitches;
        int bootTime;
        int allProcs;
        int runningProcs;
        int blockedProcs;
};


// Główny kontener na dane
struct STATE {
        MEMORY_INFO mem;
        CPU_STATIC_INFO cpus;
};

void script(){
        system(PATH);
}

// Funkcja do zczytywania danych o RAM
void read_mem(STATE &state){

        std::string line;
        std::map<std::string, int> config;
        std::ifstream infoMem("/proc/meminfo");

        if(!infoMem.is_open()){
                std::cerr<<"Blad otwarcia pliku"<<std::endl;
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


// funkcja do zczytywania danych statycznych o CPU
void read_cpus(STATE &state){

        std::string line;
        std::map<std::string, std::string> config;
        std::ifstream infoCpu("/proc/cpuinfo");

        if(!infoCpu.is_open()){
                std::cerr<<"Blad otwarcia pliku"<<std::endl;
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


void read_cpud(STATE &state){
        std::string line;
        std::ifstream data("/home/user/monitorRes/cpud.txt");

        if(!data.is_open()){
                std::cerr<<"Blad otwarcia pliku (cpud)"<<std::endl;
        }

        while(std::getline(data, line)){
                // DO ZROBIENIA
        }

}


void read_procs(STATE &state){
        std::string line;
        std::map<std::string, int> config;
        std::ifstream infoProcs("/home/user/monitorRes/procs.txt");

        if(!infoProcs.is_open()){
                std::cerr<<"Blad otwarcia pliku (processes)"<<std::endl;
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



void print_res(STATE &state){

        int max_x, max_y;
        getmaxyx(stdscr, max_y, max_x);

//----------------------------------STATIC INFO----------------------------------------------------------
        mvprintw(2, 20, "STATIC INFO");
        mvprintw(4, 0, "CPU Vendor: %s", (state.cpus.vendor).c_str());
        mvprintw(5, 0, "CPU Family: %s", (state.cpus.cpufamily).c_str());
        mvprintw(6, 0, "CPU Model Name: %s", (state.cpus.modelname).c_str());
        mvprintw(7, 0, "CPU Cores: %s", (state.cpus.cores).c_str());
        mvprintw(8, 0, "CPU Clock Speed: %d", stoi(state.cpus.mhz));
        printw(" MHz");




//----------------------------------ZASOBY---------------------------------------------------------------------------------------       
        mvprintw(10, 20, "RESOURCES");

        mvprintw(12, 0, "Total RAM: %.2f", static_cast<float> (state.mem.tot / 1000.0));
        printw(" MB");
        mvprintw(13, 0, "Available RAM: %.2f", static_cast<float> (state.mem.av / 1000.0));
        printw(" MB");
        mvprintw(14, 0, "Usage: %.2f", static_cast<float> (((state.mem.tot/1000.0) - (state.mem.av/1000.0) ) / (state.mem.tot/1000.0) * 100.0));
        printw("%%");

        mvprintw(2, 115, "PROCESSES");
        mvprintw(4, 110, "Interruptions (since boot): %d", state.proc.interr);
        mvprintw(5, 110, "Context switches: %d", state.proc.contextSwitches);
        mvprintw(6, 110, "Running processes: %d", state.proc.runningProcs);
        mvprintw(7, 110, "Blocked processes (I/O wait): %d", state.proc.blockedProcs);

}


int main(){
	
	initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);

    while(true){
		STATE state;

        read_mem(state);
        read_cpus(state);

        print_res(state);
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));   
	}	
	return 0;
}
