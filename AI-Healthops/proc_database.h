#ifndef PROC_DATABASE_H
#define PROC_DATABASE_H

#include "mainwindow.h"


struct Database
{
    Database(QString path);

    ~Database();

    bool Save(
        quint64 time_stamp,
        quint64 io_iops_read,
        quint64 io_iops_write,
        quint64 io_bytesreadpersec,
        quint64 io_byteswritepersec,
        quint64 io_totalbytesread,
        quint64 io_totalbyteswrite,
        quint64 cpu_kernpercent,
        quint64 cpu_userpercent,
        quint64 cpu_kerntotal,
        quint64 cpu_usertotal,
        quint64 proc_pagefaultcount,
        quint64 proc_workingsetsize,
        quint64 proc_peakworkingsetsize,
        quint64 proc_pagefileusage,
        quint64 proc_quotapagedpoolusage,
        quint64 proc_quotanonpagedpoolusage,
        quint64 proc_quotapeaknonpagedpoolusage);

    QSqlDatabase db;
};

#endif // PROC_DATABASE_H
