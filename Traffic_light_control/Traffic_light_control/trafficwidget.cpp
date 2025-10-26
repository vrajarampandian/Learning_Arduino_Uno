#include "trafficwidget.h"
#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QSerialPortInfo>

TrafficWidget::TrafficWidget(const QString& serialPortName, QWidget* parent)
    : QWidget(parent),
    m_state(Green),
    m_timer(this),
    m_remainingMs(0),
    m_countdownLbl(nullptr),
    m_startStopBtn(nullptr),
    m_running(false),
    m_serial(nullptr),
    m_serialName(serialPortName)
{
    auto* mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(8, 8, 8, 8);
    mainLay->setSpacing(8);

    m_countdownLbl = new QLabel(this);
    m_countdownLbl->setAlignment(Qt::AlignCenter);
    m_countdownLbl->setText("Stopped");
    mainLay->addWidget(m_countdownLbl);

    auto* btnLay = new QHBoxLayout();
    m_startStopBtn = new QPushButton(tr("Start"), this);
    auto* resetBtn = new QPushButton(tr("Reset"), this);
    btnLay->addWidget(m_startStopBtn);
    btnLay->addWidget(resetBtn);
    mainLay->addLayout(btnLay);

    connect(&m_timer, &QTimer::timeout, this, &TrafficWidget::onTick);
    connect(m_startStopBtn, &QPushButton::clicked, this, &TrafficWidget::startStop);
    connect(resetBtn, &QPushButton::clicked, this, &TrafficWidget::reset);

    m_timer.setInterval(100);

    // initialize display state (not running)
    enterState(Green);

    // Serial setup: if caller didn't pass a port name, try auto-detect
    if (m_serialName.isEmpty()) {
        m_serialName = autoDetectArduinoPort();
    }

    if (!m_serialName.isEmpty()) {
        // Attempt to open port as ReadWrite (so we can send REQ and receive)
        m_serial = new QSerialPort(m_serialName, this);
        m_serial->setBaudRate(QSerialPort::Baud115200);
        m_serial->setDataBits(QSerialPort::Data8);
        m_serial->setParity(QSerialPort::NoParity);
        m_serial->setStopBits(QSerialPort::OneStop);
        m_serial->setFlowControl(QSerialPort::NoFlowControl);

        if (m_serial->open(QIODevice::ReadWrite)) {
            connect(m_serial, &QSerialPort::readyRead, this, &TrafficWidget::onSerialData);
            // Request immediate state from Arduino
            if (m_serial->isWritable()) {
                m_serial->write("REQ\n");
                m_serial->flush();
            }
            // Make widget follow hardware immediately
            m_running = true;
            m_startStopBtn->setText(tr("Stop"));

            // Start periodic ticking (so countdown updates)
            if (!m_timer.isActive()) m_timer.start();
        }
        else {
            QString err = m_serial->errorString();
            m_countdownLbl->setText(tr("Serial open failed: %1").arg(err));
            delete m_serial;
            m_serial = nullptr;
        }
    }
    else {
        m_countdownLbl->setText(tr("Serial port not found"));
    }
}

TrafficWidget::~TrafficWidget()
{
    if (m_serial) {
        if (m_serial->isOpen()) m_serial->close();
    }
}

QString TrafficWidget::autoDetectArduinoPort()
{
    // Try to find a port whose description/manufacturer contains "Arduino" or "CH340" etc.
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info : ports) {
        QString desc = info.description().toLower();
        QString vend = info.manufacturer().toLower();
        QString name = info.portName();
        if (desc.contains("arduino") || vend.contains("arduino") ||
            desc.contains("ch340") || vend.contains("wch") ||
            desc.contains("usb")) {
            return name;
        }
    }
    // fallback: return first port if any
    if (!ports.isEmpty()) return ports.first().portName();
    return QString();
}

void TrafficWidget::onSerialData()
{
    if (!m_serial) return;

    // Read all available lines
    while (m_serial->canReadLine()) {
        QByteArray raw = m_serial->readLine().trimmed();
        if (raw.isEmpty()) continue;

        // show raw data in label for debugging (temporary)
        QString dbg = QString::fromUtf8(raw);
        // Optionally append rather than replace:
        m_countdownLbl->setText(dbg);

        // Parse expected format: STATE:COLOR:REMAIN
        QString s = QString::fromUtf8(raw);
        if (s.startsWith("STATE:", Qt::CaseInsensitive)) {
            QString payload = s.mid(6); // everything after "STATE:"
            QStringList parts = payload.split(':');
            QString color = parts.value(0).toUpper();
            int remain = parts.value(1).toInt(); // 0 if missing

            // Set internal state and remaining ms
            if (color == "GREEN") {
                m_state = Green;
            }
            else if (color == "YELLOW") {
                m_state = Yellow;
            }
            else if (color == "RED") {
                m_state = Red;
            }
            m_remainingMs = qMax(0, remain);

            // Ensure UI reflects hardware immediately
            m_running = true;
            if (!m_timer.isActive()) m_timer.start();
            m_startStopBtn->setText(tr("Stop"));

            updateCountdownLabel();
            update(); // repaint immediately
        }
        else {
            // unknown message - you can log it or ignore
        }
    }
}

void TrafficWidget::handleStateMessage(const QString& msg)
{
    // Expect messages like "STATE:GREEN" or "STATE:YELLOW" or "STATE:RED"
    if (msg.startsWith("STATE:", Qt::CaseInsensitive)) {
        QString part = msg.mid(6).toUpper();
        if (part == "GREEN") {
            enterState(Green);
            // keep running
            m_running = true;
            m_startStopBtn->setText(tr("Stop"));
        }
        else if (part == "YELLOW") {
            enterState(Yellow);
            m_running = true;
            m_startStopBtn->setText(tr("Stop"));
        }
        else if (part == "RED") {
            enterState(Red);
            m_running = true;
            m_startStopBtn->setText(tr("Stop"));
        }
    }
    else {
        // unknown message — ignore or log
    }
}

void TrafficWidget::enterState(LightState st)
{
    m_state = st;
    switch (m_state) {
    case Green:  m_remainingMs = GREEN_MS;  break;
    case Yellow: m_remainingMs = YELLOW_MS; break;
    case Red:    m_remainingMs = RED_MS;    break;
    }
    updateCountdownLabel();
    update();
}

void TrafficWidget::updateCountdownLabel()
{
    if (!m_running) {
        m_countdownLbl->setText(tr("Stopped"));
        return;
    }
    int seconds = (m_remainingMs + 999) / 1000;
    QString text;
    switch (m_state) {
    case Green:  text = tr("GREEN: %1 s").arg(seconds); break;
    case Yellow: text = tr("YELLOW: %1 s").arg(seconds); break;
    case Red:    text = tr("RED: %1 s").arg(seconds); break;
    }
    m_countdownLbl->setText(text);
}

void TrafficWidget::startStop()
{
    m_running = !m_running;
    if (m_running) {
        m_startStopBtn->setText(tr("Stop"));
        if (!m_timer.isActive()) m_timer.start();
    }
    else {
        m_startStopBtn->setText(tr("Start"));
        if (m_timer.isActive()) m_timer.stop();
    }
    updateCountdownLabel();
}

void TrafficWidget::reset()
{
    bool wasRunning = m_running;
    m_running = false;
    m_startStopBtn->setText(tr("Start"));
    if (m_timer.isActive()) m_timer.stop();
    enterState(Green);
    update();
    Q_UNUSED(wasRunning);
}

void TrafficWidget::onTick()
{
    if (!m_running) return;
    m_remainingMs -= m_timer.interval();
    if (m_remainingMs <= 0) {
        // if hardware is driving states (via serial), prefer hardware announcements;
        // but we still advance logically if needed
        switch (m_state) {
        case Green:  enterState(Yellow); break;
        case Yellow: enterState(Red);    break;
        case Red:    enterState(Green);  break;
        }
    }
    else {
        updateCountdownLabel();
    }
    update();
}

void TrafficWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRectF casing(20, 40, width() - 40, height() - 80);
    p.setBrush(Qt::black);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(casing, 10, 10);

    qreal cx = casing.center().x();
    qreal top = casing.top() + 16;
    qreal spacing = 16;
    qreal diameter = qMin(casing.width() - 40, (casing.height() - 4 * spacing) / 3.0);
    qreal r = diameter / 2.0;
    qreal x = cx - r;

    auto drawLight = [&](qreal y, const QColor& color, bool on) {
        QRectF rect(x, y, diameter, diameter);
        if (on) {
            QRadialGradient grad(rect.center(), r);
            grad.setColorAt(0.0, color.lighter(130));
            grad.setColorAt(1.0, color.darker(150));
            p.setBrush(grad);
            p.setPen(Qt::NoPen);
            p.drawEllipse(rect);
            p.setPen(QPen(Qt::white, 1));
            p.drawEllipse(rect.adjusted(4, 4, -4, -4));
        }
        else {
            QColor dim = color.darker(300);
            p.setBrush(dim);
            p.setPen(Qt::NoPen);
            p.drawEllipse(rect);
        }
        };

    qreal y0 = top;
    drawLight(y0, QColor("#2ecc71"), (m_state == Green));
    drawLight(y0 + diameter + spacing, QColor("#f1c40f"), (m_state == Yellow));
    drawLight(y0 + 2 * (diameter + spacing), QColor("#e74c3c"), (m_state == Red));
}
