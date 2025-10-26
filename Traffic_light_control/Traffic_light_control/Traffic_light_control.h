#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Traffic_light_control.h"

class Traffic_light_control : public QMainWindow
{
    Q_OBJECT

public:
    Traffic_light_control(QWidget *parent = nullptr);
    ~Traffic_light_control();

private:
    Ui::Traffic_light_controlClass ui;
};

