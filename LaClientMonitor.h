#ifndef LACLIENTMONITOR_H
#define LACLIENTMONITOR_H

#include <QObject>
#include <QSet>

class QUdpSocket;
class QTimer;
class QFile;

class LaClientMonitor : public QObject
{
    Q_OBJECT
public:
    LaClientMonitor(QObject *parent=0);

    void startMonitor();

private slots:
    void onClientResponse();
    void onKillProcessTimeout();
    void onKeepAliveTimeout();

private:
    void killAllProcess();
    void disconnectSS5();
    void terminateSS5();

    void startNewLogFile();
    void writeLog(QString log);

    int mFailtAttemptCount;

    QSet<int> mProcessIdList;

    QUdpSocket *mSocket;
    QTimer *mKeepAliveTimer;
    QTimer *mKillProcessTimer;

    // Log
    QFile *mLogFile;
};

#endif // LACLIENTMONITOR_H
