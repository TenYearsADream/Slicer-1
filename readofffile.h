#ifndef READOFFFILE_H
#define READOFFFILE_H
#include <QString>
#include <QFile>
class dataSet;
class ReadOFFFile:public QObject
{
    Q_OBJECT
public:
    int modelsize;
public:
    explicit ReadOFFFile(dataSet &_dataset);
    bool ReadOffFile(const QString filename);
private:
    void addPoint(QStringList list);
    void addFace(QStringList list);
private:
    dataSet *dataset;
    uint numberTriangles;
    uint numberVertices;
    bool isstop;
signals:
    void progressReport(float fraction,float total);
public slots:
    void ExitRead();
};

#endif // READOFFFILE_H
