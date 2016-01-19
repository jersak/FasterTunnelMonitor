#include <QApplication>
#include <QSharedMemory>
#include "LaClientMonitor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LaClientMonitor clientMonitor;

    QSharedMemory monitorsignature;
    monitorsignature.setKey("1234");
    if(monitorsignature.create(512,QSharedMemory::ReadWrite)==true) {
        //monitorsignature.lock();
        clientMonitor.writeLog("Main: Monitor iniciado.");
    } else {
        clientMonitor.writeLog("Main: Não foi possivel iniciar o monitor.");
        exit(0);
    }

    return app.exec();
}
