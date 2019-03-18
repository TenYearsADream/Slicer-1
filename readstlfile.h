#include<vector>
#include <QString>
#include <QHash>
#include <QFile>
#include <dataset.h>
using namespace std;
class ReadSTLFile
{
public:
    uint numberTriangles;
    uint numberVertices;
    vector<Point> normalList;
    vector<uint> indices;
    vector<float> vertices;
    int modelsize;
    QString filetype;
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
