#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "cameracalibration.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pushButton_cal_clicked();

    void on_pushButton_clicked();

    void on_radioButton_usb_clicked();

    void on_radioButton_CHESS_clicked();

private:
    Ui::Widget *ui;
    CameraCalibrate camera_cal;
};

#endif // WIDGET_H
