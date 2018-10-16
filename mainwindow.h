#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include "myglwidget.h"
#include "readstlfile.h"
#include "shapediameterfunction.h"
#include "dataset.h"

using namespace std;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int nFaceCount=0;
    void showMemoryInfo();
    dataSet dataset;
private slots:
    void modelSegment();
    void modelSlice();
    void modelPlace();
    void openFile();
private:
    QAction *openAction;
    ReadSTLFile readstl;
    MyGLWidget *opengl;
    ShapeDiameterFunction *shapediameterfunction;

    QDoubleSpinBox *segSpinBox,*sliceSpinBox;
    QSlider *layerSlider ;
    QSpinBox *layerSpinBox;
    QSpinBox *placeSpinBoxx,*placeSpinBoxy,*placeSpinBoxz;
    QCheckBox *isAdapt,*isParaComp;
};

#endif // MAINWINDOW_H
