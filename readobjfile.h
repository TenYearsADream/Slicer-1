#ifndef READOBJFILE_H
#define READOBJFILE_H
#include <QString>
#include <QFile>
class dataSet;
class ReadOBJFile:public QObject
{
    Q_OBJECT
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
    bool isstop;
signals:
    void progressReport(float fraction,float total);
public slots:
    void ExitRead();
};

#endif // READOBJFILE_H
