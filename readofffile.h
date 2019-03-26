#ifndef READOFFFILE_H
#define READOFFFILE_H
#include <QString>
#include <QFile>
class dataSet;
class ReadOFFFile
{
public:
    int modelsize;
public:
    ReadOFFFile(dataSet &_dataset);
    bool ReadOffFile(const QString filename);
private:
    void addPoint(QStringList list);
    void addFace(QStringList list);
private:
    dataSet *dataset;
    uint numberTriangles;
    uint numberVertices;
};

#endif // READOFFFILE_H
