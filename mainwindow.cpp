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

}

MainWindow::~MainWindow()
{

}

void MainWindow::openFile()
{
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Open File"),
                                                ".",
                                               tr("Text Files(*.stl)"));
    const char* filepath=path.toStdString().c_str();//QString转化为string类型，然后由string转化char*
    if(!path.isEmpty()) {
        readstl.ReadStlFile(filepath);
        qDebug()<<"number of faces:"<<readstl.NumTri()<<endl;
        qDebug()<<"nuber of vertexs:"<<readstl.PointList().size()<<endl;
        //qDebug()<<readstl.pointList.at(0).x<<endl;
        //qDebug()<<readstl.surroundBox[1]<<endl;
        QTime time;
        time.start();
        tableWidget->setRowCount(readstl.NumTri());
        tableWidget->setData(readstl.pointList,readstl.NumTri());
        tableWidget->show();
        qDebug()<<"time of table:"<<time.elapsed()/1000.0<<"s";
        time.start();
        opengl->m_xMove=-(readstl.surroundBox[1]+readstl.surroundBox[0])/2.0;
        opengl->m_yMove=(readstl.surroundBox[2]+readstl.surroundBox[3])/2.0;
        opengl->zoom=-3.0*qMax(qAbs(readstl.surroundBox[4]),qAbs(readstl.surroundBox[5]))-50.0;
        opengl->nFaceCount=readstl.NumTri();
        opengl->pointList=readstl.pointList;
        qDebug()<<"time of OpenGl:"<<time.elapsed()/1000.0/1000.0<<"s";

    } else {
        QMessageBox::warning(this, tr("Path"),
                             tr("You did not select any file."));
    }
}
