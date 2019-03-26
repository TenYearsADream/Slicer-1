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
    readofffile.cpp

HEADERS  += mainwindow.h \
    readstlfile.h \
    dataset.h \
    opencl.h \
    meshfix.h \
    myglwidget.h \
    Slice.h \
    readobjfile.h \
    readofffile.h

RESOURCES += \
    res.qrc


INCLUDEPATH += D:/Qt/boost_1_69_0
INCLUDEPATH += D:/Qt/Eigen3.3.7
INCLUDEPATH += D:/Qt/OpenCL/sdk/include

LIBS += -L'D:/Qt/OpenCL/sdk/lib/x64' -lOpenCL

INCLUDEPATH += D:/Qt/CGAL-4.13/include
INCLUDEPATH += D:/Qt/CGAL-4.13/auxiliary/gmp/include
LIBS += -LD:/Qt/CGAL-4.13/auxiliary/gmp/lib/ -llibgmp-10

INCLUDEPATH += D:/Qt/CGAL/include
LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL
LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL_ImageIO
LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL_Core
LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL_Qt5

QMAKE_CXXFLAGS_RELEASE += -O3

#INCLUDEPATH += $$PWD/../../Qt/CGAL_VS/include
#DEPENDPATH += $$PWD/../../Qt/CGAL_VS/include

#LIBS += -L$$PWD/../../Qt/CGAL_VS/lib/ -lCGAL_Core-vc140-mt-4.13
#LIBS += -L$$PWD/../../Qt/CGAL_VS/lib/ -lCGAL_Core-vc140-mt-gd-4.13

#LIBS += -L$$PWD/../../Qt/CGAL_VS/lib/ -lCGAL-vc140-mt-4.13
#LIBS += -L$$PWD/../../Qt/CGAL_VS/lib/ -lCGAL-vc140-mt-gd-4.13

#LIBS += -L$$PWD/../../Qt/CGAL_VS/lib/ -lCGAL_Qt5-vc140-mt-4.13
#LIBS += -L$$PWD/../../Qt/CGAL_VS/lib/ -lCGAL_Qt5-vc140-mt-gd-4.13
