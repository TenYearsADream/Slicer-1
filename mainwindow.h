#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include "myglwidget.h"
#include "mytablewidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int nFaceCount=0;
    float Array[4][9];
private:
    void openFile();
    QAction *openAction;
    MyGLWidget *opengl;
    MyTableWidget *tableWidget;
};

#endif // MAINWINDOW_H
