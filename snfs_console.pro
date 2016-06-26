QT += core
QT -= gui
QT += network

CONFIG += c++11

TARGET = snfs_console
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    client.cpp \
    server.cpp

HEADERS += \
    client.h \
    server.h
