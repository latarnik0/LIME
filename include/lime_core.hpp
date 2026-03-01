#pragma once
#include "structs.hpp" 
#include <mutex>
#include <atomic>

void read_mem(STATE &state);
void read_cpus(STATE &state);
void read_cpud(STATE &state);
void read_uptime(STATE &state);
void read_procs(STATE &state);
void count_threads(STATE &state);
void read_network(STATE &state);
void read_sysinfo(STATE &state);
void read_disks(STATE &state);
void read_disk_rw(STATE &state);
bool parse_status(STATE &state);
bool parse_state(STATE &state);
bool parse_statem(STATE &state);
bool parse_cmdline(STATE &state);
void update_proc(STATE &state);
bool is_number(const std::string& s);
bool is_physical_disk(const std::string& s);
void gather_data(STATE &state, std::mutex &m, std::atomic<bool> &run);
