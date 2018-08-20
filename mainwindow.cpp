#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextEdit>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QSplitter>
#include <QTableView>
#include <QDebug>
#include <Qtime>
#include <QLabel>
#include <iostream>
#include "windows.h"
#include"WinBase.h"
#include "Psapi.h"
#include "mainwindow.h"
#include "hierarchicalclustering.h"
#include "Slice.h"
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Slicer"));
    setMinimumSize(1000,640);
    openAction = new QAction(QIcon(":/images/file-open"), tr("&Open..."), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));

    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    QMenu *file = menuBar()->addMenu(tr("&File"));
    file->addAction(openAction);

    statusBar() ;
    QWidget *cenWidget = new QWidget(this); //this is point to QMainWindow
    this->setCentralWidget(cenWidget);

    opengl= new MyGLWidget(cenWidget);
    //tableWidget =new MyTableWidget((cenWidget));
    QPushButton *segmentButton = new QPushButton(tr("segment"),(cenWidget));
    QPushButton *sliceButton = new QPushButton(tr("slice"),(cenWidget));

    sliceSpinBox = new QDoubleSpinBox(cenWidget);
    sliceSpinBox->setRange(0.1, 20);  // 范围
    sliceSpinBox->setDecimals(2);  // 精度
    sliceSpinBox->setSingleStep(0.05); // 步长

    layerSlider = new QSlider(Qt::Horizontal);
    QLabel *layerLable = new QLabel();
    layerLable->setText("layer:");
    layerSpinBox=new QSpinBox(cenWidget);
    layerSpinBox->setValue(0);

    segSpinBox = new QDoubleSpinBox(cenWidget);
    segSpinBox->setRange(0.1, 20);  // 范围
    segSpinBox->setDecimals(2);  // 精度
    segSpinBox->setSingleStep(0.01); // 步长

    connect(segmentButton,SIGNAL(clicked()),this,SLOT(modelSegment()));
    connect(sliceButton,SIGNAL(clicked()),this,SLOT(modelSlice()));

    QHBoxLayout *mainlayout = new QHBoxLayout(cenWidget);
    QVBoxLayout *rightlayout = new QVBoxLayout();

    QHBoxLayout *layerlayout = new QHBoxLayout();

    //rightlayout->addWidget(tableWidget);
    rightlayout->addWidget(sliceButton,0,Qt::AlignCenter);
    rightlayout->addWidget(sliceSpinBox,0,Qt::AlignCenter);

    layerlayout->addWidget(layerLable);
    layerlayout->addWidget(layerSlider);
    layerlayout->addWidget(layerSpinBox);
    QObject::connect(layerSpinBox,SIGNAL(valueChanged(int)),layerSlider,SLOT(setValue(int))); //将spinBox的数值传向slider信号槽
    QObject::connect(layerSlider,SIGNAL(valueChanged(int)),layerSpinBox,SLOT(setValue(int)));//将slider的数值传向spinBox信号槽
    QObject::connect(layerSlider,SIGNAL(valueChanged(int)),opengl,SLOT(setLayer(int)));
    rightlayout->addLayout(layerlayout);


    rightlayout->addWidget(segmentButton,0,Qt::AlignCenter);
    rightlayout->addWidget(segSpinBox,0,Qt::AlignCenter);

    cenWidget->setLayout(mainlayout);
    mainlayout->addWidget(opengl);
    mainlayout->addLayout(rightlayout);
    mainlayout->setStretchFactor(opengl, 4);
    mainlayout->setStretchFactor(rightlayout,2);
    setStatusTip(tr("ready"));

}

MainWindow::~MainWindow()
{

}

void MainWindow::showMemoryInfo()
{
    HANDLE handle=GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(handle,&pmc,sizeof(pmc));
    qDebug()<<"using memory:"<<pmc.WorkingSetSize/1000000<<"MB"<<endl;
}

void MainWindow::openFile()
{
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Open File"),
                                                ".",
                                               tr("Text Files(*.stl)"));
    const char* filepath=path.toStdString().c_str();//QString转化为string类型，然后由string转化char*
    if(!path.isEmpty()) {
        QTime time;
        time.start();
        readstl.ReadStlFile(filepath);
        qDebug()<<"time of readstl:"<<time.elapsed()/1000.0<<"s";
        //qDebug()<<"number of faces:"<<readstl->NumTri()<<endl;
        //qDebug()<<readstl.surroundBox[1]<<endl;
        time.start();
//        tableWidget->setRowCount(readstl.NumTri());
//        tableWidget->setData(readstl.hashtable->vertices,readstl.faceList);
//        tableWidget->show();
//        qDebug()<<"time of table:"<<time.elapsed()/1000.0<<"s";
        time.start();
        dataset=new dataSet(readstl.hashtable->vertices,readstl.faceList);
        for(int i=0;i<6;i++)
        {
            dataset->surroundBox[i]=readstl.surroundBox[i];
        }
        qDebug()<<"number of vertices:"<<dataset->mesh.number_of_vertices();
        qDebug()<<"number of faces:"<<dataset->mesh.number_of_faces();
        qDebug()<<"number of normals:"<<readstl.normalList.size();
        opengl->xtrans=-(dataset->surroundBox[1]+dataset->surroundBox[0])/2.0;
        opengl->ytrans=(dataset->surroundBox[2]+dataset->surroundBox[3])/2.0;
        opengl->ztrans=1.0/(qMax(qAbs(dataset->surroundBox[4]),qAbs(dataset->surroundBox[5])));
        opengl->clusterTable.clear();
        opengl->vertices.clear();
        opengl->indices.clear();
        opengl->intrpoints.clear();      
        opengl->vertices=dataset->vertices;
        opengl->indices=dataset->indices;
        qDebug()<<"time of OpenGl:"<<time.elapsed()/1000.0/1000.0<<"s";
        showMemoryInfo();

    } else {
        QMessageBox::warning(this, tr("Path"),
                             tr("You did not select any file."));
    }
}

void MainWindow::modelSegment()
{
    opengl->intrpoints.clear();
    HierarchicalClustering hierarchicalclustering;
    double esp=segSpinBox->value();
    if(!readstl.faceList.empty())
    {
        vector<vector<double>> charValue=shapediameterfunction->calculateSDF(dataset->mesh);
//        for(int i=0;i<charValue.size();i++)
//        {
//            cout<<charValue[i][0]<<" "<<charValue[i][1]<<endl;
//        }
        opengl->clusterTable=hierarchicalclustering.Cluster(charValue,esp);

    } else {
        QMessageBox::warning(this, tr("error"),
                             tr("You did not import any model."));
    }

}

void MainWindow::modelSlice()
{
    Slice slice;
    opengl->clusterTable.clear();
    opengl->intrpoints.clear();
    layerSlider->setValue(1);
    slice.thick=sliceSpinBox->value();
    if(!readstl.faceList.empty())
    {
        slice.mesh=dataset->mesh;
        slice.intrPoints(dataset->surroundBox[4],dataset->surroundBox[5]);
        opengl->intrpoints=slice.intrpoints;
        cout<<"number of layers:"<<slice.layernumber<<endl;
        layerSlider->setRange(1,slice.layernumber);
        layerSpinBox->setRange(1,slice.layernumber);

    } else {
        QMessageBox::warning(this, tr("error"),
                             tr("You did not import any model."));
    }
}
