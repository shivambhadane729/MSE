#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <map>
#include "collector/LinuxMetrics.hpp"
#include "structures/DataStructures.hpp"
#include "features/Window.hpp"
#include "features/Processor.hpp"
#include "ml/MLEngine.hpp"
#include "engine/Scorer.hpp"

#include "structures/SegmentTree.hpp"
#include "structures/AVLTree.hpp"

// Global Window Manager for the session
std::map<int, ProcessWindow> windowManager;

void displayDashboard(const SystemMetrics& sys, const std::vector<ProcessMetrics>& procs, 
                      const std::map<int, std::string>& tiers,
                      const std::map<int, double>& peakMemories) {
    std::cout << "\033[2J\033[1;1H"; // Clear screen
    std::cout << "======================================================================" << std::endl;
    std::cout << "   AMSE C++ CORE ENGINE (LINUX/WSL) - FULL ANALYTICS v1.1             " << std::endl;
    std::cout << "======================================================================" << std::endl;
    std::cout << "CPU: [" << std::fixed << std::setprecision(1) << sys.cpu_percent << "%] | "
              << "RAM: " << sys.memory_percent << "% (" << sys.memory_used_mb << "/" << sys.memory_total_mb << " MB)" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    
    std::cout << std::left << std::setw(8) << "PID" 
              << std::setw(20) << "NAME" 
              << std::setw(15) << "MEM (MB)" 
              << std::setw(12) << "PEAK (MB)"
              << "TIER" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;

    for (size_t i = 0; i < std::min(procs.size(), (size_t)15); ++i) {
        int pid = procs[i].pid;
        std::string tier = tiers.count(pid) ? tiers.at(pid) : "Neutral";
        double peak = peakMemories.count(pid) ? peakMemories.at(pid) : procs[i].memory_rss_mb;
        
        std::string color = "\033[0m"; // Default
        if (tier == "Hot") color = "\033[1;36m"; // Cyan
        if (tier == "Cold") color = "\033[1;90m"; // Gray
        if (tier == "Anomalous") color = "\033[1;31m"; // Red

        std::cout << std::left << std::setw(8) << pid 
                  << std::setw(20) << procs[i].name 
                  << std::setw(15) << procs[i].memory_rss_mb 
                  << std::setw(12) << std::setprecision(1) << peak
                  << color << tier << "\033[0m" << std::endl;
    }
    std::cout << "======================================================================" << std::endl;
    std::cout << "S-TREE: O(log N) Peaks | AVL: Balanced Range Queries | Status: ACTIVE" << std::endl;
}

int main() {
    ProcessTrie trie;
    MLEngine ml;
    
    std::cout << "Initializing AMSE C++ Engine Modules..." << std::endl;
    
    while (true) {
        SystemMetrics sys = LinuxMetrics::getSystemMetrics();
        std::vector<ProcessMetrics> processes = LinuxMetrics::getAllProcessesMetrics();
        std::map<int, std::string> tiers;
        std::map<int, double> peakMemories;
        AVLTree avl; // Rebuilt every cycle for real-time range queries

        // Analytics Cycle
        for (const auto& p : processes) {
            int pid = p.pid;
            
            // 1. Update Windows
            windowManager[pid].addSnapshot(p);
            
            // 2. Compute Features
            auto history = windowManager[pid].getHistory();
            double trend = FeatureProcessor::computeTrend(history);
            double freq = FeatureProcessor::computeFrequency(history);
            
            // 3. Segment Tree Peak Query
            std::vector<double> memHistory;
            for(const auto& h : history) memHistory.push_back(h.memory_rss_mb);
            SegmentTree st(memHistory);
            peakMemories[pid] = st.query(0, memHistory.size() - 1);
            
            // 4. ML Detection & Scoring
            bool anomalous = ml.isAnomalous(p.memory_rss_mb, p.cpu_percent, trend);
            double score = Scorer::calculateScore(freq, trend);
            tiers[pid] = Scorer::classifyTier(score, anomalous);
            
            // 5. Indexing Structures
            trie.insert(p.name, pid);
            avl.insert(p.memory_rss_mb, pid);
        }
        
        displayDashboard(sys, processes, tiers, peakMemories);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
