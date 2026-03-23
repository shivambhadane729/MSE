#!/bin/bash

# AMSE Demonstration Script
# Run this in a SEPARATE WSL terminal from the dashboard.

function show_menu() {
    echo "==============================================================="
    echo "  AMSE Behavioral Tier Demonstration"
    echo "==============================================================="
    echo "  1) Trigger [HOT] Tier       (Busy CPU + Slight Memory Growth)"
    echo "  2) Trigger [COLD] Tier      (Mostly Inactive)"
    echo "  3) Trigger [ANOMALOUS] Tier (Fast Growth > 800MB)"
    echo "  4) Stop All Demo Processes"
    echo "  5) Exit"
    echo "==============================================================="
}

while true; do
    show_menu
    read -p "Select an option: " opt
    case $opt in
        1)
            echo "[INFO] Starting HOT process (Busy CPU + memory growth)..."
            python3 -c "
import time
a=[]
i=0
while True:
    a.append(' ' * 1024*1024)
    i+=1
    print(f'  [HOT] Heap size: {i} MB', end='\r')
    time.sleep(0.5)" &
            echo "[PID] $!"
            ;;
        2)
            echo "[INFO] Starting COLD process (High inactivity)..."
            python3 -c "
import time
while True:
    count=0
    [count+1 for _ in range(1000)]
    time.sleep(5)" &
            echo "[PID] $!"
            ;;
        3)
            echo "[INFO] Starting ANOMALOUS process (Fast Growth > 800MB)..."
            python3 -c "
import time
a=[]
i=0
while True:
    a.append(' ' * 25*1024*1024)
    i+=25
    print(f'  [ANOMALOUS] Heap size: {i} MB', end='\r')
    time.sleep(1)" &
            echo "[PID] $!"
            ;;
        4)
            echo "[INFO] Stopping all demo processes..."
            pkill -f "AMSE_DEMO|python3 -c.*HOT|python3 -c.*COLD|python3 -c.*ANOMALOUS"
            echo "[DONE]"
            ;;
        5)
            exit 0
            ;;
        *)
            echo "[ERROR] Invalid option"
            ;;
    esac
done
