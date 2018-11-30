#include<vector>
#include <QString>
#include <QHash>
#include <QFile>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <dataset.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;
using namespace std;
class ReadSTLFile
{
public:
    unsigned int numberTriangles;
    uint numberVertices;
    vector<Point> normalList;
    //dataSet dataset;
public:
    bool ReadStlFile(const QString filename,dataSet &dataset);
private:
    QHash<QString,uint> verticesmap;
    char* memwriter;
    QFile file;

private:
    bool ReadASCII(const char *buf,dataSet &dataset);
    bool ReadBinary(const char *cfilename,dataSet &dataset);
    uint addPoint(QString key,Point point,dataSet &dataset);

    int cpyint(const char*& p);
    float cpyfloat(const char*& p);
};
