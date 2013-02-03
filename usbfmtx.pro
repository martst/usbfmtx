#-------------------------------------------------
#
# Project created by QtCreator 2013-01-27T13:03:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = usbfmtx
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

unix:LIBS += -ludev

RESOURCES += \
    icons.qrc
