#include "LaClientMonitor.h"

#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QApplication>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDateTime>

const int MONITOR_LISTEN_PORT = 50656;
const int MONITOR_WRITE_PORT = 50657;

const int CHECK_PROCESS_TIMER_INTERVAL = 1750;
const int KILL_PROCESS_TIMER_INTERVAL = 750;

const int MAX_MONITOR_ATTEMPTS = 2;

LaClientMonitor::LaClientMonitor(QObject *parent)
    : QObject(parent)
{
    mSocket = new QUdpSocket();
    mSocket->bind(QHostAddress("127.0.0.1"), MONITOR_LISTEN_PORT);

    mFailtAttemptCount = 0;

    mCheckProcessTimer = new QTimer();
    mCheckProcessTimer->setInterval(CHECK_PROCESS_TIMER_INTERVAL);

    mLogFile = NULL;

    connect(mSocket, SIGNAL(readyRead()), this, SLOT(onClientResponse()));
    connect(mCheckProcessTimer, SIGNAL(timeout()), this, SLOT(checkProcess()));

    mCheckProcessTimer->start();
}

void LaClientMonitor::onClientResponse() {
    QByteArray datagram;
    datagram.resize(mSocket->pendingDatagramSize());
    mSocket->readDatagram(datagram.data(), datagram.size());

    QString r = QString(datagram.data());
    QStringList stringList = r.split(",", QString::SkipEmptyParts);
    mProcessIdList.clear();
    foreach (QString stringId, stringList) {
        mProcessIdList.append(stringId.toInt());
    }
}

void LaClientMonitor::checkProcess()
{
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
        if(mFailtAttemptCount >= MAX_MONITOR_ATTEMPTS) {
            writeLog("Client killed by FailAttempts");
            killAllProcess();
        }
    }
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

