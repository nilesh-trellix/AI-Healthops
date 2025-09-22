#include "proc_stats.h"


PerformanceStats::PerformanceStats(std::string db_path, int pid, int stats_query_interval) : pid_(pid), stats_query_interval_(stats_query_interval) {
    //create or open db
    //create thread to periodically query the stats and save it to db

    FILETIME ftime, fsys, fuser;
    GetSystemTimeAsFileTime(&ftime);
    memcpy(&prev_system_time, &ftime, sizeof(FILETIME));

    if (pid_ != 0) {
        const DWORD kRights = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
        hProc = ::OpenProcess(kRights, FALSE, pid_);
    }
    else {
        hProc = GetCurrentProcess();
    }

    GetProcessTimes(hProc, &ftime, &ftime, &fsys, &fuser);
    memcpy(&prev_kern_time, &fsys, sizeof(FILETIME));
    memcpy(&prev_user_time, &fuser, sizeof(FILETIME));

    GetProcessIoCounters(hProc, &prev_io_counters);
}


    PerformanceStats::~PerformanceStats() {
        if (hProc) {
            CloseHandle(hProc);
        }
    }

    std::vector<Stats> PerformanceStats::GetStats(quint64 start, quint64 end) {
        std::vector<Stats> stats;

        return stats;
    }


    Stats PerformanceStats::GetStats() {
        Stats stats;
        try {
            FILETIME ftime, fsys, fuser;
            ULARGE_INTEGER now, kernel, user;
            GetSystemTimeAsFileTime(&ftime);
            memcpy(&now, &ftime, sizeof(FILETIME));

            GetProcessTimes(hProc, &ftime, &ftime, &fsys, &fuser);
            memcpy(&kernel, &fsys, sizeof(FILETIME));
            memcpy(&user, &fuser, sizeof(FILETIME));

            double kernelTimeDiff = (double)(kernel.QuadPart - prev_kern_time.QuadPart);
            double userTimeDiff = (double)(user.QuadPart - prev_user_time.QuadPart);
            double systemTimeDiff = (double)(now.QuadPart - prev_system_time.QuadPart);

            // Update global variables for next calculation
            prev_kern_time = kernel;
            prev_user_time = user;
            prev_system_time = now;

            stats.CPU_KERNTOTAL = static_cast<quint64>(kernelTimeDiff);
            if (systemTimeDiff > 0.001 && kernelTimeDiff > 0.001) {
                stats.CPU_KERNPERCENT = static_cast<quint64>((kernelTimeDiff / systemTimeDiff) * 100.0);
            }
            else {
                stats.CPU_KERNPERCENT = 0;
            }

            stats.CPU_USERTOTAL = static_cast<quint64>(userTimeDiff);
            if (systemTimeDiff > 0.001 && userTimeDiff > 0.001) {
                stats.CPU_USERPERCENT = static_cast<quint64>((userTimeDiff / systemTimeDiff) * 100.0);
            }
            else {
                stats.CPU_USERPERCENT = 0;
            }


            PROCESS_MEMORY_COUNTERS counters;
            if (hProc && (GetProcessMemoryInfo(hProc, &counters, sizeof(counters)) != 0)) {
                stats.PROC_PAGEFAULTCOUNT = counters.PageFaultCount;
                stats.PROC_PEAKWORKINGSETSIZE = counters.PeakWorkingSetSize;
                stats.PROC_WORKINGSETSIZE = counters.WorkingSetSize;
                stats.PROC_QUOTAPAGEDPOOLUSAGE = counters.QuotaPagedPoolUsage;
                stats.PROC_QUOTANONPAGEDPOOLUSAGE = counters.QuotaPeakNonPagedPoolUsage;
                stats.PROC_QUOTAPEAKNONPAGEDPOOLUSAGE = counters.QuotaNonPagedPoolUsage;
                stats.PROC_PAGEFILEUSAGE = counters.PagefileUsage;
            }

            IO_COUNTERS cur_io_counters;
            if (GetProcessIoCounters(hProc, &cur_io_counters)) {
                stats.IO_IOPS_READ = (cur_io_counters.ReadOperationCount - prev_io_counters.ReadOperationCount) /* / stats_query_interval_ */;


                stats.IO_IOPS_WRITE = (cur_io_counters.WriteOperationCount - prev_io_counters.WriteOperationCount) /* / stats_query_interval_ */ ;

                stats.IO_TOTALBYTESREAD = cur_io_counters.ReadTransferCount - prev_io_counters.ReadTransferCount;
                if (stats.IO_TOTALBYTESREAD != 0) {
                    stats.IO_BYTESREADPERSEC = stats.IO_TOTALBYTESREAD / systemTimeDiff;
                }
                else {
                    stats.IO_BYTESREADPERSEC = 0;
                }

                if (cur_io_counters.WriteTransferCount != 0) {
                    stats.IO_TOTALBYTESWRITE = cur_io_counters.WriteTransferCount - prev_io_counters.WriteTransferCount;
                }
                else {
                    stats.IO_TOTALBYTESWRITE = 0;
                }

                if (stats.IO_TOTALBYTESWRITE != 0) {
                    stats.IO_BYTESWRITEPERSEC = stats.IO_TOTALBYTESWRITE / systemTimeDiff;
                }
                else {
                    stats.IO_BYTESWRITEPERSEC = 0;
                }

                prev_io_counters = cur_io_counters;

            }


        }
        catch(...){}

        return stats;
    }

//    int PerformanceStats::SaveStats(Stats stats) {
//
//    }
//
//    int PerformanceStats::QueryStats() {
//
//    }

