#-------------------------------------------------
#
# Project created by QtCreator 2018-07-11T18:39:06
#
#-------------------------------------------------

QT       += core gui
QT += concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Slicer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    myglwidget.cpp \
    readstlfile.cpp \
    shapediameterfunction.cpp \
    hierarchicalclustering.cpp \
    Slice.cpp \
    dataset.cpp \
    opencl.cpp

HEADERS  += mainwindow.h \
    myglwidget.h \
    readstlfile.h \
    shapediameterfunction.h \
    hierarchicalclustering.h \
    Slice.h \
    dataset.h \
    Polygon_mesh_slicer_mine.h \
    opencl.h

DISTFILES += \
    resource/open-file.png

RESOURCES += \
    res.qrc
QT +=opengl

windows {
     DEFINES *= Q_COMPILER_INITIALIZER_LISTS
}

INCLUDEPATH += D:/Qt/boost_1_59_0
INCLUDEPATH += 'D:/Qt/OpenCL/sdk/include'

LIBS += -LD:/Qt/boost_1_59_0/stage/lib/ -llibboost_system-mgw73-mt-1_59
LIBS += -LD:/Qt/boost_1_59_0/stage/lib/ -llibboost_thread-mgw73-mt-1_59
INCLUDEPATH += D:/Qt/CGAL-4.11_64bit/include
INCLUDEPATH += D:/Qt/CGAL-4.11_64bit/auxiliary/gmp/include
INCLUDEPATH += D:/Qt/CGAL_64bit/include

LIBS += -LD:/Qt/CGAL_64bit/lib/ -llibCGAL
LIBS += -LD:/Qt/CGAL_64bit/lib/ -llibCGAL_ImageIO
LIBS += -LD:/Qt/CGAL_64bit/lib/ -llibCGAL_Core
LIBS += -LD:/Qt/CGAL-4.11_64bit/auxiliary/gmp/lib/ -llibgmp-10
LIBS += -L'D:/Qt/OpenCL/sdk/lib/x64' -lOpenCL

#INCLUDEPATH += D:/Qt/CGAL-4.11/include
#INCLUDEPATH += D:/Qt/CGAL-4.11/auxiliary/gmp/include
#INCLUDEPATH += D:/Qt/CGAL/include

#LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL
#LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL_ImageIO
#LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL_Core
#LIBS += -LD:/Qt/CGAL-4.11/auxiliary/gmp/lib/ -llibgmp-10
#LIBS += -L'D:/Qt/OpenCL/sdk/lib/x86' -lOpenCL

#LIBS += -LD:/Qt/boost_1_59_0/stage/lib32/ -llibboost_system-mgw53-mt-1_59
#LIBS += -LD:/Qt/boost_1_59_0/stage/lib32/ -llibboost_thread-mgw53-mt-1_59
