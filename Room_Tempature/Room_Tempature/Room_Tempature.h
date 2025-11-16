#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Room_Tempature.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>

class Room_Tempature : public QMainWindow
{
    Q_OBJECT

public:
    Room_Tempature(QWidget* parent = nullptr);
    ~Room_Tempature();

private slots:
    void refreshPorts();
    void connectOrDisconnect();
    void handleReadyRead();
    void handleSerialError(QSerialPort::SerialPortError error);

private:
    Ui::Room_TempatureClass ui;

    QSerialPort* m_serial;
    QByteArray m_readBuffer;

    // helper
    void ensureUiWidgets();                 // create widgets at runtime if ui doesn't contain them
    void parseLine(const QString& line);    // parse serial line and update UI
    void setConnectedUi(bool connected);
};
