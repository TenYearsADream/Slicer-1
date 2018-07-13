#-------------------------------------------------
#
# Project created by QtCreator 2018-07-11T18:39:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Slicer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    myglwidget.cpp \
    mytablewidget.cpp

HEADERS  += mainwindow.h \
    myglwidget.h \
    mytablewidget.h

DISTFILES += \
    resource/open-file.png

RESOURCES += \
    res.qrc
QT +=opengl

windows {
     DEFINES *= Q_COMPILER_INITIALIZER_LISTS
}
