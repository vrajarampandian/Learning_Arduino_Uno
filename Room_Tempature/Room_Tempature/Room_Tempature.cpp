#include "Room_Tempature.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>

Room_Tempature::Room_Tempature(QWidget* parent)
    : QMainWindow(parent),
    m_serial(new QSerialPort("COM3", this))
{
    ui.setupUi(this);

    // Make sure expected widgets exist; create if missing
    ensureUiWidgets();

    // Connect UI buttons (use findChild to be tolerant)
    if (auto cb = findChild<QComboBox*>("comboPorts")) {
        Q_UNUSED(cb);
    }

    if (auto btn = findChild<QPushButton*>("btnRefresh")) {
        connect(btn, &QPushButton::clicked, this, &Room_Tempature::refreshPorts);
    }
    if (auto btn = findChild<QPushButton*>("btnConnect")) {
        connect(btn, &QPushButton::clicked, this, &Room_Tempature::connectOrDisconnect);
    }

    connect(m_serial, &QSerialPort::readyRead, this, &Room_Tempature::handleReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred, this, &Room_Tempature::handleSerialError);

    refreshPorts();
    setConnectedUi(false);
}

Room_Tempature::~Room_Tempature()
{
    if (m_serial->isOpen()) m_serial->close();
}

// Ensure widgets exist in UI; if not, create and layout them
void Room_Tempature::ensureUiWidgets()
{
    // central widget provided by Designer? if not, create one
    QWidget* central = centralWidget();
    if (!central) {
        central = new QWidget(this);
        setCentralWidget(central);
    }

    // Look up widgets by objectName; if not present create them
    QComboBox* comboPorts = findChild<QComboBox*>("comboPorts");
    QPushButton* btnRefresh = findChild<QPushButton*>("btnRefresh");
    QPushButton* btnConnect = findChild<QPushButton*>("btnConnect");
    QLabel* labelTemp = findChild<QLabel*>("labelTemp");

    // Create if missing
    if (!comboPorts) {
        comboPorts = new QComboBox(central);
        comboPorts->setObjectName("comboPorts");
    }
    if (!btnRefresh) {
        btnRefresh = new QPushButton("Refresh", central);
        btnRefresh->setObjectName("btnRefresh");
    }
    if (!btnConnect) {
        btnConnect = new QPushButton("Connect", central);
        btnConnect->setObjectName("btnConnect");
    }
    if (!labelTemp) {
        labelTemp = new QLabel("Temp: -- °C", central);
        labelTemp->setObjectName("labelTemp");
        QFont f = labelTemp->font();
        f.setPointSize(20);
        f.setBold(true);
        labelTemp->setFont(f);
        labelTemp->setMinimumHeight(60);
        labelTemp->setAlignment(Qt::AlignCenter);
    }

    // If Designer already provided a layout, try to insert our controls into it.
    // Otherwise create a simple vertical layout.
    if (!central->layout()) {
        QVBoxLayout* mainLayout = new QVBoxLayout(central);
        QHBoxLayout* topLayout = new QHBoxLayout;
        topLayout->addWidget(comboPorts);
        topLayout->addWidget(btnRefresh);
        topLayout->addWidget(btnConnect);

        mainLayout->addLayout(topLayout);
        mainLayout->addWidget(labelTemp);
        central->setLayout(mainLayout);
    }
}

// Refresh serial ports list
void Room_Tempature::refreshPorts()
{
    QComboBox* combo = findChild<QComboBox*>("comboPorts");
    if (!combo) return;

    combo->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info : ports) {
        QString display = info.portName();
        if (!info.description().isEmpty())
            display += " - " + info.description();
        combo->addItem(display, info.portName());
    }

    if (combo->count() == 0) {
        combo->addItem("No ports found");
        auto btnConnect = findChild<QPushButton*>("btnConnect");
        if (btnConnect) btnConnect->setEnabled(false);
    }
    else {
        auto btnConnect = findChild<QPushButton*>("btnConnect");
        if (btnConnect) btnConnect->setEnabled(true);
    }
}

// Connect or disconnect
void Room_Tempature::connectOrDisconnect()
{
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info : ports) {
        QString desc = info.description().toLower();
        QString vend = info.manufacturer().toLower();
        QString name = info.portName();
        if (desc.contains("arduino") || vend.contains("arduino") ||
            desc.contains("ch340") || vend.contains("wch") ||
            desc.contains("usb")) {
         
        }
    }
    if (m_serial->isOpen()) {
        m_serial->close();
        setConnectedUi(false);
        return;
    }

    QComboBox* combo = findChild<QComboBox*>("comboPorts");
    if (!combo || combo->count() == 0) {
        QMessageBox::warning(this, "Port", "No serial port selected.");
        return;
    }

    QString portName = combo->currentData().toString();
    if (portName.isEmpty()) {
        // maybe the displayed text was portName itself
        portName = combo->currentText();
    }

    m_serial->setPortName(portName);
    m_serial->setBaudRate(QSerialPort::Baud9600);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Serial", "Failed to open port: " + m_serial->errorString());
        setConnectedUi(false);
        return;
    }

    m_readBuffer.clear();
    setConnectedUi(true);
}

// Read serial and buffer lines
void Room_Tempature::handleReadyRead()
{
    m_readBuffer.append(m_serial->readAll());

    while (true) {
        int idx = m_readBuffer.indexOf('\n');
        if (idx == -1) break;
        QByteArray line = m_readBuffer.left(idx);
        m_readBuffer.remove(0, idx + 1);
        QString s = QString::fromUtf8(line).trimmed();
        if (!s.isEmpty()) parseLine(s);
    }
}

void Room_Tempature::parseLine(const QString& line)
{
    QLabel* labelTemp = findChild<QLabel*>("labelTemp");
    if (!labelTemp) return;

    // Handle formats like "TEMP:24.3" or "Temperature: 24.3 °C"
    if (line.startsWith("TEMP:", Qt::CaseInsensitive)) {
        bool ok = false;
        double val = line.mid(5).toDouble(&ok);
        if (ok) {
            labelTemp->setText(QString("Temp: %1 °C").arg(QString::number(val, 'f', 1)));
            return;
        }
    }

    // otherwise extract the first floating number from the line
    QRegularExpression rx(R"(([-+]?[0-9]*\.?[0-9]+))");
    auto match = rx.match(line);
    if (match.hasMatch()) {
        double val = match.captured(1).toDouble();
        labelTemp->setText(QString("Temp: %1 °C").arg(QString::number(val, 'f', 1)));
    }
    else {
        qDebug() << "Serial line (unparsed):" << line;
    }
}

void Room_Tempature::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, "Serial Error", m_serial->errorString());
        m_serial->close();
        setConnectedUi(false);
    }
}

void Room_Tempature::setConnectedUi(bool connected)
{
    QPushButton* btnConnect = findChild<QPushButton*>("btnConnect");
    QComboBox* combo = findChild<QComboBox*>("comboPorts");
    QPushButton* btnRefresh = findChild<QPushButton*>("btnRefresh");
    QLabel* labelTemp = findChild<QLabel*>("labelTemp");

    if (connected) {
        if (btnConnect) btnConnect->setText("Disconnect");
        if (combo) combo->setEnabled(false);
        if (btnRefresh) btnRefresh->setEnabled(false);
        if (labelTemp && labelTemp->text().contains("--")) {
            // keep waiting for first reading
        }
    }
    else {
        if (btnConnect) btnConnect->setText("Connect");
        if (combo) combo->setEnabled(true);
        if (btnRefresh) btnRefresh->setEnabled(true);
        if (labelTemp) labelTemp->setText("Temp: -- °C");
    }
}
