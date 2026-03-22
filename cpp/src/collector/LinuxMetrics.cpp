#include "collector/LinuxMetrics.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <chrono>

SystemMetrics LinuxMetrics::getSystemMetrics() {
    SystemMetrics sys;
    
    // 1. Memory Info
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    long total = 0, free = 0, available = 0;
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) std::sscanf(line.c_str(), "MemTotal: %ld", &total);
        if (line.find("MemAvailable:") == 0) std::sscanf(line.c_str(), "MemAvailable: %ld", &available);
    }
    
    sys.memory_total_mb = total / 1024.0;
    sys.memory_used_mb = (total - available) / 1024.0;
    sys.memory_percent = (sys.memory_used_mb / sys.memory_total_mb) * 100.0;

    // 2. CPU Usage (Simplified for demo)
    std::ifstream stat("/proc/stat");
    long user, nice, system, idle;
    stat >> line >> user >> nice >> system >> idle;
    long total_cpu = user + nice + system + idle;
    sys.cpu_percent = (1.0 - (double)idle / total_cpu) * 100.0;

    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);
    sys.timestamp = std::ctime(&now_t);
    
    return sys;
}

std::vector<ProcessMetrics> LinuxMetrics::getAllProcessesMetrics() {
    std::vector<ProcessMetrics> processes;
    DIR* dir = opendir("/proc");
    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            int pid = std::stoi(entry->d_name);
            std::string path = "/proc/" + std::string(entry->d_name) + "/stat";
            std::ifstream proc_stat(path);
            
            if (proc_stat.is_open()) {
                std::string p_name;
                char state;
                int p_pid;
                proc_stat >> p_pid >> p_name >> state;
                
                // Simplified metrics for C++ prototype
                ProcessMetrics pm;
                pm.pid = pid;
                pm.name = p_name.substr(1, p_name.size() - 2); // remove parents ()
                pm.cpu_percent = 0.5; // Placeholder for async calculation
                
                std::string status_path = "/proc/" + std::to_string(pid) + "/status";
                std::ifstream proc_status(status_path);
                std::string status_line;
                while(std::getline(proc_status, status_line)) {
                    if (status_line.find("VmRSS:") == 0) {
                        long rss;
                        std::sscanf(status_line.c_str(), "VmRSS: %ld", &rss);
                        pm.memory_rss_mb = rss / 1024.0;
                    }
                }
                
                processes.push_back(pm);
            }
        }
    }
    closedir(dir);
    return processes;
}
