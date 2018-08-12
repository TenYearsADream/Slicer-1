#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include "myglwidget.h"
#include "mytablewidget.h"
#include "readstlfile.h"
#include "shapediameterfunction.h"

using namespace std;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int nFaceCount=0;
    void showMemoryInfo();
    ReadSTLFile readstl;
private slots:
    void modelSegment();
    void modelSlice();
private:
    void openFile();
    QAction *openAction;
    MyGLWidget *opengl;
    MyTableWidget *tableWidget;
    ShapeDiameterFunction *shapediameterfunction;
    bool ReadASCII(uchar *buffer);
    bool ReadBinary(uchar *buffer);
    QDoubleSpinBox *segSpinBox,*sliceSpinBox;
};

#endif // MAINWINDOW_H
