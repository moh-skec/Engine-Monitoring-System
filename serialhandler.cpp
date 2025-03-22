#include "serialhandler.h"
#include <QDebug>

SerialHandler::SerialHandler(QObject *parent) : QObject(parent) {
    connect(&serialPort, &QSerialPort::readyRead, this, &SerialHandler::readData);
}

bool SerialHandler::openSerialPort(const QString &portName, qint32 baudRate,
                                   QSerialPort::Parity parity,
                                   QSerialPort::StopBits stopBits) {
    serialPort.setPortName(portName);
    serialPort.setBaudRate(baudRate);
    serialPort.setParity(parity);
    serialPort.setStopBits(stopBits);

    if (serialPort.open(QIODevice::ReadOnly)) {
        return true;
    } else {
        qWarning() << "Failed to open port" << portName << serialPort.errorString();
        return false;
    }
}

void SerialHandler::closeSerialPort() {
    if (serialPort.isOpen()) {
        serialPort.close();
    }
}

void SerialHandler::readData() {
    const QByteArray data = serialPort.readAll();
    emit dataReceived(data);
}
