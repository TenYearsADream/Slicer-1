#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QStatusBar>
#include <QTextEdit>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QProgressDialog>
#include <iostream>
#include "windows.h"
#include "mainwindow.h"
#include "Slice.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle(tr("Slicer"));
    setMinimumSize(1000,640);
    initStatusBar();

    initMenuWidget();
    QWidget *cenWidget = new QWidget(this); //this is point to QMainWindow
    this->setCentralWidget(cenWidget);

    opengl= new MyGLWidget(cenWidget);
    toolWidget=new QWidget(cenWidget);
    QHBoxLayout *mainlayout = new QHBoxLayout(cenWidget);
    toollayout = new QVBoxLayout();
    cenWidget->setLayout(mainlayout);
    mainlayout->addWidget(opengl);
    mainlayout->addWidget(toolWidget);
    toolWidget->setLayout(toollayout);
    mainlayout->setStretchFactor(opengl, 4);
    mainlayout->setStretchFactor(toolWidget,2);

    //模型摆放布局
    initPlaceWidget();
    //分层显示布局
    initSliceWidget();
    //模型分割布局
    initSegmentWidget();

    //日志打印窗口
    logEdit = new QTextEdit("slicer");
    toollayout->addWidget(logEdit);
    connect(this,SIGNAL(outputMsg(QString)),logEdit,SLOT(append(QString)));
    connect(&slice,SIGNAL(outputMsg(QString)),logEdit,SLOT(append(QString)),Qt::DirectConnection);
    connect(&meshfix,SIGNAL(outputMsg(QString)),logEdit,SLOT(append(QString)));

    emit outputMsg("Device: "+QString::fromStdString(slice.opencl.deviceinfo.deviceName));
    emit outputMsg("Parallel compute units: "+QString::number(slice.opencl.deviceinfo.maxComputeUnits));
    emit outputMsg("maxLocalMemSize: "+QString::number(slice.opencl.deviceinfo.maxLocalMemSize)+"KB");
    emit outputMsg("maxMemAllocSize: "+QString::number(slice.opencl.deviceinfo.maxMemAllocSize)+"MB");
    emit outputMsg("maxGlobalMemSize: "+QString::number(slice.opencl.deviceinfo.maxGlobalMemSize)+"MB");
    emit outputMsg("maxWorkItemPerGroup: "+QString::number(slice.opencl.deviceinfo.maxWorkItemPerGroup));
    emit outputMsg("maxConstantBufferSize: "+QString::number(slice.opencl.deviceinfo.maxConstantBufferSize)+"KB");
}

MainWindow::~MainWindow()
{
    delete(openAction);
    delete(opengl);
}

void MainWindow::initStatusBar()
{
    statusLabel = new QLabel; //新建标签
    currentTimeLabel = new QLabel;

    statusLabel->setMinimumSize(statusLabel->sizeHint()); //设置标签最小尺寸
    statusLabel->setAlignment(Qt::AlignCenter);
//    statusLabel->setFrameShape(QFrame::WinPanel); //设置标签形状
//    statusLabel->setFrameShadow(QFrame::Sunken); //设置标签阴影

    statusBar()->addWidget(statusLabel);
    statusLabel->setText(tr("ready")); //初始化内容
    QTimer *timer = new QTimer(this);
    timer->start(1000); //每隔1000ms发送timeout的信号
    connect(timer, SIGNAL(timeout()),this,SLOT(timeUpdate()));
    statusBar()->addPermanentWidget(currentTimeLabel); //现实永久信息

}

void MainWindow::initPlaceWidget()
{
    QLabel *placeLablex = new QLabel();
    placeLablex->setText("Degree of rotation around X axis:");
    QSpinBox *placeSpinBoxx = new QSpinBox();
    placeSpinBoxx->setRange(-360, 360);  // 范围
    QLabel *placeLabley = new QLabel();
    placeLabley->setText("Degree of rotation around Y axis:");
    QSpinBox *placeSpinBoxy = new QSpinBox();
    placeSpinBoxy->setRange(-360, 360);  // 范围
    QLabel *placeLablez = new QLabel();
    placeLablez->setText("Degree of rotation around Z axis:");
    QSpinBox *placeSpinBoxz = new QSpinBox();
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
}

void MainWindow::initSegmentWidget()
{
    segSpinBox = new QDoubleSpinBox(this);
    segSpinBox->setRange(0.1, 20);  // 范围
    segSpinBox->setDecimals(2);  // 精度
    segSpinBox->setSingleStep(0.01); // 步长
    QPushButton *segmentButton = new QPushButton(tr("segment"),(this));
//    connect(segmentButton,SIGNAL(clicked()),this,SLOT(modelSegment()));
    QWidget *segmentQWidget=new QWidget(toolWidget);
    QHBoxLayout *segmentlayout = new QHBoxLayout();
    segmentlayout->addWidget(segmentButton,0,Qt::AlignCenter);
    segmentlayout->addWidget(segSpinBox,0,Qt::AlignCenter);
    segmentQWidget->setLayout(segmentlayout);
    toollayout->addWidget(segmentQWidget);
    toollayout->setStretchFactor(segmentQWidget,2);
}

void MainWindow::initSliceWidget()
{
    sliceSpinBox = new QDoubleSpinBox(this);
    sliceSpinBox->setRange(0.01, 20);  // 范围
    sliceSpinBox->setDecimals(2);  // 精度
    sliceSpinBox->setSingleStep(0.05); // 步长
    QPushButton *sliceButton = new QPushButton(tr("slice"));
    connect(sliceButton,SIGNAL(clicked()),this,SLOT(modelSlice()));
    layerSlider = new QSlider(Qt::Horizontal);
    QLabel *layerLable = new QLabel();
    layerLable->setText("layer:");
    layerSpinBox=new QSpinBox(this);
    layerSpinBox->setValue(0);
    isAdapt = new QCheckBox();
    isAdapt->setText("adapt slice");
    comboBox = new QComboBox(this);
    comboBox->addItem(tr("CPU"));
    comboBox->addItem(tr("GPU"));
    comboBox->addItem(tr("CPU2"));
    QWidget *layerQWidget=new QWidget(toolWidget);
    //layerQWidget->setStyleSheet("background-color:green;");
    QVBoxLayout *layerlayout = new QVBoxLayout();
    QHBoxLayout *layerlayout1 = new QHBoxLayout();
    layerlayout1->addWidget(sliceButton,0,Qt::AlignCenter);
    layerlayout1->addWidget(sliceSpinBox,0,Qt::AlignCenter);
    QHBoxLayout *layerlayout2 = new QHBoxLayout();
    layerlayout2->addWidget(isAdapt,0,Qt::AlignCenter);
    layerlayout2->addWidget(comboBox,0,Qt::AlignCenter);
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
}

void MainWindow::initMenuWidget()
{
    QMenu *file = menuBar()->addMenu(tr("&File"));
    openAction = new QAction(QIcon(":/images/resource/open-file.png"), tr("&Open..."), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction,&QAction::triggered,this,&MainWindow::openFile);
    file->addAction(openAction);
    saveStl = new QAction(QIcon(":/images/resource/save-file.png"), tr("&Save..."), this);
    saveStl->setShortcuts(QKeySequence::Save);
    saveStl->setStatusTip(tr("Save the model"));
    connect(saveStl,&QAction::triggered,this,&MainWindow::saveFile);
    file->addAction(saveStl);
}

void MainWindow::closeEvent(QCloseEvent *event) //系统自带退出确定程序
{
    int choose;
    choose= QMessageBox::question(this, tr("EXIT"),
                                   QString(tr("YES?")),
                                   QMessageBox::Yes | QMessageBox::No);

    if (choose== QMessageBox::No)
     {
          event->ignore();  //忽略//程序继续运行
    }
    else if (choose== QMessageBox::Yes)
    {
          event->accept();  //接受//程序退出
    }
}


void MainWindow::openFile()
{
    statusLabel->setText(tr("opening..."));
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
            statusLabel->setText(tr("Failed to read STL file!"));
            return;
        }
        emit outputMsg(path);
        emit outputMsg("Model Size: "+QString::number(readstl.modelsize)+"M");
        emit outputMsg("File Type: "+readstl.filetype);
        int readtime=time.elapsed()/1000;
        emit outputMsg("time of readstl: "+QString::number(readtime)+"s");
        emit outputMsg("number of vertices: "+QString::number(dataset.mesh.number_of_vertices()));
        emit outputMsg("number of edges: "+QString::number(dataset.mesh.number_of_edges()));
        emit outputMsg("number of faces: "+QString::number(dataset.mesh.number_of_faces()));
        emit outputMsg("number of normals: "+QString::number(readstl.normalList.size()));
        qDebug()<<"time of readstl:"<<readtime<<"s";
        qDebug()<<"number of vertices:"<<dataset.mesh.number_of_vertices();
        qDebug()<<"number of edges:"<<dataset.mesh.number_of_edges();
        qDebug()<<"number of faces:"<<dataset.mesh.number_of_faces();
        qDebug()<<"number of normals:"<<readstl.normalList.size();
        vector<Point>().swap(readstl.normalList);
        dataset.halfedgeOnGpu();
        dataset.getIndices();
        float x=dataset.surroundBox[1]-dataset.surroundBox[0];
        float y=dataset.surroundBox[3]-dataset.surroundBox[2];
        float z=dataset.surroundBox[5]-dataset.surroundBox[4];
        emit outputMsg("surroundBox of the model: "+QString("%1").arg(x)+"*"+QString("%1").arg(y)+"*"+QString("%1").arg(z)+"mm");
        qDebug()<<"surroundBox of the model:"<<x<<"*"<<y<<"*"<<z;
        statusLabel->setText(tr("open file success"));
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
        if(dataset.vertices.empty())
        {
            dataset.mesh.clear();
            cout<<"can't simplify mesh."<<endl;
            emit outputMsg("模型不能简化，不显示模型");
        }
        else
        {
            opengl->vertices=dataset.vertices;
            opengl->indices=dataset.indices;
            opengl->vertexnormals=dataset.vertexnormals;
            vector<uint>().swap(dataset.indices);
            vector<float>().swap(dataset.vertices);
            vector<float>().swap(dataset.vertexnormals);
        }


    } else {
        QMessageBox::warning(this, tr("Path"),
                             tr("You did not select any file."));
    }
}

void MainWindow::saveFile()
{
    qDebug()<<"start save file...";
    statusLabel->setText(tr("saveing..."));
	emit outputMsg("开始保存模型...");
    QString stlfileName=slicepath+"_repaired.stl";
    dataset.exportSTL(stlfileName);
    qDebug()<<"save file to"<<stlfileName;
	emit outputMsg("模型保存到"+stlfileName+".");
    statusLabel->setText(tr("save file success"));
}

void MainWindow::modelSlice()
{
    statusLabel->setText(tr("slicing..."));
    opengl->clusterTable.clear();
    opengl->intrpoints.clear();
    layerSlider->setValue(1);
    slice.thick=float(sliceSpinBox->value());
    slice.isAdapt=isAdapt->isChecked();
    slice.sliceType=comboBox->currentText();
    if(!dataset.halfedgeset.empty())
    {
        cout<<"start slice"<<endl;        
        emit outputMsg("start slice......");
        slice.startSlice(dataset.vertexset,dataset.halfedgeset,dataset.surroundBox,opengl->intrpoints);
        emit outputMsg("number of layers: "+QString::number(slice.layernumber));
        if(slice.isAdapt)
            cout<<"number of layers with adapt:"<<slice.layernumber<<endl;
        else
            cout<<"number of layers without adapt:"<<slice.layernumber<<endl;
        layerSlider->setRange(1,int(slice.layernumber));
        layerSpinBox->setRange(1,int(slice.layernumber));
        opengl->update();
        statusLabel->setText(tr("slice finished."));
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
        vector<uint>().swap(dataset.indices);
        vector<float>().swap(dataset.vertices);

    } else {
        QMessageBox::warning(this, tr("error"),
                             tr("You did not import any model."));
    }

}

void MainWindow::modelRepair()
{
    statusLabel->setText(tr("repairing..."));
    QTime time;
    time.start();
    meshfix.repair(dataset.mesh);
    dataset.halfedgeOnGpu();
    dataset.getIndices();
    emit outputMsg("time of repairing the model: "+QString::number(time.elapsed()/1000.0)+"s");
    qDebug()<<"time of repairing the model:"<<time.elapsed()/1000.0<<"s";
    qDebug()<<"number of vertices after repairing:"<<dataset.mesh.number_of_vertices();
    qDebug()<<"number of edges after repairing:"<<dataset.mesh.number_of_edges();
    qDebug()<<"number of faces after repairing:"<<dataset.mesh.number_of_faces();
    emit outputMsg("number of vertices after repairing:"+QString::number(dataset.mesh.number_of_vertices()));
    emit outputMsg("number of edges after repairing:"+QString::number(dataset.mesh.number_of_edges()));
    emit outputMsg("number of faces after repairing:"+QString::number(dataset.mesh.number_of_faces()));
    float x=dataset.surroundBox[1]-dataset.surroundBox[0];
    float y=dataset.surroundBox[3]-dataset.surroundBox[2];
    float z=dataset.surroundBox[5]-dataset.surroundBox[4];
    emit outputMsg("surroundBox of the model:"+QString("%1").arg(x)+"*"+QString("%1").arg(y)+"*"+QString("%1").arg(z)+"mm");
    qDebug()<<"surroundBox of the model:"<<x<<"*"<<y<<"*"<<z;
    statusLabel->setText(tr("repair finished."));
    opengl->vertices.clear();
    opengl->indices.clear();
    opengl->vertexnormals.clear();
    opengl->vertices=dataset.vertices;
    opengl->indices=dataset.indices;
    opengl->vertexnormals=dataset.vertexnormals;
    vector<uint>().swap(dataset.indices);
    vector<float>().swap(dataset.vertices);
    vector<float>().swap(dataset.vertexnormals);
}

void MainWindow::timeUpdate()
{
    QDateTime current_time = QDateTime::currentDateTime();
    QString timestr = current_time.toString( "yyyy/MM/dd hh:mm:ss"); //设置显示的格式
    currentTimeLabel->setText(timestr); //设置label的文本内容为时间
}
