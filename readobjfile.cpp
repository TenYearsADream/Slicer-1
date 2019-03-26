#include "readobjfile.h"
#include <dataset.h>
#include <QDebug>
#include <QTextStream>
ReadOBJFile::ReadOBJFile(dataSet &_dataset)
{
    modelsize=0;
    dataset=&_dataset;
}
bool ReadOBJFile::ReadObjFile(const QString filename)
{
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
    while (!line.isNull()) {
        list=line.split(" ");
        if(list[0]=="v")
        {
            addPoint(list);
        }
        else if (list[0]=="f") {
            addFace(list);
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
