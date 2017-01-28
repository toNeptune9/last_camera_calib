#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_cal_clicked()
{
    // boardSize
    QString width = ui->lineEdit_width->text();
    width.toInt();
    QString height = ui->lineEdit_height->text();

    camera_cal.set_boardSize( width.toInt(), height.toInt());

    //Pattern
    if (ui->radioButton_CHESS->isChecked())
    {
//        camera_cal.set_calibrationPattern(Settings::CHESSBOARD);
//        if (camera_cal.get_calibrationPattern()== 1)
//                  ui->plainTextEdit_chess->setPlainText("yeah");
    }
    if (ui->radioButton_CIRCLE->isChecked())
    {
          camera_cal.set_calibrationPattern(Settings::CIRCLES_GRID);
    }
    else
     {
          camera_cal.set_calibrationPattern(Settings::ASYMMETRIC_CIRCLES_GRID);
     }



    // Square Size
     QString square_s = ui->lineEdit_size->text();

     camera_cal.set_squareSize(square_s.toFloat());

     //input type (list img)
     if(ui->tabWidget_input->currentIndex()==0)
       {
           camera_cal.set_inputType(Settings::CAMERA);
       }
     else
     {
             camera_cal.set_inputType(Settings::IMAGE_LIST);
      }

     //number of frames
       QString number_f = ui->lineEdit_number->text();
       camera_cal.set_nrFrames( number_f.toInt());


       // bool detection f. points, write ext param, show undist
         camera_cal.set_bwritePoints(ui->checkBox_det);
         camera_cal.set_bwriteExtrinsics(ui->checkBox_extr);
         camera_cal.set_showUndistorsed(ui->checkBox_undist);

         camera_cal.set_aspectRatio(1);
         camera_cal.set_delay(100);
         camera_cal.set_calibZeroTangentDist(1);
         camera_cal.set_calibFixPrincipalPoint(1);
         camera_cal.set_flipVertical(0);
         // CALIBRATE
         camera_cal.calibrate();
}

void Widget::on_pushButton_clicked()
{
    QStringList file_name = QFileDialog::getOpenFileNames(this,"open a file");
        ui->plainTextEdit->setPlainText(file_name.join("\n"));
        vector<string> * vs = camera_cal.get_imageList();
        camera_cal.set_input(file_name.join("\n").toStdString());
        vs->clear();
        foreach( QString str, file_name) {
            vs->push_back(str.toStdString());
        }
}

void Widget::on_radioButton_usb_clicked()
{
    QString str2;
        str2.resize(1);
        str2[0]=QChar ('0');
         ui->plainTextEdit_touch->setPlainText(str2);
         camera_cal.set_input(str2.toStdString());
         if (!camera_cal.get_input().empty())
             ui->plainTextEdit_input->setPlainText("not null");
}

void Widget::on_radioButton_CHESS_clicked()
{
    camera_cal.set_calibrationPattern(Settings::CHESSBOARD);
    if (camera_cal.get_calibrationPattern()== 1)
              ui->plainTextEdit_chess->setPlainText("yeah its chess");
}
