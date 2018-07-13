﻿
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

#include "mainwindow.h"

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
    opengl->nFaceCount = nFaceCount;
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
        qDebug()<<readstl.PointList().size()<<endl;
        qDebug()<<readstl.pointList.at(0).x<<endl;
        tableWidget->setRowCount(readstl.NumTri());
        tableWidget->setData(readstl.pointList,readstl.NumTri());
        tableWidget->show();

    } else {
        QMessageBox::warning(this, tr("Path"),
                             tr("You did not select any file."));
    }
}
