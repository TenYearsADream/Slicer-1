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
    readstlfile.cpp \
    shapediameterfunction.cpp \
    hierarchicalclustering.cpp \
    Slice.cpp \
    dataset.cpp

HEADERS  += mainwindow.h \
    myglwidget.h \
    readstlfile.h \
    shapediameterfunction.h \
    hierarchicalclustering.h \
    Slice.h \
    dataset.h

DISTFILES += \
    resource/open-file.png

RESOURCES += \
    res.qrc
QT +=opengl

windows {
     DEFINES *= Q_COMPILER_INITIALIZER_LISTS
}

INCLUDEPATH += D:/Qt/CGAL-4.11/include
INCLUDEPATH += D:/Qt/CGAL-4.11/auxiliary/gmp/include
INCLUDEPATH += D:/Qt/CGAL/include
INCLUDEPATH += D:/Qt/boost_1_59_0

win32: LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL.dll
win32: LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL_ImageIO.dll
win32: LIBS += -LD:/Qt/CGAL/lib/ -llibCGAL_Core.dll
win32: LIBS += -LD:/Qt/CGAL-4.11/auxiliary/gmp/lib/ -llibgmp-10

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-variable

win32: LIBS += -L'C:/Program Files (x86)/AMD APP/lib/x86/' -lOpenCL

INCLUDEPATH += 'C:/Program Files (x86)/AMD APP/include'
DEPENDPATH += 'C:/Program Files (x86)/AMD APP/include'
