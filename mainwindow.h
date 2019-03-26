#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include "myglwidget.h"
#include "readstlfile.h"
#include "dataset.h"
#include "Slice.h"
#include "meshfix.h"
using namespace std;
class ReadOBJFile;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void closeEvent(QCloseEvent *event);
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showMemoryInfo();
    dataSet dataset;
signals:
    void outputMsg(QString);
private slots:
//    void modelSegment();
    void modelSlice();
    void modelPlace();
    void OpenStlFile();
    void modelRepair();
    void saveFile();
    void timeUpdate();
private:
    QAction *openStl,*saveStl;
    ReadSTLFile readstl;
    ReadOBJFile *readobj;
    MyGLWidget *opengl;
    QWidget *toolWidget;
    QVBoxLayout *toollayout;

    Slice slice;
    MeshFix meshfix;
    QDoubleSpinBox *sliceSpinBox,*segSpinBox;
    QSlider *layerSlider ;
    QSpinBox *layerSpinBox;
    QSpinBox *placeSpinBoxx,*placeSpinBoxy,*placeSpinBoxz;
    QCheckBox *isAdapt;
    QComboBox *comboBox;
    QTextEdit *logEdit;

    QString slicepath;

private:
    QLabel* statusLabel; //声明标签对象，用于显示状态信息
    QLabel *currentTimeLabel;//显示时间
    void initStatusBar(); //初始化状态栏
    void initPlaceWidget();
    void initSliceWidget();
    void initSegmentWidget();
    void initMenuWidget();
};

#endif // MAINWINDOW_H
