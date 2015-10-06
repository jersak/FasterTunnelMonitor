#include <QApplication>
#include "LaClientMonitor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LaClientMonitor clientMonitor;
    clientMonitor.startMonitor();

    return app.exec();
}
