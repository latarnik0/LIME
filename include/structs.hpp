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
	unsigned long long user=0, nice=0, system=0, idle=0, iowait=0, irq=0, softirq=0;
	float usage=0.0f;
	float smoothUsage=0.0f;
	unsigned long long prevTotal=0, prevIdle=0;
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
	unsigned long long txPrev=0;
 	unsigned long long rxPrev=0;
	unsigned long long rxDiff=0, txDiff=0;
};

struct SYSINFO {
	std::string os;
	std::string kver;
	std::string hostname;
}; 

struct FileSystemInfo {
	std::string device;
	std::string mountPoint;
	std::string fsType;
	unsigned long long total, used, available; 
    	float usage;
};

struct DISK_INFO {
	std::vector<FileSystemInfo> filesystems;
	unsigned long long prevRead=0, prevWrite=0, readDiff=0, writeDiff=0;
};


// MAIN STRUCTURE
struct STATE {
	MEMORY_INFO mem;
	CPU_STATIC_INFO cpus;
	CPU_DYNAMIC_INFO cpud;
	PROCESS_INFO proc;
	DISK_INFO disks;
	PSAUX psaux;
	NETWORK net;
	SYSINFO sys;
};
