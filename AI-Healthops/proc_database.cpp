#include "proc_database.h"

    Database::Database(QString path){
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(path);

        if (!db.open()) {
            qDebug() << "Error: Could not open database:" << db.lastError().text();
        }

        QSqlQuery query;
        if (!query.exec("CREATE TABLE IF NOT EXISTS stats (ID INTEGER PRIMARY KEY, TIME_STAMP INTEGER, IO_IOPS_READ INTEGER, IO_IOPS_WRITE INTEGER, IO_BYTESREADPERSEC INTEGER, IO_BYTESWRITEPERSEC INTEGER, IO_TOTALBYTESREAD INTEGER, IO_TOTALBYTESWRITE INTEGER, CPU_KERNPERCENT INTEGER, CPU_USERPERCENT INTEGER, CPU_KERNTOTAL INTEGER, CPU_USERTOTAL INTEGER, PROC_PAGEFAULTCOUNT INTEGER, PROC_WORKINGSETSIZE INTEGER, PROC_PEAKWORKINGSETSIZE INTEGER, PROC_PAGEFILEUSAGE INTEGER, PROC_QUOTAPAGEDPOOLUSAGE INTEGER, PROC_QUOTANONPAGEDPOOLUSAGE INTEGER, PROC_QUOTAPEAKNONPAGEDPOOLUSAGE INTEGER)")) {
            qDebug() << "Error creating table:" << query.lastError().text();
            db.close();
        }
        qDebug() << "Table 'stats' created or already exists.";
    }

    Database::~Database(){
        if(db.isValid()){
            db.close();
        }
    }

    bool Database::Save(
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
        quint64 proc_quotapeaknonpagedpoolusage){

        QSqlQuery query;
        query.prepare("INSERT INTO stats (TIME_STAMP , IO_IOPS_READ , IO_IOPS_WRITE , IO_BYTESREADPERSEC , IO_BYTESWRITEPERSEC , IO_TOTALBYTESREAD , IO_TOTALBYTESWRITE , CPU_KERNPERCENT , CPU_USERPERCENT , CPU_KERNTOTAL , CPU_USERTOTAL , PROC_PAGEFAULTCOUNT , PROC_WORKINGSETSIZE , PROC_PEAKWORKINGSETSIZE , PROC_PAGEFILEUSAGE , PROC_QUOTAPAGEDPOOLUSAGE , PROC_QUOTANONPAGEDPOOLUSAGE , PROC_QUOTAPEAKNONPAGEDPOOLUSAGE ) VALUES (:TIME_STAMP, :IO_IOPS_READ, :IO_IOPS_WRITE, :IO_BYTESREADPERSEC, :IO_BYTESWRITEPERSEC, :IO_TOTALBYTESREAD, :IO_TOTALBYTESWRITE, :CPU_KERNPERCENT, :CPU_USERPERCENT, :CPU_KERNTOTAL, :CPU_USERTOTAL, :PROC_PAGEFAULTCOUNT, :PROC_WORKINGSETSIZE, :PROC_PEAKWORKINGSETSIZE, :PROC_PAGEFILEUSAGE, :PROC_QUOTAPAGEDPOOLUSAGE, :PROC_QUOTANONPAGEDPOOLUSAGE, :PROC_QUOTAPEAKNONPAGEDPOOLUSAGE)");

        query.bindValue(":TIME_STAMP", time_stamp);
        query.bindValue(":IO_IOPS_READ", io_iops_read);
        query.bindValue(":IO_IOPS_WRITE", io_iops_write);
        query.bindValue(":IO_BYTESREADPERSEC", io_bytesreadpersec);
        query.bindValue(":IO_BYTESWRITEPERSEC", io_byteswritepersec);
        query.bindValue(":IO_TOTALBYTESREAD", io_totalbytesread);
        query.bindValue(":IO_TOTALBYTESWRITE", io_totalbyteswrite);
        query.bindValue(":CPU_KERNPERCENT", cpu_kernpercent);
        query.bindValue(":CPU_USERPERCENT", cpu_userpercent);
        query.bindValue(":CPU_KERNTOTAL", cpu_kerntotal);
        query.bindValue(":CPU_USERTOTAL", cpu_usertotal);
        query.bindValue(":PROC_PAGEFAULTCOUNT", proc_pagefaultcount);
        query.bindValue(":PROC_WORKINGSETSIZE", proc_workingsetsize);
        query.bindValue(":PROC_PEAKWORKINGSETSIZE", proc_peakworkingsetsize);
        query.bindValue(":PROC_PAGEFILEUSAGE", proc_pagefileusage);
        query.bindValue(":PROC_QUOTAPAGEDPOOLUSAGE", proc_quotapagedpoolusage);
        query.bindValue(":PROC_QUOTANONPAGEDPOOLUSAGE", proc_quotanonpagedpoolusage);
        query.bindValue(":PROC_QUOTAPEAKNONPAGEDPOOLUSAGE", proc_quotapeaknonpagedpoolusage);
        if (!query.exec()) {
            qDebug() << "Failed to insert stats:" << query.lastError().text();
            return false;
        }
        qDebug() << "Data inserted.";
        return true;
    }
