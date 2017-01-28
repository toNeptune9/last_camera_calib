#-------------------------------------------------
#
# Project created by QtCreator 2017-01-24T17:06:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Camera_Calibration_GUI
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    cameracalibration.cpp

HEADERS  += widget.h \
    cameracalibration.h

FORMS    += \
    widget.ui

#LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_features2d -lopencv_highgui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv

