#include <iostream>
#include <cstdlib>
#include <algorithm>
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

#include "ui/Dashboard.hpp"
#include <QApplication>

// Global Window Manager for the session
std::map<int, ProcessWindow> windowManager;

void runCLILoop() {
    ProcessTrie trie;
    MLEngine ml;
    while (true) {
        SystemMetrics sys = LinuxMetrics::getSystemMetrics();
        std::vector<ProcessMetrics> processes = LinuxMetrics::getAllProcessesMetrics();
        
        // Clear screen (Linux/WSL)
        std::cout << "\033[2J\033[H"; 
        
        std::cout << "===============================================================" << std::endl;
        std::cout << "  AMSE - Adaptive Multi-Layer Storage Engine (Terminal)" << std::endl;
        std::cout << "  Timestamp: " << sys.timestamp;
        std::cout << "  CPU Usage: " << std::fixed << std::setprecision(1) << sys.cpu_percent << "% | ";
        std::cout << "  Memory: " << sys.memory_used_mb << " / " << sys.memory_total_mb << " MB (" << sys.memory_percent << "%)" << std::endl;
        std::cout << "===============================================================" << std::endl;
        std::cout << std::left << std::setw(8) << "PID" << std::setw(20) << "NAME" << std::setw(12) << "MEM (MB)" << std::setw(10) << "PEAK" << std::setw(8) << "TIER" << std::endl;
        std::cout << "---------------------------------------------------------------" << std::endl;

        // Sort by memory to show top ones
        std::sort(processes.begin(), processes.end(), [](const ProcessMetrics& a, const ProcessMetrics& b) {
            return a.memory_rss_mb > b.memory_rss_mb;
        });

        int displayed = 0;
        for (const auto& p : processes) {
            if (displayed++ >= 15) break; // Top 15

            windowManager[p.pid].addSnapshot(p);
            auto history = windowManager[p.pid].getHistory();
            double trend = FeatureProcessor::computeTrend(history);
            double freq = FeatureProcessor::computeFrequency(history);
            
            std::vector<double> memHistory;
            for(const auto& h : history) memHistory.push_back(h.memory_rss_mb);
            SegmentTree st(memHistory);
            double peakMemory = st.query(0, memHistory.size() - 1);
            
            bool anomalous = ml.isAnomalous(p, history, trend);
            std::string tier = Scorer::classifyTier(Scorer::calculateScore(freq, trend), anomalous);
            
            std::cout << std::left << std::setw(8) << p.pid 
                      << std::setw(20) << p.name.substr(0, 19) 
                      << std::setw(12) << std::fixed << std::setprecision(1) << p.memory_rss_mb
                      << std::setw(10) << peakMemory
                      << std::setw(8) << tier << std::endl;
        }
        
        std::cout << "---------------------------------------------------------------" << std::endl;
        std::cout << "[CTRL+C to exit]" << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char *argv[]) {
    bool forceNoGui = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--no-gui") {
            forceNoGui = true;
            break;
        }
    }

    // 1. Check if we should run in GUI mode (if DISPLAY is set and not forced no-gui)
    char* display = getenv("DISPLAY");
    
    if (display != nullptr && !forceNoGui) {
        std::cout << "[INFO] Display detected. Launching Graphical Dashboard..." << std::endl;
        try {
            // Attempting to set XCB as fallback if wayland fails or isn't specified
            if (getenv("QT_QPA_PLATFORM") == nullptr) {
                setenv("QT_QPA_PLATFORM", "xcb", 0); 
            }

            QApplication app(argc, argv);
            AMSEDashboard window;
            window.show();
            return app.exec();
        } catch (...) {
            std::cerr << "[ERROR] GUI Initialization failed. Falling back to Terminal..." << std::endl;
        }
    } 
    
    if (forceNoGui) {
        std::cout << "[INFO] --no-gui flag detected. Using Terminal Dashboard." << std::endl;
    } else if (display == nullptr) {
        std::cout << "[INFO] No DISPLAY detected. Falling back to Terminal Dashboard." << std::endl;
    }

    // Fallback to Terminal Dashboard
    runCLILoop();
    
    return 0;
}
