#pragma once
#include "structs.hpp"
#include <mutex>
#include <atomic>

void read_mem(STATE &state);
void read_cpus(STATE &state);
void read_cpud(STATE &state, double dtSec);
void read_cpu_freqs(STATE &state);
void read_cpu_temp(STATE &state);
void read_sensors(STATE &state);
void read_loadavg(STATE &state);
void read_stat_extras(STATE &state);
void read_uptime(STATE &state);
void count_threads(STATE &state);
void read_network(STATE &state, double dtSec);
void read_sysinfo(STATE &state);
void read_disks(STATE &state);
void read_disk_rw(STATE &state, double dtSec);
void read_gpu(STATE &state, double dtSec);
void read_power(STATE &state, double dtSec);
void read_battery(STATE &state);
void update_history(STATE &state);
void update_proc(STATE &state);
void calc_proc_cpu(STATE &state);
bool is_number(const std::string &s);
bool is_physical_disk(const std::string &s);
void gather_data(STATE &state, std::mutex &m, std::atomic<bool> &run, int intervalMs = 1000);
