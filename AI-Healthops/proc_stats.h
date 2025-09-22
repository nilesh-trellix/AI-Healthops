#ifndef PROC_STATS_H
#define PROC_STATS_H

#include "mainwindow.h"

struct Stats {
    quint64 IO_IOPS_READ;
    quint64 IO_IOPS_WRITE;
    quint64 IO_BYTESREADPERSEC;
    quint64 IO_BYTESWRITEPERSEC;
    quint64 IO_TOTALBYTESREAD;
    quint64 IO_TOTALBYTESWRITE;
    quint64 CPU_KERNPERCENT;
    quint64 CPU_USERPERCENT;
    quint64 CPU_KERNTOTAL;
    quint64 CPU_USERTOTAL;
    quint64 PROC_PAGEFAULTCOUNT;
    quint64 PROC_WORKINGSETSIZE;
    quint64 PROC_PEAKWORKINGSETSIZE;
    quint64 PROC_PAGEFILEUSAGE;
    quint64 PROC_QUOTAPAGEDPOOLUSAGE;
    quint64 PROC_QUOTANONPAGEDPOOLUSAGE;
    quint64 PROC_QUOTAPEAKNONPAGEDPOOLUSAGE;
};

struct PerformanceStats
{

    PerformanceStats(std::string db_path = "", int pid = 0, int stats_query_interval = 0);
    ~PerformanceStats();

    std::vector<Stats> GetStats(quint64 start, quint64 end);
    Stats GetStats();
    int SaveStats(Stats stats);
    int QueryStats();

//signals:
//    void ResetTableWidget();
//    void UpdateTableWidget(Stats stats);


    int pid_;
    int stats_query_interval_;
    HANDLE hProc = nullptr;
    IO_COUNTERS prev_io_counters;
    ULARGE_INTEGER prev_kern_time;
    ULARGE_INTEGER prev_user_time;
    ULARGE_INTEGER prev_system_time;
};



#endif // PROC_STATS_H
