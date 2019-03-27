#include "readofffile.h"
#include <dataset.h>
#include "loadprogressbar.h"
#include <QDebug>
#include <QTextStream>
#include <QApplication>
ReadOFFFile::ReadOFFFile(dataSet &_dataset)
{
    modelsize=0;
    numberVertices=0;
    numberTriangles=0;
    isstop=false;
    dataset=&_dataset;
}
bool ReadOFFFile::ReadOffFile(const QString filename)
{
    loadProgressBar progressbar("read off...");
    connect(this,SIGNAL(progressReport(float,float)),&progressbar,SLOT(setProgressBar(float,float)));
    connect(&progressbar,SIGNAL(signalExit()),this,SLOT(ExitRead()));
    dataset->mesh.clear();
    QFile file(filename);
    QStringList list;
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"cann't open the file:"<<filename<<endl;
        return false;
    }
    modelsize=int(file.size()/1048576);
    qDebug() <<"内存大小："<<modelsize<<"M";
    QTextStream in(&file);
    QString line = in.readLine();
    line=in.readLine();
    list=line.split(" ");
    numberVertices=list[0].toUInt();
    numberTriangles=list[1].toUInt();
    uint total=numberVertices+numberTriangles;
    float fraction=0.0f;
    for(uint i=0;i<numberVertices;i++)
    {
        line = in.readLine();
        list=line.split(" ");
        addPoint(list);
        fraction=float(i+1)/total;
        emit progressReport(100*fraction,100.0f);
        QApplication::processEvents();
    }
    for(uint i=0;i<numberTriangles;i++)
    {
        line = in.readLine();
        list=line.split(" ");
        addFace(list);
        fraction=float(numberVertices+i+1)/total;
        emit progressReport(100*fraction,100.0f);
        QApplication::processEvents();
    }
    file.close();
    if(isstop)
        return false;
    else
        return true;
}

void ReadOFFFile::addPoint(QStringList list)
{
    float x,y,z;
    x=list[0].toFloat();
    y=list[1].toFloat();
    z=list[2].toFloat();
    dataset->mesh.add_vertex(Point(x,y,z));
}

void ReadOFFFile::addFace(QStringList list)
{
    QStringList sublist;
    uint point[3];
    int n=list[0].toInt();
    for(int i=0;i<3;i++)
    {
        point[i]=list[i+1].toUInt();
    }
    Mesh::Vertex_index v0(point[0]);
    Mesh::Vertex_index v1(point[1]);
    Mesh::Vertex_index v2(point[2]);
    dataset->mesh.add_face(v0,v1,v2);
}
void ReadOFFFile::ExitRead(){
    modelsize=0;
    numberVertices=0;
    numberTriangles=0;
    dataset->mesh.clear();
    isstop=true;
}
