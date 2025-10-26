#pragma once

#include <QWidget>
#include <QTimer>
#include <QSerialPort>

class QPushButton;
class QLabel;

class TrafficWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrafficWidget(const QString& serialPortName = QString(), QWidget* parent = nullptr);
    ~TrafficWidget() override;

    enum LightState { Green, Yellow, Red };

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override { return QSize(160, 360); }

private slots:
    void onTick();
    void startStop();
    void reset();
    void onSerialData();          // called when serial has data
    void handleStateMessage(const QString& msg);

private:
    void enterState(LightState st);
    void updateCountdownLabel();
    QString autoDetectArduinoPort(); // optional helper

    LightState m_state;
    QTimer  m_timer;
    int     m_remainingMs;
    QLabel* m_countdownLbl;
    QPushButton* m_startStopBtn;
    bool    m_running;

    QSerialPort* m_serial;
    QString m_serialName;

    // durations in milliseconds
    const int GREEN_MS = 10000;
    const int YELLOW_MS = 3000;
    const int RED_MS = 5000;
};
