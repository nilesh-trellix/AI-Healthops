#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QString>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <thread>


#include <windows.h>
#include <tlhelp32.h>
#include <QTimer>
#include <QDateTime>
#include <QBrush>
#include <QColor>

#include <QtTypes>




#include <QGridLayout>
#include <QSizePolicy>
#include <QFont>
#include <QPalette>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>

#include <QSplitter>
#include <QGroupBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>


#include <QLabel>



#include <psapi.h>


#include <QtGlobal>  // For quint64





#include <QScrollArea> 


// Add these Qt includes
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>


#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QTextBrowser>


#include <QStatusBar>
#include <QKeySequence>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFile>
#include <QIODevice>
#include <QDialog>
#include <QPushButton>
#include <QItemSelectionModel>

#include <QRandomGenerator>


class QSplitter;
class QTreeWidget;
class QTableWidget;
class QListWidget;
class QTextEdit;
class QLabel;
class QProgressBar;
class QGroupBox;
class QTabWidget;
class QToolBar;
class QStatusBar;
class QAction;
class QWidget;
class QTextBrowser;
class QSelectionModel;

// Forward declaration for Stats struct - ADD THIS LINE
struct Stats;



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAnalysisItemClicked();
    void onProcessSelectionChanged();
    void updateSystemActivity();
    void openFile(); // Add this slot to handle the file open action
    void attachToProcess();  // Add this line
private:
    void setupUI();
    void createLeftPanel();
    void createCenterPanel();
    void createRecommendationsTab();
    void createMenuBar();
    void createToolBar();
    void createStatusBar();
    void updateRecommendations(const QJsonObject &json);
    void addSampleRecommendations();
    // Add this helper function to format and display the JSON data
    void displayAnalysisData(const QJsonObject &json);
    void populateEventsTable(); 
    void updateEventsTableWithRealData(const Stats& stat);  // Add this line to private methods
    void getCurrentUserProcesses();  // Add this line
    void updateProcessTable();       // Add this line

    // Main UI Elements
    QWidget *centralWidget;
    QSplitter *mainSplitter;

    QTimer *processUpdateTimer;   

    // Left Panel
    QGroupBox *leftPanelGroup;
    QTreeWidget *analysisTreeWidget;

    // Center Panel
    QTabWidget *centerTabWidget;
    QWidget *timelineWidget;
    QLabel *timelineLabel;
    QTableWidget *eventsTableWidget;
    QTextBrowser *aiAnalysisBrowser; // Add this to display the analysis text

    // Recommendations Tab
    QWidget *recommendationsWidget;          // Add this line
    QScrollArea *recommendationsScrollArea;  // Add this line
    QVBoxLayout *recommendationsLayout;      // Add this line


    // Actions, Menu, Toolbar, Statusbar
    QAction *openFileAction;
    QAction *exitAction;
    QAction *analyzeAction;
    QToolBar *toolBar;
    QStatusBar *statusBar;
    QAction *attachProcessAction;  // Add this line

    std::thread stats_thread;
    bool stop = false;
};

#endif // MAINWINDOW_H
