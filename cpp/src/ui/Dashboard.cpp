#include "ui/Dashboard.hpp"
#include <QHeaderView>
#include <QApplication>
#include <QDateTime>

AMSEDashboard::AMSEDashboard(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    applyDarkTheme();
    
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &AMSEDashboard::updateData);
    updateTimer->start(1000); // 1 FPS refresh
}

AMSEDashboard::~AMSEDashboard() {}

void AMSEDashboard::setupUI() {
    setWindowTitle("AMSE C++ High-Performance Dashboard (Linux)");
    resize(1000, 700);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Header Stats
    QHBoxLayout *headerLayout = new QHBoxLayout();
    cpuLabel = new QLabel("CPU Usage: 0.0%", this);
    memLabel = new QLabel("RAM Usage: 0.0%", this);
    cpuLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #00bcd4;");
    memLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #4caf50;");
    
    headerLayout->addWidget(cpuLabel);
    headerLayout->addWidget(memLabel);
    layout->addLayout(headerLayout);

    // Process Table
    processTable = new QTableWidget(this);
    processTable->setColumnCount(5);
    processTable->setHorizontalHeaderLabels({"PID", "Process Name", "Memory (MB)", "Peak (MB)", "Tier"});
    processTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    processTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    processTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    layout->addWidget(processTable);
}

void AMSEDashboard::applyDarkTheme() {
    this->setStyleSheet(
        "QMainWindow { background-color: #121212; }"
        "QTableWidget { background-color: #1e1e1e; color: #e0e0e0; gridline-color: #333; border: none; font-family: 'Segoe UI', sans-serif; }"
        "QHeaderView::section { background-color: #333; color: #bbb; padding: 5px; border: 1px solid #444; }"
        "QLabel { color: #e0e0e0; }"
    );
}

void AMSEDashboard::updateData() {
    SystemMetrics sys = LinuxMetrics::getSystemMetrics();
    std::vector<ProcessMetrics> processes = LinuxMetrics::getAllProcessesMetrics();

    cpuLabel->setText(QString("CPU Usage: %1%").arg(sys.cpu_percent, 0, 'f', 1));
    memLabel->setText(QString("RAM Usage: %1% (%2/%3 MB)").arg(sys.memory_percent, 0, 'f', 1)
                      .arg(sys.memory_used_mb, 0, 'f', 0).arg(sys.memory_total_mb, 0, 'f', 0));

    processTable->setRowCount(std::min((int)processes.size(), 50));

    for (int i = 0; i < processTable->rowCount(); ++i) {
        const auto& p = processes[i];
        int pid = p.pid;
        
        // 1. Update Windows
        windowManager[pid].addSnapshot(p);
        auto history = windowManager[pid].getHistory();
        
        // 2. Analytics
        double trend = FeatureProcessor::computeTrend(history);
        double freq = FeatureProcessor::computeFrequency(history);
        
        std::vector<double> memHistory;
        for(const auto& h : history) memHistory.push_back(h.memory_rss_mb);
        SegmentTree st(memHistory);
        double peak = st.query(0, memHistory.size() - 1);
        
        bool anomalous = ml.isAnomalous(p.memory_rss_mb, p.cpu_percent, trend);
        std::string tier = Scorer::classifyTier(Scorer::calculateScore(freq, trend), anomalous);

        // 3. Populate Table
        processTable->setItem(i, 0, new QTableWidgetItem(QString::number(pid)));
        processTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(p.name)));
        processTable->setItem(i, 2, new QTableWidgetItem(QString::number(p.memory_rss_mb, 'f', 1)));
        processTable->setItem(i, 3, new QTableWidgetItem(QString::number(peak, 'f', 1)));
        
        QTableWidgetItem *tierItem = new QTableWidgetItem(QString::fromStdString(tier));
        if (tier == "Anomalous") tierItem->setForeground(QColor("#ff5252"));
        else if (tier == "Hot") tierItem->setForeground(QColor("#00bcd4"));
        else if (tier == "Cold") tierItem->setForeground(QColor("#9e9e9e"));
        
        processTable->setItem(i, 4, tierItem);
    }
}
