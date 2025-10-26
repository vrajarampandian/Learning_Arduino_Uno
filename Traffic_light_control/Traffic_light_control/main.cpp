#include <QApplication>
#include <QMainWindow>
#include "trafficwidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;

    TrafficWidget* widget = new TrafficWidget("COM3", &window);

    window.setCentralWidget(widget);
    window.setWindowTitle("Traffic Light Emulator - Live Arduino Sync");
    window.resize(220, 460);
    window.show();

    return app.exec();
}

