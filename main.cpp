#include "mainwindow.h"
#include "serialhandler.h"

#include <QApplication>
#include <QSerialPortInfo>


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    SerialHandler handler;

    MainWindow w;
    w.show();
    return a.exec();
}

