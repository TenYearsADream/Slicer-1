#-------------------------------------------------
#
# Project created by QtCreator 2018-07-11T18:39:06
#
#-------------------------------------------------

QT       += core gui xml
QT +=opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Slicer
TEMPLATE = app

CONFIG += c++11
#CONFIG +=debug
#CONFIG +=release

SOURCES += main.cpp\
        mainwindow.cpp \
    readstlfile.cpp \
    dataset.cpp \
    opencl.cpp \
    meshfix.cpp \
    myglwidget.cpp \
    Slice.cpp \
    readobjfile.cpp \
    readofffile.cpp \
    loadprogressbar.cpp

HEADERS  += mainwindow.h \
    readstlfile.h \
    dataset.h \
    opencl.h \
    meshfix.h \
    myglwidget.h \
    Slice.h \
    readobjfile.h \
    readofffile.h \
    loadprogressbar.h

RESOURCES += \
    res.qrc


INCLUDEPATH += $$PWD/resource/OpenCL/include
LIBS += -L$$PWD/resource/OpenCL/lib/x64/ -lOpenCL

INCLUDEPATH += $$PWD/resource/Eigen3.3.7
INCLUDEPATH += $$PWD/resource/CGAL
INCLUDEPATH += $$PWD/resource/CGAL/include
INCLUDEPATH += $$PWD/resource/CGAL/gmp/include

LIBS += -L$$PWD/resource/CGAL/gmp/lib/ -llibgmp-10

LIBS += -L$$PWD/resource/CGAL/lib/ -llibCGAL.dll
LIBS += -L$$PWD/resource/CGAL/lib/ -llibCGAL_Core.dll
LIBS += -L$$PWD/resource/CGAL/lib/ -llibCGAL_Qt5.dll
LIBS += -L$$PWD/resource/CGAL/lib/ -llibCGAL_ImageIO.dll

QMAKE_CXXFLAGS_RELEASE += -O3



