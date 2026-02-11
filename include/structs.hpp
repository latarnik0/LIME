#pragma once
#include <string>
#include <vector>

struct MEMORY_INFO {
	int tot = 0;
	int av = 0;
	int usg = 0;
	int swapt = 0;
	int swapf = 0;
	int swapusg = 0;
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
	long user=0, nice=0, system=0, idle=0;
    	long iowait=0, irq=0, softirq=0;
	float usage=0;
	int usageInt=0;
};

struct PROCESS_INFO {
	int interr=0;
   	int contextSwitches=0;
	int bootTime=0;
    	int allProcs=0;
    	int runningProcs=0;
    	int blockedProcs=0;
	long uptimeRaw=0;
	int uptimeH=0, uptimeM=0, uptimeS=0;
};

struct PSAUX {
	int kthrd=0;
	int uthrd=0;
	std::string paththrd;
};

struct NETWORK {
	unsigned long tx=0;
	unsigned long rx=0;
};

struct SYSINFO {
	std::string os;
	std::string kver;
	std::string hostname;
}; 

struct DISK_INFO {
	// TODO
};


// MAIN STRUCTURE
struct STATE {
	MEMORY_INFO mem;
	CPU_STATIC_INFO cpus;
	CPU_DYNAMIC_INFO prev;
	CPU_DYNAMIC_INFO curr;
	PROCESS_INFO proc;
	DISK_INFO disk;
	PSAUX psaux;
	NETWORK net;
	SYSINFO sys;
};
