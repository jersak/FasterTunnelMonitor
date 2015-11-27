#ifndef LACLIENTMONITOR_H
#define LACLIENTMONITOR_H

#include <QObject>
#include <QVector>

class QUdpSocket;
class QTimer;
class QFile;

class LaClientMonitor : public QObject
{
    Q_OBJECT
public:
    LaClientMonitor(QObject *parent=0);

private slots:
    void onClientResponse();
    void checkProcess();

private:
    void killAllProcess();
    void disconnectSS5();
    void terminateSS5();

    void startNewLogFile();
    void writeLog(QString log);

    int mFailtAttemptCount;

    QVector<int> mProcessIdList;

    QUdpSocket *mSocket;
    QTimer *mCheckProcessTimer;
    QTimer *mKillProcessTimer;

    // Log
    QFile *mLogFile;
};

#endif // LACLIENTMONITOR_H
