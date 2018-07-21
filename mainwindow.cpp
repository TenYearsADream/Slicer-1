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
#include "windows.h"
#include"WinBase.h"
#include "Psapi.h"
#include "mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Slicer"));
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
    tableWidget =new MyTableWidget((cenWidget));
    connect(tableWidget,SIGNAL(rowClicked(int)),opengl,SLOT(changeColorFlag(int)));
    QPushButton *button = new QPushButton(tr("useless"),(cenWidget));

    QHBoxLayout *mainlayout = new QHBoxLayout(cenWidget);

    QVBoxLayout *rightlayout = new QVBoxLayout(cenWidget);
    QSplitter *splitter=new QSplitter(cenWidget);

    rightlayout->addWidget(tableWidget);
    rightlayout->addWidget(button);
    splitter->addWidget(opengl);
    splitter->setMinimumWidth(400);
    splitter->setMinimumHeight(400);
    cenWidget->setLayout(mainlayout);

    mainlayout->addWidget(splitter);
    mainlayout->addLayout(rightlayout);
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
        //qDebug()<<"number of faces:"<<readstl.NumTri()<<endl;
        //qDebug()<<readstl.surroundBox[1]<<endl;
        time.start();
        tableWidget->setRowCount(readstl.NumTri());
        tableWidget->setData(readstl.hashtable->vertices,readstl.faceList);
        tableWidget->show();
        qDebug()<<"time of table:"<<time.elapsed()/1000.0<<"s";
        time.start();
        opengl->m_xMove=-(readstl.surroundBox[1]+readstl.surroundBox[0])/2.0;
        opengl->m_yMove=(readstl.surroundBox[2]+readstl.surroundBox[3])/2.0;
        opengl->zoom=-3.0*qMax(qAbs(readstl.surroundBox[4]),qAbs(readstl.surroundBox[5]))-50.0;
        opengl->vertices=readstl.hashtable->vertices;
        opengl->faceList=readstl.faceList;
        qDebug()<<"time of OpenGl:"<<time.elapsed()/1000.0/1000.0<<"s";
        showMemoryInfo();
        vector<vector<double>> charValue=shapediameterfunction->calculateSDF(readstl.hashtable->vertices,readstl.faceList);
        for(int i=0;i<charValue.size();i++)
        {
            qDebug()<<charValue[i][0]<<" "<<charValue[i][1]<<endl;
        }
        //readstl.hashtable->show();
        qDebug()<<"number of vertices:"<<readstl.hashtable->size;
        qDebug()<<"number of faces:"<<readstl.faceList.size()<<endl;

    } else {
        QMessageBox::warning(this, tr("Path"),
                             tr("You did not select any file."));
    }
}
