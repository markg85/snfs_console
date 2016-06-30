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
    server.cpp \
    filejob.cpp \
    benchmarkjob.cpp

HEADERS += \
    client.h \
    server.h \
    filejob.h \
    benchmarkjob.h
