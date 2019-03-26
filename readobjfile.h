#ifndef READOBJFILE_H
#define READOBJFILE_H
#include <QString>
#include <QFile>
class dataSet;
class ReadOBJFile
{
public:
    int modelsize;
public:
    ReadOBJFile(dataSet &_dataset);
    bool ReadObjFile(const QString filename);
private:
    void addPoint(QStringList list);
    void addFace(QStringList list);
private:
    dataSet *dataset;
};

#endif // READOBJFILE_H
