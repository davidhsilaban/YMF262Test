#-------------------------------------------------
#
# Project created by QtCreator 2015-06-11T16:28:01
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = YMF262Test
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    ymf262io.cpp \
    ymf262.c \
    playthread.cpp \
    synththread.cpp \
    midifilereader.cpp

HEADERS  += mainwindow.h \
    driver.h \
    ymf262.h \
    ymf262io.h \
    playthread.h \
    synththread.h \
    midifilereader.h

LIBS    += -lwinmm

FORMS    += mainwindow.ui
