#include "LaClientMonitor.h"

#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QApplication>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QSharedMemory>

const int SS5_LOG_LOCAL_PORT = 50666;

const int CHECK_CLIENT_PROCESS_TIMER_INTERVAL = 3000;

const int MAX_MONITOR_ATTEMPTS = 3;

LaClientMonitor::LaClientMonitor(QObject *parent)
    : QObject(parent)
{

    mFailtAttemptCount = 0;

    mCheckProcessTimer = new QTimer();
    mCheckProcessTimer->setInterval(CHECK_CLIENT_PROCESS_TIMER_INTERVAL);

    mReadProcessIdsTimer = new QTimer();
    mReadProcessIdsTimer->setInterval(1000);
    mReadProcessIdsTimer->start();

    mLogFile = NULL;

    startNewLogFile();

    connect(mCheckProcessTimer, SIGNAL(timeout()), this, SLOT(checkProcess()));
    connect(mReadProcessIdsTimer, SIGNAL(timeout()), this, SLOT(readProcessIds()));


    QSharedMemory monitorsignature("61BB201D-3569-453e-9144-");
    if(monitorsignature.create(512,QSharedMemory::ReadWrite)==true) {
        monitorsignature.lock();
        writeLog("Monitor iniciado.");
    } else {
        writeLog("Não foi possivel iniciar o monitor.");
        exit(0);
    }

    mCheckProcessTimer->start();
}

void LaClientMonitor::checkProcess()
{
    QSharedMemory shared("61BB200D-3579-453e-9044-");
    if(shared.create(512,QSharedMemory::ReadWrite)==true) {
        writeLog("Nao foi possivel encontrar o cliente. Finalizando.");
        killAllProcess();
    } else {
        //writeLog("Cliente encontrado.");
    }

    /*
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setReadChannelMode(QProcess::MergedChannels);
    process.start("wmic.exe process get description");

    process.waitForStarted(1000);
    process.waitForFinished(1000);

    QByteArray list = process.readAll();
    QString processList = QString(list);

    if(processList.contains("FasterTunnelClient.exe")) {
        qDebug() << "Faster Tunnel is running";
        mFailtAttemptCount=0;
    }
    else {
        qDebug() << "Faster Tunnel is NOT running";
        mFailtAttemptCount++;

        writeLog("Failed to find client process ["
                 + QString::number(mFailtAttemptCount) + " Attempts] - " + QString(list));

        if(mFailtAttemptCount >= MAX_MONITOR_ATTEMPTS) {
            writeLog("Client killed by FailAttempts");
            killAllProcess();
        }
    } */
}

void LaClientMonitor::killAllProcess() {
    foreach (int pid, mProcessIdList) {
        QProcess *p = new QProcess(this);
        QString cmd="taskkill";
        QStringList params;
        params << "/pid" << QString::number(pid) << "/f";
        p->start(cmd,params);
        if(!p->waitForFinished()) {
            delete p;
            return;
        }

        QString result = QString(p->readAllStandardOutput());
        qDebug() << "KillProcess: " << result;
    }

    qApp->quit();
}

void LaClientMonitor::disconnectSS5() {
    QStringList params;
    params << "3" << "2";

    QString path = qApp->applicationDirPath() + "/ss5/ss5capcmd.exe";

    QProcess *p = new QProcess();
    p->start(path, params);
}

void LaClientMonitor::terminateSS5() {
    QStringList params;
    params << "4" << "4";

    QString path = qApp->applicationDirPath() + "/ss5/ss5capcmd.exe";

    QProcess *p = new QProcess();
    p->start(path, params);
}

void LaClientMonitor::startNewLogFile() {
    QString fileName = "monitor_" + QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss") +
            ".log";

    QString path = qApp->applicationDirPath() + "/logs/";

    if(mLogFile && mLogFile->isOpen())
        mLogFile->close();

    QDir().mkdir(path);
    mLogFile = new QFile(path+fileName);
    mLogFile->open(QFile::WriteOnly | QIODevice::Text);
}

void LaClientMonitor::writeLog(QString log) {
    if(!mLogFile)
        startNewLogFile();

    QString time = QTime::currentTime().toString("hh:mm:ss: ");
    mLogFile->write(time.toUtf8() + log.toUtf8());
    mLogFile->write("\n");
    mLogFile->flush();
}

void LaClientMonitor::readProcessIds()
{
    QString path = qApp->applicationDirPath() + QDir::separator();
    QString fileName = "pids.bin";

    QFile * pIdsFile = new QFile(path + fileName);
    QDir().mkdir(path);
    if(pIdsFile->open(QFile::ReadOnly | QIODevice::Text)) {
        mProcessIdList.clear();
        QString pIds = QString(pIdsFile->readLine());
        QStringList pIdsList = pIds.split(";", QString::SkipEmptyParts);
        foreach (QString pid, pIdsList) {
            mProcessIdList.append(pid.toInt());
        }
    }
}

