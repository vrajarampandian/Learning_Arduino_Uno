#include "Room_Tempature.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Room_Tempature window;
    window.show();
    return app.exec();
}
