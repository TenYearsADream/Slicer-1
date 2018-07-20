#include "mainwindow.h"
#include <QApplication>
#include "hashtable.h"
#include "point3f.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>
using namespace std;
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
