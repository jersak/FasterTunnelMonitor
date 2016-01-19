#ifndef LACLIENTMONITOR_H
#define LACLIENTMONITOR_H

#include <QObject>
#include <QVector>
#include <QSharedMemory>

class QUdpSocket;
class QTimer;
class QFile;

class LaClientMonitor : public QObject
{
    Q_OBJECT
public:
    LaClientMonitor(QObject *parent=0);
    void killAllProcess();
    void disconnectSS5();
    void terminateSS5();

    void startNewLogFile();
    void writeLog(QString log);

private slots:
    void checkProcess();
    void readProcessIds();

private:

    int mFailtAttemptCount;

    QVector<int> mProcessIdList;

    QUdpSocket *mSocket;

    QTimer *mCheckProcessTimer;
    QTimer *mReadProcessIdsTimer;

    // Log
    QFile *mLogFile;
};

#endif // LACLIENTMONITOR_H
