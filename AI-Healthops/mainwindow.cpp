#include "mainwindow.h"
#include "proc_stats.h"

// Add these Windows API includes
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

// Add these standard library includes
#include <thread>
#include <chrono>
#include <algorithm>

// Qt Charts includes - MUST COME BEFORE using namespace QtCharts
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QChartView>

//using namespace QtCharts;

std::vector<Stats> stats;

QLineSeries* cpu_series;
//QLineSeries* line_mark;
QChartView* cpu_chartView;



struct ProcessInfo {
    QString processName;
    DWORD processID;
    double cpuUsage;
    SIZE_T workingSetSize;
    QString userName;
    QString timestamp;
};



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Initialize process update timer
    processUpdateTimer = new QTimer(this);
    connect(processUpdateTimer, &QTimer::timeout, this, &MainWindow::updateProcessTable);

    stats_thread = std::thread([this](){
        PerformanceStats perf_stats;
        int index = 0;
        bool filled = false;
        int size = 100;
        QRandomGenerator generator(123435);
        while(!this->stop){
            std::this_thread::sleep_for(std::chrono::seconds(1));
            auto stat = perf_stats.GetStats();

            if(index >= size){
                filled = true;
                index = 0;
            }

            if(cpu_series != nullptr){
                //cpu_series->remove(index);
                //cpu_series->insert(index, QPointF(index, stat.CPU_KERNPERCENT + stat.CPU_USERPERCENT));
                //auto random_value = generator.generate()%100;
                int random_value = 0;

                if(filled){
                    cpu_series->remove(index);
                    cpu_series->insert(index, QPointF(index, stat.CPU_KERNPERCENT + stat.CPU_USERPERCENT + random_value));
                } else{
                    cpu_series->append(QPointF(index, stat.CPU_KERNPERCENT + stat.CPU_USERPERCENT + random_value));
                }
                //line_mark->clear();
                //line_mark->append(QPointF(index, 50));
                //line_mark->append(QPointF(index, 100));

                 // --- UPDATE TABLE WITH REAL DATA ---
                QMetaObject::invokeMethod(this, [this, stat]() {
                    updateEventsTableWithRealData(stat);
                }, Qt::QueuedConnection);
                // -----------------------------------

                qInfo() << "Index: " << index << "Random value: " << random_value;
                cpu_chartView->update();

                index++;
            }

        }
    });

    setupUI();
    createMenuBar();
    createToolBar();
    createStatusBar();

    // Start the process update timer (update every 10 seconds)
    processUpdateTimer->start(10000);

    // Set window properties
    setWindowTitle("Performance Analyzer - AI HealthOps");
    setMinimumSize(1200, 800);
    resize(1400, 900);
}

MainWindow::~MainWindow()
{
    stop = true;
    if (processUpdateTimer) {
        processUpdateTimer->stop();
    }
    if (stats_thread.joinable()) {
        stats_thread.join();
    }
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create main splitter (horizontal)
    mainSplitter = new QSplitter(Qt::Horizontal, this);

    createLeftPanel();
    createCenterPanel();

    // Add panels to main splitter
    mainSplitter->addWidget(leftPanelGroup);
    mainSplitter->addWidget(centerTabWidget);

    // Set splitter proportions
    mainSplitter->setSizes({300, 900});
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(mainSplitter);
    mainLayout->setContentsMargins(5, 5, 5, 5);
}


void MainWindow::createLeftPanel()
{
    leftPanelGroup = new QGroupBox("Analysis", this);
    leftPanelGroup->setMaximumWidth(350);
    leftPanelGroup->setMinimumWidth(250);

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanelGroup);

    // Analysis tree widget (similar to Graph Explorer)
    analysisTreeWidget = new QTreeWidget(this);
    analysisTreeWidget->setHeaderLabel("Process Activity");

    // Add tree items similar to Reference-2
    QTreeWidgetItem *systemActivity = new QTreeWidgetItem(analysisTreeWidget);
    systemActivity->setText(0, "Process Activity");
    systemActivity->setExpanded(true);

    QTreeWidgetItem *genericEvents = new QTreeWidgetItem(systemActivity);
    genericEvents->setText(0, "CPU usage");

    QTreeWidgetItem *deviceIO = new QTreeWidgetItem(systemActivity);
    deviceIO->setText(0, "Device I/O");

    QTreeWidgetItem *images = new QTreeWidgetItem(systemActivity);
    images->setText(0, "Memory usage");

    QTreeWidgetItem *marks = new QTreeWidgetItem(systemActivity);
    marks->setText(0, "Remark");

    QTreeWidgetItem *processes = new QTreeWidgetItem(systemActivity);
    processes->setText(0, "Processes");

    QTreeWidgetItem *regions = new QTreeWidgetItem(systemActivity);
    regions->setText(0, "Regions of Interest");

    QTreeWidgetItem *stacks = new QTreeWidgetItem(systemActivity);
    stacks->setText(0, "Stacks");

    QTreeWidgetItem *threadLifetimes = new QTreeWidgetItem(systemActivity);
    threadLifetimes->setText(0, "Thread Lifetimes");

    leftLayout->addWidget(analysisTreeWidget);

    connect(analysisTreeWidget, &QTreeWidget::itemClicked,
            this, &MainWindow::onAnalysisItemClicked);
}

void MainWindow::createCenterPanel()
{
    centerTabWidget = new QTabWidget(this);

    // Create main analysis tab
    QWidget *analysisTab = new QWidget();
    QVBoxLayout *analysisLayout = new QVBoxLayout(analysisTab);

    // Timeline/Graph area (placeholder for now)
    timelineWidget = new QWidget();
    timelineWidget->setMinimumHeight(200);
    timelineWidget->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");
    //timelineLabel = new QLabel("Timeline View (Activity by Provider, Task, Opcode)", timelineWidget);
    //timelineLabel->setAlignment(Qt::AlignCenter);

    cpu_series = new QLineSeries();
    //line_mark = new QLineSeries();

    QChart* chart = new QChart();
    chart->addSeries(cpu_series);
    //chart->addSeries(line_mark);
    chart->setTitle("CPU usage");

    QValueAxis* axisX = new QValueAxis();
    axisX->setRange(0, 100);
    QFont axisX_title_font;
    axisX_title_font.setPointSize(12); // Sets the font size to 12 points
    // Apply the font to the axis title
    axisX->setTitleFont(axisX_title_font);
    axisX->setTitleText("Samples");
    chart->addAxis(axisX, Qt::AlignBottom);
    cpu_series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisY->setTitleText("CPU");
    chart->addAxis(axisY, Qt::AlignLeft);
    cpu_series->attachAxis(axisY);

    // Set chart margins to give more space for Y-axis labels
chart->setMargins(QMargins(60, 20, 20, 40));  // left, top, right, bottom

    cpu_chartView = new QChartView(chart);
    cpu_chartView->setMinimumHeight(300);

    QVBoxLayout *timelineLayout = new QVBoxLayout(timelineWidget);
    //timelineLayout->addWidget(timelineLabel);
    timelineLayout->addWidget(cpu_chartView);

    // Events table (similar to Reference-1)
    eventsTableWidget = new QTableWidget(this);
    eventsTableWidget->setColumnCount(7);
    QStringList headers = {"Line #", "Process", "PID", "CPU-User%", "CPU-Kernel", "Total CPU", "TimeStamp"};
    eventsTableWidget->setHorizontalHeaderLabels(headers);
    // ... (rest of eventsTableWidget setup is unchanged) ...
    eventsTableWidget->horizontalHeader()->setStretchLastSection(true);
    eventsTableWidget->setAlternatingRowColors(true);
    eventsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    populateEventsTable();
    
     // Load real current user processes instead of sample data
    getCurrentUserProcesses();

    analysisLayout->addWidget(timelineWidget);
    analysisLayout->addWidget(eventsTableWidget, 1);

     // AI Analysis tab
    aiAnalysisBrowser = new QTextBrowser(this);
    aiAnalysisBrowser->setReadOnly(true);
    aiAnalysisBrowser->setPlaceholderText("Open a process usage analysis file (File -> Open...) to view the AI analysis.");

    // --- CREATE RECOMMENDATIONS TAB ---
    createRecommendationsTab();
    // ----------------------------------

    centerTabWidget->addTab(analysisTab, "Overview");
    centerTabWidget->addTab(aiAnalysisBrowser, "AI based analysis");
    centerTabWidget->addTab(recommendationsWidget, "Recommendation");
}


void MainWindow::createMenuBar()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");

    openFileAction = new QAction("&Open...", this);
    openFileAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openFileAction);

    // --- ADD THIS CONNECTION ---
    connect(openFileAction, &QAction::triggered, this, &MainWindow::openFile);
    // -------------------------

    fileMenu->addSeparator();

    // --- ADD ATTACH TO PROCESS ACTION ---
    attachProcessAction = new QAction("&Attach to a Process...", this);
    attachProcessAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
    fileMenu->addAction(attachProcessAction);
    connect(attachProcessAction, &QAction::triggered, this, &MainWindow::attachToProcess);
    // ------------------------------------

    fileMenu->addSeparator();

    // Add refresh processes action
    QAction *refreshProcessesAction = new QAction("&Refresh Processes", this);
    refreshProcessesAction->setShortcut(QKeySequence(Qt::Key_F5));
    fileMenu->addAction(refreshProcessesAction);
    connect(refreshProcessesAction, &QAction::triggered, this, &MainWindow::getCurrentUserProcesses);

    fileMenu->addSeparator();

    exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    fileMenu->addAction(exitAction);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // Trace menu
    QMenu *traceMenu = menuBar()->addMenu("&Trace");
    analyzeAction = new QAction("&Analyze", this);
    traceMenu->addAction(analyzeAction);

    // Profiles menu
    menuBar()->addMenu("&Profiles");

    // Window menu
    menuBar()->addMenu("&Window");

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    QAction *aboutAction = new QAction("&About", this);
    helpMenu->addAction(aboutAction);
}


void MainWindow::createToolBar()
{
    toolBar = addToolBar("Main");
    toolBar->addAction(openFileAction);
    toolBar->addSeparator();
    toolBar->addAction(analyzeAction);
}

void MainWindow::createStatusBar()
{
    statusBar = QMainWindow::statusBar();
    statusBar->showMessage("Ready");
}

void MainWindow::onAnalysisItemClicked()
{
    QTreeWidgetItem *item = analysisTreeWidget->currentItem();
    if (item) {
        QString itemText = item->text(0);
        statusBar->showMessage("Selected: " + itemText);

        // Switch tab content based on selection
        if (itemText == "CPU usage") {
            centerTabWidget->setCurrentIndex(1);
        } else if (itemText == "Processes") {
            centerTabWidget->setCurrentIndex(2);
        } else {
            centerTabWidget->setCurrentIndex(0);
        }
    }
}

void MainWindow::onProcessSelectionChanged()
{
    // Handle process selection changes
}

void MainWindow::updateSystemActivity()
{
    // Update system activity data
}


// --- ADD THE IMPLEMENTATION FOR THE NEW SLOT AND HELPER FUNCTION ---

/**
 * @brief Opens a file dialog, reads the selected file, parses the JSON,
 * and calls the display function.
 */
void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Analysis File", "", "Text Files (*.txt);;All Files (*)");
    if (filePath.isEmpty()) {
        return; // User cancelled the dialog
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file: " + file.errorString());
        return;
    }

    QByteArray fileContent = file.readAll();
    file.close();

    // The provided file has a text header. We need to find the start of the JSON object '{'.
    int jsonStartPos = fileContent.indexOf('{');
    if (jsonStartPos == -1) {
        QMessageBox::warning(this, "Parsing Error", "Could not find the start of JSON content in the file.");
        return;
    }

    // Extract only the JSON part of the file
    QByteArray jsonData = fileContent.mid(jsonStartPos);
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);

    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        QMessageBox::warning(this, "Parsing Error", "The file does not contain valid JSON data.");
        return;
    }

    displayAnalysisData(jsonDoc.object());
    statusBar->showMessage("Successfully loaded and parsed: " + filePath);
}

void MainWindow::updateRecommendations(const QJsonObject &json)
{
    // Clear existing recommendations except the title
    QLayoutItem *item;
    while ((item = recommendationsLayout->takeAt(2)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // Add updated recommendations from JSON
    if (json.contains("recommendations") && json["recommendations"].isArray()) {
        QGroupBox *aiRecommendationsGroup = new QGroupBox("AI Generated Recommendations");
        aiRecommendationsGroup->setStyleSheet(
            "QGroupBox {"
            "    font-weight: bold;"
            "    border: 2px solid #4CAF50;"
            "    border-radius: 5px;"
            "    margin-top: 10px;"
            "    padding-top: 10px;"
            "    background-color: #f8fff8;"
            "}"
            "QGroupBox::title {"
            "    subcontrol-origin: margin;"
            "    left: 10px;"
            "    padding: 0 5px 0 5px;"
            "    color: #4CAF50;"
            "}"
        );
        
        QVBoxLayout *aiLayout = new QVBoxLayout(aiRecommendationsGroup);
        
        for (const QJsonValue &value : json["recommendations"].toArray()) {
            QJsonObject obj = value.toObject();
            if (obj.contains("label") && obj.contains("details")) {
                QString recText = QString("• <b>%1:</b> %2")
                                  .arg(obj["label"].toString())
                                  .arg(obj["details"].toString());
                
                QLabel *recLabel = new QLabel(recText);
                recLabel->setWordWrap(true);
                recLabel->setTextFormat(Qt::RichText);
                recLabel->setStyleSheet("margin: 5px 0px;");
                
                aiLayout->addWidget(recLabel);
            }
        }
        
        recommendationsLayout->addWidget(aiRecommendationsGroup);
    }
    
    // Add additional static recommendations
    addSampleRecommendations();
    recommendationsLayout->addStretch();
}


/**
 * @brief Formats the JSON object into HTML and displays it in the QTextBrowser.
 * @param json The root QJsonObject parsed from the file.
 */
void MainWindow::displayAnalysisData(const QJsonObject &json)
{
    QString html;
    html.append("<h1>AI Process Usage Analysis</h1>");

    // Summary
    if (json.contains("summary") && json["summary"].isString()) {
        html.append("<h2>Summary</h2>");
        html.append("<p>" + json["summary"].toString() + "</p>");
    }

    // Helper lambda to format sections that are arrays of objects
    auto formatSection = [&](const QString& key, const QString& title) {
        if (json.contains(key) && json[key].isArray()) {
            html.append("<h2>" + title + "</h2>");
            html.append("<ul>");
            for (const QJsonValue &value : json[key].toArray()) {
                QJsonObject obj = value.toObject();
                if (obj.contains("label") && obj.contains("details")) {
                    html.append("<li><b>" + obj["label"].toString() + ":</b> " + obj["details"].toString() + "</li>");
                }
            }
            html.append("</ul>");
        }
    };

    // Use the helper to process each section
    formatSection("keyPoints", "Key Points");
    formatSection("recommendations", "Recommendations");
    formatSection("performanceProfile", "Performance Profile");
    formatSection("resourceHotspots", "Resource Hotspots");

    aiAnalysisBrowser->setHtml(html);
    centerTabWidget->setCurrentWidget(aiAnalysisBrowser); // Automatically switch to the tab
    updateRecommendations(json);
}



/**
 * @brief Shows a dialog to attach to a running process for monitoring
 */
void MainWindow::attachToProcess()
{
    // Create a simple dialog to select a process
    QDialog dialog(this);
    dialog.setWindowTitle("Attach to Process");
    dialog.setModal(true);
    dialog.resize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *label = new QLabel("Select a process to attach to:");
    layout->addWidget(label);

    QTableWidget *processTable = new QTableWidget();
    processTable->setColumnCount(3);
    processTable->setHorizontalHeaderLabels({"Process Name", "PID", "Memory Usage"});
    processTable->horizontalHeader()->setStretchLastSection(true);
    processTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Populate with running processes (placeholder data for now)
    // You can integrate with actual process enumeration later
    processTable->setRowCount(5);
    QStringList processes = {"chrome.exe", "notepad.exe", "explorer.exe", "system", "winlogon.exe"};
    QStringList pids = {"1234", "5678", "9012", "4", "568"};
    QStringList memory = {"245 MB", "12 MB", "156 MB", "8 MB", "45 MB"};

    for (int i = 0; i < 5; ++i) {
        processTable->setItem(i, 0, new QTableWidgetItem(processes[i]));
        processTable->setItem(i, 1, new QTableWidgetItem(pids[i]));
        processTable->setItem(i, 2, new QTableWidgetItem(memory[i]));
    }

    layout->addWidget(processTable);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *attachButton = new QPushButton("Attach");
    QPushButton *cancelButton = new QPushButton("Cancel");

    buttonLayout->addStretch();
    buttonLayout->addWidget(attachButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(attachButton, &QPushButton::clicked, [&]() {
        QModelIndexList selection = processTable->selectionModel()->selectedRows();
        if (!selection.isEmpty()) {
            int row = selection.first().row();
            QString processName = processTable->item(row, 0)->text();
            QString pid = processTable->item(row, 1)->text();

            statusBar->showMessage("Attached to process: " + processName + " (PID: " + pid + ")");
            dialog.accept();
        }
    });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}


void MainWindow::createRecommendationsTab()
{
    recommendationsWidget = new QWidget();
    
    // Create scroll area for recommendations
    recommendationsScrollArea = new QScrollArea();
    recommendationsScrollArea->setWidgetResizable(true);
    recommendationsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    recommendationsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Create content widget for scroll area
    QWidget *scrollContent = new QWidget();
    recommendationsLayout = new QVBoxLayout(scrollContent);
    recommendationsLayout->setSpacing(15);
    recommendationsLayout->setContentsMargins(20, 20, 20, 20);
    
    // Add default content
    QLabel *titleLabel = new QLabel("<h2>Performance Recommendations</h2>");
    titleLabel->setTextFormat(Qt::RichText);
    recommendationsLayout->addWidget(titleLabel);
    
    QLabel *placeholderLabel = new QLabel(
        "<p style='color: #666; font-style: italic;'>"
        "Load a process analysis file to view personalized recommendations for optimizing system performance."
        "</p>"
    );
    placeholderLabel->setTextFormat(Qt::RichText);
    placeholderLabel->setWordWrap(true);
    recommendationsLayout->addWidget(placeholderLabel);
    
    // Add sample recommendations (these will be replaced when file is loaded)
    addSampleRecommendations();
    
    recommendationsLayout->addStretch();
    
    // Set the content widget to scroll area
    recommendationsScrollArea->setWidget(scrollContent);
    
    // Main layout for recommendations widget
    QVBoxLayout *mainRecommendationLayout = new QVBoxLayout(recommendationsWidget);
    mainRecommendationLayout->addWidget(recommendationsScrollArea);
    mainRecommendationLayout->setContentsMargins(0, 0, 0, 0);
}

void MainWindow::addSampleRecommendations()
{
    // CPU Optimization Recommendations
    QGroupBox *cpuGroup = new QGroupBox("CPU Optimization");
    cpuGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #cccccc;"
        "    border-radius: 5px;"
        "    margin-top: 10px;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "}"
    );
    
    QVBoxLayout *cpuLayout = new QVBoxLayout(cpuGroup);
    
    QLabel *cpuRec1 = new QLabel("• <b>Reduce Background Processes:</b> Disable unnecessary startup programs to free up CPU resources.");
    cpuRec1->setWordWrap(true);
    cpuRec1->setTextFormat(Qt::RichText);
    
    QLabel *cpuRec2 = new QLabel("• <b>Update System Drivers:</b> Ensure all hardware drivers are up to date for optimal performance.");
    cpuRec2->setWordWrap(true);
    cpuRec2->setTextFormat(Qt::RichText);
    
    QLabel *cpuRec3 = new QLabel("• <b>Check for Malware:</b> Run a full system scan to ensure no malicious processes are consuming CPU.");
    cpuRec3->setWordWrap(true);
    cpuRec3->setTextFormat(Qt::RichText);
    
    cpuLayout->addWidget(cpuRec1);
    cpuLayout->addWidget(cpuRec2);
    cpuLayout->addWidget(cpuRec3);
    
    // Memory Optimization Recommendations
    QGroupBox *memoryGroup = new QGroupBox("Memory Optimization");
    memoryGroup->setStyleSheet(cpuGroup->styleSheet());
    
    QVBoxLayout *memoryLayout = new QVBoxLayout(memoryGroup);
    
    QLabel *memRec1 = new QLabel("• <b>Increase Virtual Memory:</b> Consider increasing page file size if physical RAM is frequently maxed out.");
    memRec1->setWordWrap(true);
    memRec1->setTextFormat(Qt::RichText);
    
    QLabel *memRec2 = new QLabel("• <b>Close Unused Applications:</b> Regularly close applications that are not actively being used.");
    memRec2->setWordWrap(true);
    memRec2->setTextFormat(Qt::RichText);
    
    QLabel *memRec3 = new QLabel("• <b>Clear System Cache:</b> Periodically clear temporary files and system cache to free up memory.");
    memRec3->setWordWrap(true);
    memRec3->setTextFormat(Qt::RichText);
    
    memoryLayout->addWidget(memRec1);
    memoryLayout->addWidget(memRec2);
    memoryLayout->addWidget(memRec3);
    
    // System Performance Recommendations
    QGroupBox *systemGroup = new QGroupBox("System Performance");
    systemGroup->setStyleSheet(cpuGroup->styleSheet());
    
    QVBoxLayout *systemLayout = new QVBoxLayout(systemGroup);
    
    QLabel *sysRec1 = new QLabel("• <b>Regular System Maintenance:</b> Schedule regular disk cleanup and defragmentation.");
    sysRec1->setWordWrap(true);
    sysRec1->setTextFormat(Qt::RichText);
    
    QLabel *sysRec2 = new QLabel("• <b>Monitor Startup Programs:</b> Use Task Manager to control which programs start with Windows.");
    sysRec2->setWordWrap(true);
    sysRec2->setTextFormat(Qt::RichText);
    
    QLabel *sysRec3 = new QLabel("• <b>Update Operating System:</b> Keep Windows updated with the latest patches and security updates.");
    sysRec3->setWordWrap(true);
    sysRec3->setTextFormat(Qt::RichText);
    
    systemLayout->addWidget(sysRec1);
    systemLayout->addWidget(sysRec2);
    systemLayout->addWidget(sysRec3);
    
    recommendationsLayout->addWidget(cpuGroup);
    recommendationsLayout->addWidget(memoryGroup);
    recommendationsLayout->addWidget(systemGroup);
}



void MainWindow::populateEventsTable()
{
    // Clear existing data
    eventsTableWidget->setRowCount(0);
    
    // Sample process data
    struct ProcessData {
        QString processName;
        QString pid;
        double cpuUser;
        double cpuKernel;
        QString timestamp;
    };
    
    // Create sample data for multiple processes
    QVector<ProcessData> sampleData = {
        {"chrome.exe", "1234", 15.2, 3.8, "2024-12-20 10:15:23.456"},
        {"notepad.exe", "5678", 0.5, 0.2, "2024-12-20 10:15:24.123"},
        {"explorer.exe", "9012", 8.7, 2.1, "2024-12-20 10:15:24.789"},
        {"system", "4", 2.3, 12.5, "2024-12-20 10:15:25.345"},
        {"winlogon.exe", "568", 0.1, 0.8, "2024-12-20 10:15:25.901"},
        {"firefox.exe", "3456", 22.1, 5.3, "2024-12-20 10:15:26.567"},
        {"code.exe", "7890", 18.9, 4.2, "2024-12-20 10:15:27.234"},
        {"outlook.exe", "2345", 12.4, 2.9, "2024-12-20 10:15:27.890"},
        {"teams.exe", "6789", 16.7, 3.6, "2024-12-20 10:15:28.456"},
        {"svchost.exe", "1111", 1.2, 3.4, "2024-12-20 10:15:29.123"},
        {"dwm.exe", "2222", 5.6, 1.8, "2024-12-20 10:15:29.789"},
        {"audiodg.exe", "3333", 2.1, 0.9, "2024-12-20 10:15:30.345"},
        {"powershell.exe", "4444", 7.8, 2.3, "2024-12-20 10:15:30.901"},
        {"cmd.exe", "5555", 0.8, 0.4, "2024-12-20 10:15:31.567"},
        {"taskeng.exe", "6666", 1.5, 1.1, "2024-12-20 10:15:32.234"},
        {"spoolsv.exe", "7777", 0.3, 0.7, "2024-12-20 10:15:32.890"},
        {"lsass.exe", "8888", 0.9, 2.1, "2024-12-20 10:15:33.456"},
        {"csrss.exe", "9999", 1.3, 1.7, "2024-12-20 10:15:34.123"},
        {"winrar.exe", "1010", 25.4, 6.8, "2024-12-20 10:15:34.789"},
        {"steam.exe", "1212", 14.2, 3.5, "2024-12-20 10:15:35.345"}
    };
    
    // Set the number of rows
    eventsTableWidget->setRowCount(sampleData.size());
    
    // Populate the table with sample data
    for (int i = 0; i < sampleData.size(); ++i) {
        const ProcessData &data = sampleData[i];
        double totalCpu = data.cpuUser + data.cpuKernel;
        
        // Line #
        eventsTableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        
        // Process Name
        eventsTableWidget->setItem(i, 1, new QTableWidgetItem(data.processName));
        
        // PID
        eventsTableWidget->setItem(i, 2, new QTableWidgetItem(data.pid));
        
        // CPU-User%
        QTableWidgetItem *userCpuItem = new QTableWidgetItem(QString::number(data.cpuUser, 'f', 1) + "%");
        userCpuItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        eventsTableWidget->setItem(i, 3, userCpuItem);
        
        // CPU-Kernel%
        QTableWidgetItem *kernelCpuItem = new QTableWidgetItem(QString::number(data.cpuKernel, 'f', 1) + "%");
        kernelCpuItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        eventsTableWidget->setItem(i, 4, kernelCpuItem);
        
        // Total CPU%
        QTableWidgetItem *totalCpuItem = new QTableWidgetItem(QString::number(totalCpu, 'f', 1) + "%");
        totalCpuItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        
        // Color code based on CPU usage
        if (totalCpu > 20.0) {
            totalCpuItem->setBackground(QBrush(QColor(255, 200, 200))); // Light red for high usage
        } else if (totalCpu > 10.0) {
            totalCpuItem->setBackground(QBrush(QColor(255, 255, 200))); // Light yellow for medium usage
        } else {
            totalCpuItem->setBackground(QBrush(QColor(200, 255, 200))); // Light green for low usage
        }
        
        eventsTableWidget->setItem(i, 5, totalCpuItem);
        
        // TimeStamp
        eventsTableWidget->setItem(i, 6, new QTableWidgetItem(data.timestamp));
    }
    
    // Auto-resize columns to content
    eventsTableWidget->resizeColumnsToContents();
    
    // Make sure the last column stretches
    eventsTableWidget->horizontalHeader()->setStretchLastSection(true);
    
    // Sort by Total CPU (descending) initially
    eventsTableWidget->sortItems(5, Qt::DescendingOrder);
}

void MainWindow::updateEventsTableWithRealData(const Stats& stat)
{
    // Add new row at the top
    eventsTableWidget->insertRow(0);
    
    // Get current time
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    
    // Populate the new row with real data
    eventsTableWidget->setItem(0, 0, new QTableWidgetItem(QString::number(eventsTableWidget->rowCount())));
    eventsTableWidget->setItem(0, 1, new QTableWidgetItem("AI-Healthops")); // You can get actual process name
    eventsTableWidget->setItem(0, 2, new QTableWidgetItem(QString::number(GetCurrentProcessId())));
    
    QTableWidgetItem *userCpuItem = new QTableWidgetItem(QString::number(stat.CPU_USERPERCENT, 'f', 1) + "%");
    userCpuItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    eventsTableWidget->setItem(0, 3, userCpuItem);
    
    QTableWidgetItem *kernelCpuItem = new QTableWidgetItem(QString::number(stat.CPU_KERNPERCENT, 'f', 1) + "%");
    kernelCpuItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    eventsTableWidget->setItem(0, 4, kernelCpuItem);
    
    double totalCpu = stat.CPU_USERPERCENT + stat.CPU_KERNPERCENT;
    QTableWidgetItem *totalCpuItem = new QTableWidgetItem(QString::number(totalCpu, 'f', 1) + "%");
    totalCpuItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Color code the total CPU
    if (totalCpu > 20.0) {
        totalCpuItem->setBackground(QBrush(QColor(255, 200, 200)));
    } else if (totalCpu > 10.0) {
        totalCpuItem->setBackground(QBrush(QColor(255, 255, 200)));
    } else {
        totalCpuItem->setBackground(QBrush(QColor(200, 255, 200)));
    }
    
    eventsTableWidget->setItem(0, 5, totalCpuItem);
    eventsTableWidget->setItem(0, 6, new QTableWidgetItem(currentTime));
    
    // Keep only last 50 entries for performance
    if (eventsTableWidget->rowCount() > 50) {
        eventsTableWidget->removeRow(50);
    }
}



// Simplified CPU usage function (remove the complex one)
double GetProcessCPUUsage(HANDLE hProcess) {
    // For simplicity, return a random value
    // In a real implementation, you'd calculate actual CPU usage over time
    Q_UNUSED(hProcess);  // Suppress unused parameter warning
    return (double)(rand() % 100) / 10.0; // 0-10% CPU usage
}

// Helper function to get current user name
QString getCurrentUserName() {
    char username[256];
    DWORD size = sizeof(username);
    if (GetUserNameA(username, &size)) {
        return QString::fromLocal8Bit(username);
    }
    return "Unknown";
}

void MainWindow::getCurrentUserProcesses()
{
    // Clear existing data
    eventsTableWidget->setRowCount(0);
    
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    QVector<ProcessInfo> processInfoList;
    QString currentUser = getCurrentUserName();

    // Take a snapshot of all processes in the system
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return;
    }

    // Set the size of the structure before using it
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return;
    }

    // Walk through the process list
    do {
        ProcessInfo procInfo;
        procInfo.processName = QString::fromWCharArray(pe32.szExeFile);
        procInfo.processID = pe32.th32ProcessID;
        procInfo.userName = currentUser; // For simplicity, assuming current user
        procInfo.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

        // Open process to get CPU and memory info
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
        if (hProcess != NULL) {
            // Get memory info
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                procInfo.workingSetSize = pmc.WorkingSetSize;
            } else {
                procInfo.workingSetSize = 0;
            }

            // Get CPU usage (simplified - using random values for demo)
            // In a real implementation, you'd need to calculate actual CPU usage
            procInfo.cpuUsage = (double)(rand() % 100) / 10.0; // 0-10% CPU usage

            CloseHandle(hProcess);
        } else {
            procInfo.workingSetSize = 0;
            procInfo.cpuUsage = 0.0;
        }

        processInfoList.append(procInfo);

    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    // Sort by CPU usage (descending)
    std::sort(processInfoList.begin(), processInfoList.end(), 
              [](const ProcessInfo &a, const ProcessInfo &b) {
                  return a.cpuUsage > b.cpuUsage;
              });

    // Populate the table with process data
    eventsTableWidget->setRowCount(processInfoList.size());
    
    for (int i = 0; i < processInfoList.size() && i < 100; ++i) { // Limit to 100 processes
        const ProcessInfo &procInfo = processInfoList[i];
        
        // Line #
        eventsTableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        
        // Process Name
        eventsTableWidget->setItem(i, 1, new QTableWidgetItem(procInfo.processName));
        
        // PID
        eventsTableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(procInfo.processID)));
        
        // CPU-User% (simplified - showing total CPU usage)
        double userCpu = procInfo.cpuUsage * 0.7; // Assume 70% is user CPU
        QTableWidgetItem *userCpuItem = new QTableWidgetItem(QString::number(userCpu, 'f', 1) + "%");
        userCpuItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        eventsTableWidget->setItem(i, 3, userCpuItem);
        
        // CPU-Kernel% (simplified - showing remaining CPU usage)
        double kernelCpu = procInfo.cpuUsage * 0.3; // Assume 30% is kernel CPU
        QTableWidgetItem *kernelCpuItem = new QTableWidgetItem(QString::number(kernelCpu, 'f', 1) + "%");
        kernelCpuItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        eventsTableWidget->setItem(i, 4, kernelCpuItem);
        
        // Total CPU%
        QTableWidgetItem *totalCpuItem = new QTableWidgetItem(QString::number(procInfo.cpuUsage, 'f', 1) + "%");
        totalCpuItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        
        // Color code based on CPU usage
        if (procInfo.cpuUsage > 5.0) {
            totalCpuItem->setBackground(QBrush(QColor(255, 200, 200))); // Light red for high usage
        } else if (procInfo.cpuUsage > 2.0) {
            totalCpuItem->setBackground(QBrush(QColor(255, 255, 200))); // Light yellow for medium usage
        } else {
            totalCpuItem->setBackground(QBrush(QColor(200, 255, 200))); // Light green for low usage
        }
        
        eventsTableWidget->setItem(i, 5, totalCpuItem);
        
        // TimeStamp
        eventsTableWidget->setItem(i, 6, new QTableWidgetItem(procInfo.timestamp));
    }
    
    // Auto-resize columns to content
    eventsTableWidget->resizeColumnsToContents();
    
    // Make sure the last column stretches
    eventsTableWidget->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::updateProcessTable()
{
    // This method updates the process table periodically
    getCurrentUserProcesses();
    statusBar->showMessage("Process list updated at " + QDateTime::currentDateTime().toString("hh:mm:ss"));
}
