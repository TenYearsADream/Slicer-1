#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QSurfaceFormat format;
    format.setVersion(3, 2);
    MainWindow w;
    w.show();
    return app.exec();
}
