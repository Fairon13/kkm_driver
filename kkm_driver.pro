QT += core
QT -= gui

CONFIG += c++11

TARGET = kkm_driver
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    kkm_device.cpp \
    kkm_protocol.cpp \
    delayclass.cpp \
    httphandler.cpp

INCLUDEPATH += serialport \
               serialport/qt4support/include \
               serialport/qt4support/include/private \
               serialport/qt4support/include/QtCore

include(serialport/serialport-lib.pri)
include(serialport/qt4support/install-helper.pri)
include(httpserver/httpserver.pri)

LIBS += -lqjson

HEADERS += \
    kkm_device.h \
    kkm_protocol.h \
    delayclass.h \
    httphandler.h
