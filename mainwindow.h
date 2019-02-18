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
#include "Slice.h"

using namespace std;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showMemoryInfo();
    dataSet dataset;
private slots:
    void modelSegment();
    void modelSlice();
    void modelPlace();
    void openFile();
    void modelRepair();
    void saveFile();
private:
    QAction *openAction,*saveStl;
    ReadSTLFile readstl;
    MyGLWidget *opengl;
    Slice slice;
    ShapeDiameterFunction *shapediameterfunction;

    QDoubleSpinBox *segSpinBox,*sliceSpinBox;
    QSlider *layerSlider ;
    QSpinBox *layerSpinBox;
    QSpinBox *placeSpinBoxx,*placeSpinBoxy,*placeSpinBoxz;
    QCheckBox *isAdapt,*isParaComp;

    QString slicepath;
};

#endif // MAINWINDOW_H
