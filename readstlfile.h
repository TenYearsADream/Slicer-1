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
    uint numberTriangles;
    uint numberVertices;
    vector<Point> normalList;
public:
    bool ReadStlFile(const QString filename,dataSet &dataset);
private:
    QHash<QString,uint> verticesmap;
    QFile file;

private:
    void ReadASCII(const char *buf,dataSet &dataset);
    void ReadBinary(char *buf,dataSet &dataset);
    uint addPoint(QString key,Point point,dataSet &dataset);
};
