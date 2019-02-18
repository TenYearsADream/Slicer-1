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
#include <QTime>
#include <QLabel>
#include <QProgressDialog>
#include <QThread>
#include <iostream>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include "windows.h"
#include "mainwindow.h"
#include "hierarchicalclustering.h"
#include "Slice.h"
#include "meshfix.h"
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle(tr("Slicer"));
    setMinimumSize(1000,640);

    openAction = new QAction(QIcon(":/images/resource/open-file.png"), tr("&Open..."), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    QMenu *file = menuBar()->addMenu(tr("&File"));
    file->addAction(openAction);

    saveStl = new QAction(QIcon(":/images/resource/save-file.png"), tr("&Save..."), this);
    saveStl->setShortcuts(QKeySequence::Save);
    saveStl->setStatusTip(tr("Save the model"));
    connect(saveStl, &QAction::triggered, this, &MainWindow::saveFile);
    file->addAction(saveStl);


    statusBar() ;
    QWidget *cenWidget = new QWidget(this); //this is point to QMainWindow
    this->setCentralWidget(cenWidget);

    opengl= new MyGLWidget(cenWidget);
    QWidget *toolWidget=new QWidget(cenWidget);
    QHBoxLayout *mainlayout = new QHBoxLayout(cenWidget);
    QVBoxLayout *toollayout = new QVBoxLayout();
    cenWidget->setLayout(mainlayout);
    mainlayout->addWidget(opengl);
    mainlayout->addWidget(toolWidget);
    toolWidget->setLayout(toollayout);
    mainlayout->setStretchFactor(opengl, 4);
    mainlayout->setStretchFactor(toolWidget,2);

    //模型摆放布局
    QLabel *placeLablex = new QLabel();
    placeLablex->setText("Degree of rotation around X axis:");
    placeSpinBoxx = new QSpinBox();
    placeSpinBoxx->setRange(-360, 360);  // 范围
    QLabel *placeLabley = new QLabel();
    placeLabley->setText("Degree of rotation around Y axis:");
    placeSpinBoxy = new QSpinBox();
    placeSpinBoxy->setRange(-360, 360);  // 范围
    QLabel *placeLablez = new QLabel();
    placeLablez->setText("Degree of rotation around Z axis:");
    placeSpinBoxz = new QSpinBox();
    placeSpinBoxz->setRange(-360, 360);  // 范围
    QPushButton *placeButton = new QPushButton(tr("place"));
    connect(placeButton,SIGNAL(clicked()),this,SLOT(modelPlace()));
    QPushButton *repairButton = new QPushButton(tr("repair"));
    connect(repairButton,SIGNAL(clicked()),this,SLOT(modelRepair()));
    QWidget *modelsetQWidget=new QWidget(toolWidget);
    QVBoxLayout *modellayout = new QVBoxLayout();
    QHBoxLayout *modellayout1 = new QHBoxLayout();
    QHBoxLayout *modellayout2 = new QHBoxLayout();
    QHBoxLayout *modellayout3 = new QHBoxLayout();
    QHBoxLayout *modellayout4 = new QHBoxLayout();
    modellayout1->addWidget(placeLablex,0,Qt::AlignCenter);
    modellayout1->addWidget(placeSpinBoxx,0,Qt::AlignCenter);
    modellayout2->addWidget(placeLabley,0,Qt::AlignCenter);
    modellayout2->addWidget(placeSpinBoxy,0,Qt::AlignCenter);
    modellayout3->addWidget(placeLablez,0,Qt::AlignCenter);
    modellayout3->addWidget(placeSpinBoxz,0,Qt::AlignCenter);
    modellayout4->addWidget(placeButton,0,Qt::AlignCenter);
    modellayout4->addWidget(repairButton,0,Qt::AlignCenter);
    modellayout->addLayout(modellayout1);
    modellayout->addLayout(modellayout2);
    modellayout->addLayout(modellayout3);
    modellayout->addLayout(modellayout4);
    modelsetQWidget->setLayout(modellayout);
    toollayout->addWidget(modelsetQWidget);
    toollayout->setStretchFactor(modelsetQWidget,4);

    //分层显示布局
    sliceSpinBox = new QDoubleSpinBox(cenWidget);
    sliceSpinBox->setRange(0.01, 20);  // 范围
    sliceSpinBox->setDecimals(2);  // 精度
    sliceSpinBox->setSingleStep(0.05); // 步长
    QPushButton *sliceButton = new QPushButton(tr("slice"),(cenWidget));
    connect(sliceButton,SIGNAL(clicked()),this,SLOT(modelSlice()));
    layerSlider = new QSlider(Qt::Horizontal);
    QLabel *layerLable = new QLabel();
    layerLable->setText("layer:");
    layerSpinBox=new QSpinBox(cenWidget);
    layerSpinBox->setValue(0);
    isAdapt = new QCheckBox();
    isAdapt->setText("adapt slice");
    isParaComp = new QCheckBox();
    isParaComp->setText("Parallel computing");
    QWidget *layerQWidget=new QWidget(toolWidget);
    //layerQWidget->setStyleSheet("background-color:green;");
    QVBoxLayout *layerlayout = new QVBoxLayout();
    QHBoxLayout *layerlayout1 = new QHBoxLayout();
    layerlayout1->addWidget(sliceButton,0,Qt::AlignCenter);
    layerlayout1->addWidget(sliceSpinBox,0,Qt::AlignCenter);
    QHBoxLayout *layerlayout2 = new QHBoxLayout();
    layerlayout2->addWidget(isAdapt,0,Qt::AlignCenter);
    layerlayout2->addWidget(isParaComp,0,Qt::AlignCenter);
    QHBoxLayout *layerlayout3 = new QHBoxLayout();
    layerlayout3->addWidget(layerLable);
    layerlayout3->addWidget(layerSlider);
    layerlayout3->addWidget(layerSpinBox);
    QObject::connect(layerSpinBox,SIGNAL(valueChanged(int)),layerSlider,SLOT(setValue(int))); //将spinBox的数值传向slider信号槽
    QObject::connect(layerSlider,SIGNAL(valueChanged(int)),layerSpinBox,SLOT(setValue(int)));//将slider的数值传向spinBox信号槽
    QObject::connect(layerSlider,SIGNAL(valueChanged(int)),opengl,SLOT(setLayer(int)));
    layerQWidget->setLayout(layerlayout);
    layerlayout->addLayout(layerlayout1);
    layerlayout->addLayout(layerlayout2);
    layerlayout->addLayout(layerlayout3);
    toollayout->addWidget(layerQWidget);
    toollayout->setStretchFactor(layerQWidget,4);

    //模型分割布局
    segSpinBox = new QDoubleSpinBox(cenWidget);
    segSpinBox->setRange(0.1, 20);  // 范围
    segSpinBox->setDecimals(2);  // 精度
    segSpinBox->setSingleStep(0.01); // 步长
    QPushButton *segmentButton = new QPushButton(tr("segment"),(cenWidget));
    connect(segmentButton,SIGNAL(clicked()),this,SLOT(modelSegment()));
    QWidget *segmentQWidget=new QWidget(toolWidget);
    QHBoxLayout *segmentlayout = new QHBoxLayout();
    segmentlayout->addWidget(segmentButton,0,Qt::AlignCenter);
    segmentlayout->addWidget(segSpinBox,0,Qt::AlignCenter);
    segmentQWidget->setLayout(segmentlayout);
    toollayout->addWidget(segmentQWidget);
    toollayout->setStretchFactor(segmentQWidget,2);

    setStatusTip(tr("ready"));
}

MainWindow::~MainWindow()
{
    delete(openAction);
    delete(opengl);
    delete(shapediameterfunction);
}


void MainWindow::openFile()
{
    QFileInfo fileinfo;
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Open File"),
                                                ".",
                                               tr("Text Files(*.stl)"));    
    if(!path.isEmpty()) {
        QTime time;
        time.start();
        dataset.mesh.clear();
        int index=path.lastIndexOf(".");
        slicepath=path;
        slicepath.truncate(index);
        slice.slicepath[0]=slicepath+"_cpu.slc";
        slice.slicepath[1]=slicepath+"_gpu.slc";
        if(!readstl.ReadStlFile(path,dataset))
        {
            cout<<"Failed to read STL file!"<<endl;
            return;
        }
        qDebug()<<"time of readstl:"<<time.elapsed()/1000.0<<"s";        
        qDebug()<<"number of vertices:"<<dataset.mesh.number_of_vertices();
        qDebug()<<"number of edges:"<<dataset.mesh.number_of_edges();
        qDebug()<<"number of faces:"<<dataset.mesh.number_of_faces();
        qDebug()<<"number of normals:"<<readstl.normalList.size();
        dataset.halfedgeOnGpu();
        dataset.getIndices();
        //dataset.mesh.clear();
        float x=dataset.surroundBox[1]-dataset.surroundBox[0];
        float y=dataset.surroundBox[3]-dataset.surroundBox[2];
        float z=dataset.surroundBox[5]-dataset.surroundBox[4];
        qDebug()<<"surroundBox of the model:"<<x<<"*"<<y<<"*"<<z;
        opengl->xtrans=-(dataset.surroundBox[1]+dataset.surroundBox[0])/2.0f;
        opengl->ytrans=(dataset.surroundBox[2]+dataset.surroundBox[3])/2.0f;
        opengl->ztrans=-z;
        opengl->scale=90.0f/(*max_element(dataset.surroundBox, dataset.surroundBox + 6));
        opengl->clusterTable.clear();
        opengl->vertices.clear();
        opengl->indices.clear();
        opengl->vertexnormals.clear();
        opengl->intrpoints.clear();
//        opengl->vertices=readstl.vertices;
//        opengl->indices=readstl.indices;
        opengl->vertices=dataset.vertices;
        opengl->indices=dataset.indices;
        opengl->vertexnormals=dataset.vertexnormals;

    } else {
        QMessageBox::warning(this, tr("Path"),
                             tr("You did not select any file."));
    }
}

void MainWindow::saveFile()
{
    qDebug()<<"start save file...";
    QString stlfileName=slicepath+"_repaired.stl";
    QFile file(stlfileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Can't open file for writing";
    }
    char name[80]="1234";
    char attribute[2]="w";
    file.write(name,sizeof(name));
    uint facesnumber=dataset.mesh.number_of_faces();
    float x,y,z;
    file.write((char*)(&facesnumber),4);
    auto fnormals =dataset.mesh.add_property_map<Mesh::Face_index, Vector>("f:normals", CGAL::NULL_VECTOR).first;
    CGAL::Polygon_mesh_processing::compute_face_normals(dataset.mesh,fnormals,
           CGAL::Polygon_mesh_processing::parameters::vertex_point_map(dataset.mesh.points()).geom_traits(Kernel()));
    for(Mesh::Face_index f:dataset.mesh.faces())
    {
        //cout<<f<<"--"<<endl;
        //cout<<fnormals[f]<<endl;
        x=fnormals[f].x();y=fnormals[f].y();z=fnormals[f].z();
        file.write((char*)&x,4);
        file.write((char*)&y,4);
        file.write((char*)&z,4);
        BOOST_FOREACH(Mesh::Vertex_index vd,vertices_around_face(dataset.mesh.halfedge(f),dataset.mesh))
        {
           //cout <<dataset.mesh.point(vd)<<endl;
           x=dataset.mesh.point(vd).x();y=dataset.mesh.point(vd).y();z=dataset.mesh.point(vd).z();
           file.write((char*)&x,4);
           file.write((char*)&y,4);
           file.write((char*)&z,4);
         }
        file.write(attribute,2);
    }
    file.close();
    qDebug()<<"save file to"<<stlfileName;
}

void MainWindow::modelSegment()
{
    opengl->intrpoints.clear();
    HierarchicalClustering hierarchicalclustering;
    double esp=segSpinBox->value();
    if(!dataset.mesh.is_empty())
    {
        vector<vector<double>> charValue=shapediameterfunction->calculateSDF(dataset.mesh);
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
    opengl->clusterTable.clear();
    opengl->intrpoints.clear();
    layerSlider->setValue(1);
    slice.thick=float(sliceSpinBox->value());
    slice.isAdapt=isAdapt->isChecked();
    slice.isParaComp=isParaComp->isChecked();
    if(!dataset.halfedgeset.empty())
    {
        cout<<"start slice"<<endl;
        slice.startSlice(dataset.vertexset,dataset.halfedgeset,dataset.surroundBox,opengl->intrpoints);
        if(slice.isAdapt)
            cout<<"number of layers with adapt:"<<slice.layernumber<<endl;
        else
            cout<<"number of layers without adapt:"<<slice.layernumber<<endl;
        layerSlider->setRange(1,int(slice.layernumber));
        layerSpinBox->setRange(1,int(slice.layernumber));

    } else {
        QMessageBox::warning(this, tr("error"),
                             tr("You did not import any model."));
    }
}

void MainWindow::modelPlace()
{
    opengl->clusterTable.clear();
    opengl->intrpoints.clear();
    int x=placeSpinBoxx->value();
    int y=placeSpinBoxy->value();
    int z=placeSpinBoxz->value();
    if(!dataset.mesh.is_empty())
    {
        dataset.rotateModel(x,y,z);
        opengl->xtrans=-(dataset.surroundBox[1]+dataset.surroundBox[0])/2.0f;
        opengl->ytrans=(dataset.surroundBox[2]+dataset.surroundBox[3])/2.0f;
        opengl->ztrans=1.0f/(qMax(qAbs(dataset.surroundBox[4]),qAbs(dataset.surroundBox[5])));
        opengl->vertices=dataset.vertices;
        opengl->indices=dataset.indices;

    } else {
        QMessageBox::warning(this, tr("error"),
                             tr("You did not import any model."));
    }

}

void MainWindow::modelRepair()
{
    QTime time;
    time.start();
    MeshFix meshfix=MeshFix(&dataset.mesh);
    dataset.halfedgeOnGpu();
    dataset.getIndices();
    qDebug()<<"time of repairing the model:"<<time.elapsed()/1000.0<<"s";
    qDebug()<<"number of vertices after repairing:"<<dataset.mesh.number_of_vertices();
    qDebug()<<"number of edges after repairing:"<<dataset.mesh.number_of_edges();
    qDebug()<<"number of faces after repairing:"<<dataset.mesh.number_of_faces();
    float x=dataset.surroundBox[1]-dataset.surroundBox[0];
    float y=dataset.surroundBox[3]-dataset.surroundBox[2];
    float z=dataset.surroundBox[5]-dataset.surroundBox[4];
    qDebug()<<"surroundBox of the model:"<<x<<"*"<<y<<"*"<<z;
    opengl->vertices.clear();
    opengl->indices.clear();
    opengl->vertexnormals.clear();
    opengl->vertices=dataset.vertices;
    opengl->indices=dataset.indices;
    opengl->vertexnormals=dataset.vertexnormals;
}

