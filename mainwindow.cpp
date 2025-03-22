#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMainWindow>
#include <QtQuickWidgets/QtQuickWidgets>
#include <QQmlEngine>
#include <QQmlContext>
#include <QDebug>
#include <QTimer>
#include <QFileDialog>

static quint8 msgCounter = 0;
static QString filePath = "engine_data.csv";

QStringList dataFields =
    {
        "Oil Pressure",
        "Oil Temp",
        "Fuel Flow",
        "Fuel",
        "EGT",
        "Torque",
        "Indicated Power",
        "Friction Power",
        "Therm Efficiency",
        "Air-Fuel Ratio",
        "Motor Speed",
        "Output Air Speed",
        "Vibration",
        "Body Temp",
        "Air Temp",
};

QStringList sensorFields =
    {
        "Oil Pressure Sensor",
        "Oil Temp Sensor",
        "Fuel Flow Sensor",
        "Fuel Sensor",
        "EGT Sensor",
        "Torque Sensor",
        "Indicated Power Sensor",
        "Friction Power Sensor",
        "Therm Efficiency Sensor",
        "Air-Fuel Ratio Sensor",
        "Motor Speed Sensor",
        "Output Air Speed Sensor",
        "Vibration Sensor",
        "Body Temp Sensor",
        "Air Temp Sensor",
};

quint16 calculateChecksum(const QByteArray &data)
{
    quint16 sum = 0;
    for (char byte : data)
    {
        sum += static_cast<quint8>(byte);
    }
    return sum;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), handler(new SerialHandler(this))
{
    ui->setupUi(this);

    // Set up gauges
    setupGauges();

    on_portComboBox_activated(1);

    connect(&serialHandler, &SerialHandler::dataReceived, this, &MainWindow::handleData);

    // Set up timer for periodic data handling
    QTimer *timer = new QTimer(this);
    timer->start(1000); // 1 second interval
    connect(timer, &QTimer::timeout, this, &MainWindow::processData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startButton_clicked()
{
    QString portName = ui->portComboBox->currentText();
    int baudRate = ui->baudRateComboBox->currentText().toInt();

    QString parityText = ui->parityComboBox->currentText();
    QSerialPort::Parity parity = QSerialPort::NoParity;

    if (parityText == "Odd")
        parity = QSerialPort::OddParity;
    else if (parityText == "Even")
        parity = QSerialPort::EvenParity;
    else if (parityText == "Mark")
        parity = QSerialPort::MarkParity;
    else if (parityText == "Space")
        parity = QSerialPort::SpaceParity;

    QSerialPort::StopBits stopBit = QSerialPort::OneStop;
    QString stopBitText = ui->stopBitComboBox->currentText();

    if (stopBitText == "1.5")
        stopBit = QSerialPort::OneAndHalfStop;
    else if (stopBitText == "2")
        stopBit = QSerialPort::TwoStop;

    if (serialHandler.openSerialPort(portName, baudRate, parity, stopBit))
        ui->statusLabel->setText("Status: Connected");
    else
        ui->statusLabel->setText("Status: Failed to connect");
}

void MainWindow::on_stopButton_clicked()
{
    serialHandler.closeSerialPort();
    ui->statusLabel->setText("Disconnected");
}

void MainWindow::handleData(const QByteArray &data)
{
    // Store the received data
    receivedData = data;
}

void MainWindow::on_selectDirectoryButton_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"), "",
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty())
    {
        filePath = directory + "/engine_data.csv";
        ui->directoryLabel->setText(directory);
    }
}

void MainWindow::processData()
{
    if (!receivedData.isEmpty())
    {
        parseMessage(receivedData);
        saveDataToCSV("engine_data.csv", receivedData);
        receivedData.clear(); // Clear the buffer
    }
}

void MainWindow::saveDataToCSV(const QString &fileName, const QByteArray &data)
{
    QFile file(fileName);
    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing";
        return;
    }

    QTextStream out(&file);
    int dataSize = data.size();
    QByteArray unpackedData = data.mid(4, dataSize - 7);
    for (int i = 4; i < dataSize - 3; i += 10)
    {
        std::reverse(unpackedData.begin() + i, unpackedData.begin() + i + 4);
        std::reverse(unpackedData.begin() + i + 4, unpackedData.begin() + i + 8);
    }

    quint8 idNumber = unpackedData[1];
    out << QString::number(idNumber);
    for (int i = 2; i < 10 * idNumber; i += 10)
    {
        QByteArray valueBytes = unpackedData.mid(i + 2, 4);
        std::reverse(valueBytes.begin(), valueBytes.end());
        quint32 value = *reinterpret_cast<const quint32 *>(valueBytes.data());
        QByteArray factorBytes = unpackedData.mid(i + 6, 4);
        std::reverse(factorBytes.begin(), factorBytes.end());
        quint32 factor = *reinterpret_cast<const quint32 *>(factorBytes.data());
        double realValue = factor == 0 ? value : static_cast<double>(value) / factor;
        out << "," << QString::number(realValue, 'f', 1);
    }
    out << "\n";
    file.close();
}

void MainWindow::parseMessage(const QByteArray &data)
{
    int dataSize = data.size();
    if (dataSize % 10 != 9 || dataSize > 309)
        return; // Ensure true message length

    // Check header and footer
    QByteArray checkSum = data.mid(dataSize - 3, 2);
    std::reverse(checkSum.begin(), checkSum.end());
    QByteArray unpackedData = data.mid(4, dataSize - 7);
    for (int i = 4; i < dataSize - 3; i += 10)
    {
        std::reverse(unpackedData.begin() + i, unpackedData.begin() + i + 4);
        std::reverse(unpackedData.begin() + i + 4, unpackedData.begin() + i + 8);
    }
    if (data.left(4) == QByteArray::fromHex("A5A5A5A5") && data.right(1) == QByteArray::fromHex("55") && qFromBigEndian<quint16>(checkSum.constData()) == calculateChecksum(unpackedData))
    {
        if (msgCounter == unpackedData[0])
            return;
        msgCounter = unpackedData[0];
        quint8 idNumber = unpackedData[1];
        for (int i = 2; i < 10 * idNumber; i += 10)
        {
            quint8 id = unpackedData[i];
            QByteArray valueBytes = unpackedData.mid(i + 2, 4);
            std::reverse(valueBytes.begin(), valueBytes.end());
            quint32 value = *reinterpret_cast<const quint32 *>(valueBytes.data());
            QByteArray factorBytes = unpackedData.mid(i + 6, 4);
            std::reverse(factorBytes.begin(), factorBytes.end());
            quint32 factor = *reinterpret_cast<const quint32 *>(factorBytes.data());
            double realValue = factor == 0 ? value : static_cast<double>(value) / factor;
            updateDisplay(id, QString::number(realValue, 'f', 1).toDouble()); // Update specific gauges based on ID
        }
    }
}

void MainWindow::updateDisplay(int id, double value)
{
    QTableWidgetItem *dataItem = new QTableWidgetItem(QString::number(value));
    dataItem->setData(Qt::UserRole, QVariant::fromValue(dataItem));
    QTableWidgetItem *sensorItem = new QTableWidgetItem();
    sensorItem->setData(Qt::UserRole, QVariant::fromValue(sensorItem));
    switch (id)
    {
    case 0x01:
        if (value >= 0 && value <= 1000)
        {
            oilPressureNeedle->setCurrentValue(value);
            ui->dataTable->setItem(0, 3, dataItem);
        }
        break;
    case 0x02:
        if (value >= 0 && value <= 400)
        {
            oilTempNeedle->setCurrentValue(value);
            ui->dataTable->setItem(1, 3, dataItem);
        }
        break;
    case 0x03:
        if (value >= 0 && value <= 800)
        {
            ui->dataTable->setItem(2, 3, dataItem);
        }
        break;
    case 0x04:
        if (value >= 0 && value <= 800)
        {
            fuelNeedle->setCurrentValue(value);
            ui->dataTable->setItem(3, 3, dataItem);
        }
        break;
    case 0x05:
        if (value >= 0 && value <= 400)
        {
            ui->dataTable->setItem(4, 3, dataItem);
        }
        break;
    case 0x06:
        if (value >= 0 && value <= 400)
        {
            torqueNeedle->setCurrentValue(value);
            ui->dataTable->setItem(5, 3, dataItem);
        }
        break;
    case 0x07:
        if (value >= 0 && value <= 400)
        {
            ui->dataTable->setItem(6, 3, dataItem);
        }
        break;
    case 0x08:
        if (value >= 0 && value <= 400)
        {
            ui->dataTable->setItem(7, 3, dataItem);
        }
        break;
    case 0x09:
        if (value >= 0 && value <= 100)
        {
            ui->dataTable->setItem(8, 3, dataItem);
        }
        break;
    case 0x0A:
        if (value >= 0 && value <= 20)
        {
            ui->dataTable->setItem(9, 3, dataItem);
        }
        break;
    case 0x0B:
        if (value >= 0 && value <= 1000)
        {
            motorSpeedNeedle->setCurrentValue(value);
            ui->dataTable->setItem(10, 3, dataItem);
        }
        break;
    case 0x0C:
        if (value >= 0 && value <= 1000)
        {
            ui->dataTable->setItem(11, 3, dataItem);
        }
        break;
    case 0x0D:
        if (value >= 0 && value <= 100)
        {
            vibrationNeedle->setCurrentValue(value);
            ui->dataTable->setItem(12, 3, dataItem);
        }
        break;
    case 0x0E:
        if (value >= 0 && value <= 400)
        {
            ui->dataTable->setItem(13, 3, dataItem);
        }
        break;
    case 0x0F:
        if (value >= 0 && value <= 400)
        {
            ui->dataTable->setItem(14, 3, dataItem);
        }
        break;
    case 0x11:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(0, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(0, 1, sensorItem);
        }
        break;
    case 0x12:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(1, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(1, 1, sensorItem);
        }
        break;
    case 0x13:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(2, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(2, 1, sensorItem);
        }
        break;
    case 0x14:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(3, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(3, 1, sensorItem);
        }
        break;
    case 0x15:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(4, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(4, 1, sensorItem);
        }
        break;
    case 0x16:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(5, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(5, 1, sensorItem);
        }
        break;
    case 0x17:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(6, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(6, 1, sensorItem);
        }
        break;
    case 0x18:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(7, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(7, 1, sensorItem);
        }
        break;
    case 0x19:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(8, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(8, 1, sensorItem);
        }
        break;
    case 0x1A:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(9, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(9, 1, sensorItem);
        }
        break;
    case 0x1B:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(10, 20, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(10, 1, sensorItem);
        }
        break;
    case 0x1C:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(11, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(11, 1, sensorItem);
        }
        break;
    case 0x1D:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(12, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(12, 1, sensorItem);
        }
        break;
    case 0x1E:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(13, 1, sensorItem);
        }
        else if (value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(13, 1, sensorItem);
        }
        break;
    case 0x1F:
        if (value == 1)
        {
            sensorItem->setText("ERROR");
            sensorItem->setBackground(QBrush(Qt::red));
            ui->sensorTable->setItem(14, 1, sensorItem);
        }
        else if(value == 0)
        {
            sensorItem->setText("OK");
            sensorItem->setBackground(QBrush(Qt::green));
            ui->sensorTable->setItem(14, 1, sensorItem);
        }
        break;
    }
}

void MainWindow::setupGauges()
{
    oilPressureNeedle = createGauge(oilPressureNeedle, "Oil Pressure", ui->oilPressureLayout, 0, 1000);
    oilTempNeedle = createGauge(oilTempNeedle, "Oil Temp", ui->oilTempLayout, 0, 400);
    fuelNeedle = createGauge(fuelNeedle, "Fuel", ui->fuelLayout, 0, 800);
    torqueNeedle = createGauge(torqueNeedle, "Torque", ui->torqueLayout, 0, 400);
    motorSpeedNeedle = createGauge(motorSpeedNeedle, "Motor Speed", ui->motorSpeedLayout, 0, 1000);
    vibrationNeedle = createGauge(vibrationNeedle, "Vibration", ui->vibrationLayout, 0, 100);
}


void MainWindow::on_portComboBox_activated(int index)
{
    Q_UNUSED(index);
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
    {
        if (ui->portComboBox->findText(info.portName()) == -1)
        {
            ui->portComboBox->addItem(info.portName());
        }
    }
}

QcNeedleItem *MainWindow::createGauge(QcNeedleItem *needle, const QString &title, QLayout *layout, int minValue, int maxValue)
{
    QcGaugeWidget *gauge = new QcGaugeWidget;
    gauge->addBackground(99);
    QcBackgroundItem *bkg1 = gauge->addBackground(92);
    bkg1->clearrColors();
    bkg1->addColor(0.1,Qt::black);
    bkg1->addColor(1.0,Qt::white);

    QcBackgroundItem *bkg2 = gauge->addBackground(88);
    bkg2->clearrColors();
    bkg2->addColor(0.1,Qt::gray);
    bkg2->addColor(1.0,Qt::darkGray);

    gauge->addArc(55);
    gauge->addDegrees(65)->setValueRange(minValue, maxValue);
    gauge->addColorBand(50);

    gauge->addValues(80)->setValueRange(minValue, maxValue);

    gauge->addLabel(70)->setText(title);
    QcLabelItem *lab = gauge->addLabel(40);
    lab->setText("0");
    needle = gauge->addNeedle(60);
    needle->setLabel(lab);
    needle->setColor(Qt::white);
    needle->setValueRange(minValue, maxValue);
    gauge->addBackground(7);
    gauge->addGlass(88);
    layout->addWidget(gauge);

    return needle;
}
