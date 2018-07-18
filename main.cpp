#include "mainwindow.h"
#include <QApplication>

#include <CGAL/Simple_cartesian.h>

using namespace std;

typedef CGAL::Simple_cartesian <double> Kernel;
typedef Kernel::Point_2                 Point_2;
typedef Kernel::Segment_2               Segment_2;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Point_2 p(1,1);
    Point_2 q(10,10);
    cout<<"p="<<1<<endl;
    cout<<"q="<<2<<endl;
    cout << "sqdist(p,q)=" <<CGAL::squared_distance(p,q) <<endl;

    MainWindow w;
    w.show();
    return a.exec();
}
