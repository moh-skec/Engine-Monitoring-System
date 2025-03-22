#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuickWidget>
#include <QTableWidget>
#include <QTimer>
#include "serialhandler.h"
#include "qcgaugewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void handleData(const QByteArray &data);
    void processData();

    void on_portComboBox_activated(int index);

    void on_selectDirectoryButton_clicked();

private:
    Ui::MainWindow *ui;
    SerialHandler serialHandler;

    SerialHandler *handler;

    QByteArray receivedData;

    QcNeedleItem *oilPressureNeedle;
    QcNeedleItem *oilTempNeedle;
    QcNeedleItem *fuelNeedle;
    QcNeedleItem *torqueNeedle;
    QcNeedleItem *motorSpeedNeedle;
    QcNeedleItem *vibrationNeedle;

    QcNeedleItem *createGauge(QcNeedleItem *needle, const QString &title, QLayout *layout, int minValue, int maxValue);

    void parseMessage(const QByteArray &data);
    void updateDisplay(int id, double value);
    void setupGauges();

    void saveDataToCSV(const QString &fileName, const QByteArray &data);
};
#endif // MAINWINDOW_H
