#include<vector>
#include <QString>
#include <QHash>
#include <QFile>
#include <dataset.h>
using namespace std;
class ReadSTLFile:public QObject
{
    Q_OBJECT
public:
    uint numberTriangles;
    uint numberVertices;
    vector<Point> normalList;
    vector<uint> indices;
    vector<float> vertices;
    int modelsize;
    QString filetype;
public:
    explicit ReadSTLFile(dataSet &_dataset);
    ~ReadSTLFile();
    bool ReadStlFile(const QString filename);
private:
    QHash<QString,uint> verticesmap;
    QFile file;
    dataSet *dataset;
    bool isstop;
private:
    bool ReadASCII(const char *buf);
    bool ReadBinary(char *buf);
    uint addPoint(QString key,Point point);
signals:
    void progressReport(float fraction,float total);
public slots:
    void ExitRead();

};
