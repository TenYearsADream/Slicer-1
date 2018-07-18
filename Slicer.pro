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
    mytablewidget.cpp \
    point3f.cpp \
    readstlfile.cpp

HEADERS  += mainwindow.h \
    myglwidget.h \
    mytablewidget.h \
    point3f.h \
    readstlfile.h

DISTFILES += \
    resource/open-file.png

RESOURCES += \
    res.qrc
QT +=opengl

windows {
     DEFINES *= Q_COMPILER_INITIALIZER_LISTS
}

INCLUDEPATH += E:/ProgramFiles/boost_1_59_0 \
                E:/ProgramFiles/CGAL/include \

INCLUDEPATH += $$PWD/bin
DEPENDPATH += $$PWD/bin
LIBS += -L$$PWD/bin/ -lCGAL-vc120-mt-4.11
LIBS += -L$$PWD/bin/ -lCGAL_Core-vc120-mt-4.11
LIBS += -L$$PWD/bin/ -lCGAL_ImageIO-vc120-mt-4.11
LIBS += -L$$PWD/bin/ -lCGAL_Qt5-vc120-mt-4.11
LIBS += -L$$PWD/bin/ -llibboost_system-vc120-mt-1_59
LIBS += -L$$PWD/bin/ -llibboost_thread-vc120-mt-1_59
