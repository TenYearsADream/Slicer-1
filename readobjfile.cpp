#include "readobjfile.h"
#include <dataset.h>
#include "loadprogressbar.h"
#include <QDebug>
#include <QTextStream>
#include <QApplication>
ReadOBJFile::ReadOBJFile(dataSet &_dataset)
{
    modelsize=0;
    dataset=&_dataset;
    isstop=false;
}
bool ReadOBJFile::ReadObjFile(const QString filename)
{
    loadProgressBar progressbar("read obj...");
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

    QTextStream in1(&file);
    int total=0,cur=0;
    float fraction=0.0f;
    QString line = in1.readLine();
    while (!line.isNull()) {
        line = in1.readLine();
        list=line.split(" ");
        if(list[0]=="v")
        {
            total++;
        }
        else if (list[0]=="f") {
            total++;
        }
    }
    file.close();
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);
    line = in.readLine();
    while (!line.isNull()) {
        if(isstop)
        {
            return false;
        }
        list=line.split(" ");
        if(list[0]=="v")
        {
            cur++;
            addPoint(list);
            fraction=float(cur)/total;
            emit progressReport(100*fraction,100.0f);
            QApplication::processEvents();
        }
        else if (list[0]=="f") {
            cur++;
            addFace(list);
            fraction=float(cur)/total;
            emit progressReport(100*fraction,100.0f);
            QApplication::processEvents();
        }
        line = in.readLine();
    }
    file.close();
    return true;
}

void ReadOBJFile::addPoint(QStringList list)
{
    float x,y,z;
    x=list[1].toFloat();
    y=list[2].toFloat();
    z=list[3].toFloat();
    dataset->mesh.add_vertex(Point(x,y,z));
}
void ReadOBJFile::addFace(QStringList list)
{
    QStringList sublist;
    uint point[3];
    for(int i=1;i<list.size();i++)
    {
        sublist=list[i].split("/");
        point[i-1]=sublist[0].toUInt();
    }
    Mesh::Vertex_index v0(point[0]-1);
    Mesh::Vertex_index v1(point[1]-1);
    Mesh::Vertex_index v2(point[2]-1);
    dataset->mesh.add_face(v0,v1,v2);
}
void ReadOBJFile::ExitRead(){
    modelsize=0;
    dataset->mesh.clear();
    isstop=true;
}
