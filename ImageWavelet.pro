#-------------------------------------------------
#
# Project created by QtCreator 2015-05-03T18:01:20
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageWavelet
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imageviewer.cpp \
    wavelet.cpp \
    imageutils.cpp \
    matrixutils.cpp

HEADERS  += mainwindow.h \
    imageviewer.h \
    wavelet.h \
    performancetimer.h \
    performancetimer.h \
    imageutils.h \
    matrix.h \
    matrixutils.h
