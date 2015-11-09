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

const int KEEP_ALIVE_TIMER_INTERVAL = 750;
const int KILL_PROCESS_TIMER_INTERVAL = 750;

const int MAX_MONITOR_ATTEMPTS = 5;

LaClientMonitor::LaClientMonitor(QObject *parent)
    : QObject(parent)
{
    mSocket = new QUdpSocket();
    mSocket->bind(QHostAddress("127.0.0.1"), MONITOR_LISTEN_PORT);

    mFailtAttemptCount = 0;

    mKeepAliveTimer = new QTimer();
    mKeepAliveTimer->setInterval(KEEP_ALIVE_TIMER_INTERVAL);
    mKillProcessTimer = new QTimer();
    mKillProcessTimer->setInterval(KILL_PROCESS_TIMER_INTERVAL);

    mLogFile = NULL;

    connect(mSocket, SIGNAL(readyRead()), this, SLOT(onClientResponse()));
    connect(mKillProcessTimer, SIGNAL(timeout()), this, SLOT(onKillProcessTimeout()));
    connect(mKeepAliveTimer, SIGNAL(timeout()), this, SLOT(onKeepAliveTimeout()));
}

void LaClientMonitor::startMonitor() {
    mKeepAliveTimer->start();
//    mKillProcessTimer->start();
}

void LaClientMonitor::onClientResponse() {
    QByteArray datagram;
    datagram.resize(mSocket->pendingDatagramSize());
    mSocket->readDatagram(datagram.data(), datagram.size());

    QString r = QString(datagram.data());

    qDebug() << "ClientResponse: " << r;

    if(r.contains("process_id:")){
        QString pIdString = r.remove("process_id:");
        int pId = pIdString.toInt();
        mProcessIdList.insert(pId);
        qDebug() << "Received ProcessId: " << pId;
    }

    if(r.contains("clearPidList")){
        mProcessIdList.clear();
        qDebug() << "PIdList is clear";
    }

    if(r.contains("ClientIsRunning")) {
        mFailtAttemptCount = 0;

        if(!mKillProcessTimer->isActive()) {
            mKillProcessTimer->start();
        }
    }
}

void LaClientMonitor::onKeepAliveTimeout() {
    QString s = QString("MonitorIsRunning");
    QByteArray b = QByteArray(s.toStdString().c_str());
    mSocket->writeDatagram(b, QHostAddress("127.0.0.1"), MONITOR_WRITE_PORT);
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

void LaClientMonitor::onKillProcessTimeout() {
    mFailtAttemptCount++;

    qDebug() << "FailAttempts: " <<mFailtAttemptCount;

    if(mFailtAttemptCount > MAX_MONITOR_ATTEMPTS) {
//        mKillProcessTimer->stop();
//        mKeepAliveTimer->stop();

        disconnectSS5();
        terminateSS5();

        killAllProcess();

        qDebug() << "Kill all process";
        qApp->quit();
    }
}

void LaClientMonitor::startNewLogFile() {

    QString fileName = QDateTime::currentDateTime().toString("monitor_yyyy_MM_dd-hh_mm_ss") +
            ".log";
    QString path = qApp->applicationDirPath() + QDir::separator() +
            "logs" + QDir::separator();

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

