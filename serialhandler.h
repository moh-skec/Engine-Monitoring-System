#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialHandler : public QObject {
    Q_OBJECT
public:
    explicit SerialHandler(QObject *parent = nullptr);
    bool openSerialPort(const QString &portName, qint32 baudRate = 115200,
                        QSerialPort::Parity parity = QSerialPort::OddParity,
                        QSerialPort::StopBits stopBits = QSerialPort::OneStop);
    void closeSerialPort();

signals:
    void dataReceived(const QByteArray &data);

private slots:
    void readData();

private:
    QSerialPort serialPort;
};
#endif // SERIALHANDLER_H
