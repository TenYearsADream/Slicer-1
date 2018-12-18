#-------------------------------------------------
#
# Project created by QtCreator 2018-07-11T18:39:06
#
#-------------------------------------------------

QT       += core gui
QT += concurrent
QT +=opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Slicer
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    readstlfile.cpp \
    shapediameterfunction.cpp \
    hierarchicalclustering.cpp \
    dataset.cpp \
    opencl.cpp \
    meshfix.cpp \
    myglwidget.cpp \
    Slice.cpp

HEADERS  += mainwindow.h \
    readstlfile.h \
    shapediameterfunction.h \
    hierarchicalclustering.h \
    dataset.h \
    opencl.h \
    meshfix.h \
    myglwidget.h \
    Slice.h

DISTFILES += \
    resource/open-file.png

RESOURCES += \
    res.qrc


INCLUDEPATH += D:/Qt/boost_1_65_0
INCLUDEPATH += D:/Qt/Eigen3.3.7
INCLUDEPATH += D:/Qt/OpenCL/sdk/include

LIBS += -L'D:/Qt/OpenCL/sdk/lib/x64' -lOpenCL


INCLUDEPATH += D:/Qt/CGAL-4.13/include
INCLUDEPATH += D:/Qt/CGAL-4.13/auxiliary/gmp/include
INCLUDEPATH += D:/Qt/CGAL/include

LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL
LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL_ImageIO
LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL_Core
LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL_Qt5
LIBS += -LD:/Qt/CGAL-4.13/auxiliary/gmp/lib/ -llibgmp-10
QMAKE_CXXFLAGS_RELEASE += -O3

