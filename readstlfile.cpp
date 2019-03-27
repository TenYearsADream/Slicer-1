#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <QDebug>
#include <QtCore/qmath.h>
#include <QProgressDialog>
#include <QMessageBox>
#include"readstlfile.h"
#include "loadprogressbar.h"
#include <QApplication>
using namespace std;
ReadSTLFile::ReadSTLFile(dataSet &_dataset)
{
    dataset=&_dataset;
    isstop=false;
    numberTriangles=0;
    numberVertices=0;
    verticesmap.clear();
}
bool ReadSTLFile::ReadStlFile(const QString filename)
{
    loadProgressBar progressbar("read stl...");
    connect(this,SIGNAL(progressReport(float,float)),&progressbar,SLOT(setProgressBar(float,float)));
    connect(&progressbar,SIGNAL(signalExit()),this,SLOT(ExitRead()));
    normalList.clear();//清空vector
    vertices.clear();
    indices.clear();
    dataset->mesh.clear();
    file.setFileName(filename);
    uchar* buffer;
    int headoffset;
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray line = file.readLine();
        string header(line);
        headoffset=line.size();
        if(header[0]=='s')
        {
            cout<<"File is ASCII"<<endl;
            filetype="ASCII";
            modelsize=int(file.size()/1048576);
            qDebug() <<"内存大小："<< file.size()/1048576<<"M";
            buffer=file.map(0,file.size());
            QTextStream in(&file);
            uint total=1;
            QString line = in.readLine();
            while (!line.isNull()) {
                line = in.readLine();
                total++;
            }
            numberTriangles=(total-2)/7;
            if(buffer)
            {
                if(!ReadASCII((char*)buffer))
                {
                    return false;
                }
            }
            else
            {
                cout<<"out of memory error"<<endl;
                return false;
            }
        }
        else
        {
            cout<<"File is Binary"<<endl;
            filetype="Binary";
            modelsize=int(file.size()/1048576);
            qDebug() <<"内存大小："<< file.size()/1048576<<"M";
            buffer=file.map(0,file.size());
            if(buffer)
            {
                if(!ReadBinary((char*)buffer))
                {
                    return false;
                }
            }
            else
            {
                cout<<"out of memory error"<<endl;
                return false;
            }
        }
        file.unmap(buffer);
        file.close();
        verticesmap.clear();
        return true;
    }
    else
    {
        qDebug()<<"cann't open the file:"<<filename<<endl;
        return false;
    }
}

bool ReadSTLFile::ReadBinary(char *buffer)
{
    float x=0,y=0,z=0;
    uint index=0;
    Point normal,p0,p1,p2,ab,bc,nor;
    QString strx(" "),stry(" "),strz(" "),key(" ");
    char name[80];
    uint *point=new uint[3]();
    float fraction=0.0f;
    memcpy(name, buffer, 80);//80字节文件头
    //cout<<name<<endl;
    buffer += 80;
    numberTriangles=0;
    numberVertices=0;
    memcpy(&numberTriangles,buffer,4);//4字节三角面片个数
    normalList.reserve(numberTriangles);
    verticesmap.reserve(numberTriangles/2);
    //cout<<numberTriangles<<endl;
    buffer +=4;
    //读取三角形面片
    for (uint i = 0; i <numberTriangles; i++)
    {
//        cout<<i<<endl;
        memcpy(&x,buffer, 4);buffer +=4;
        memcpy(&y,buffer, 4);buffer +=4;
        memcpy(&z,buffer, 4);buffer +=4;
        //cout<<x<<" "<<y<<" "<<z<<endl;
        normalList.push_back(Point(x, y, z));//法向量
        for (int j = 0; j < 3; j++)//读取三顶点
        {
            memcpy(&x,buffer, 4);buffer +=4;
            memcpy(&y,buffer, 4);buffer +=4;
            memcpy(&z,buffer, 4);buffer +=4;
            if(qAbs(x)<1e-12f)x=0;
            if(qAbs(y)<1e-12f)y=0;
            if(qAbs(z)<1e-12f)z=0;
            //cout<<x<<" "<<y<<" "<<z<<endl;
            strx = QString::number(double(x), 10, 15);
            stry = QString::number(double(y), 10, 15);
            strz = QString::number(double(z), 10, 15);
            //qDebug()<<strx<<" "<<stry<<" "<<strz;
            key="1"+strx+stry+strz;
            index=addPoint(key,Point(x,y,z));
            point[j]=index;
        }
        Mesh::Vertex_index v0(point[0]);
        Mesh::Vertex_index v1(point[1]);
        Mesh::Vertex_index v2(point[2]);
        dataset->mesh.add_face(v0,v1,v2);
//        p0=dataset.mesh.point(v0);
//        p1=dataset.mesh.point(v1);
//        p2=dataset.mesh.point(v2);
//        ab=Point(p0.x()-p1.x(),p0.y()-p1.y(),p0.z()-p1.z());
//        bc=Point(p1.x()-p2.x(),p1.y()-p2.y(),p1.z()-p2.z());
//        nor=Point(ab.y()*bc.z()-bc.y()*ab.z(),bc.x()*ab.z()-ab.x()*bc.z(),ab.x()*bc.y()-bc.x()*ab.y());
//        if((normal.x()*nor.x()+normal.y()*nor.y()+normal.z()*nor.z())<0)
//            dataset.mesh.add_face(v0,v1,v2);
//        else
//            dataset.mesh.add_face(v0,v2,v1);
//        indices.push_back(ushort(point[0]));
//        indices.push_back(ushort(point[1]));
//        indices.push_back(ushort(point[2]));
        buffer += 2;//跳过尾部标志
        fraction=float(i+1)/numberTriangles;
        emit progressReport(100*fraction,100.0f);
        QApplication::processEvents();
    }
    delete[] point;
    if(isstop)
        return false;
    else
        return true;
}

uint ReadSTLFile::addPoint(QString key,Point point){
    uint index;
    auto it = verticesmap.find(key);
    if(it != verticesmap.end())
    {
        index=it.value();
        //cout<<"索引："<<it.value()<<endl;
    }
    else
    {
        dataset->mesh.add_vertex(point);
//        vertices.push_back(float(point.x()));
//        vertices.push_back(float(point.y()));
//        vertices.push_back(float(point.z()));
        verticesmap.insert(key,numberVertices);
        index=numberVertices;
        numberVertices++;
    }
    return index;
}

bool ReadSTLFile::ReadASCII(const char *buf)
{
    const int offset=280;
    numberVertices=0;
    int cur=0;
    double x=0, y=0, z=0;
    Point normal,p0,p1,p2,ab,bc,nor;
    QString strx(" "),stry(" "),strz(" "),key(" ");
    uint index=0;
    float fraction=0.0f;
    uint *point=new uint[3]();
    const char *buffer=strstr(buf,"facet normal");
    int namelength=int(buffer-buf);
    char *name=new char[namelength];
    strncpy(name,buf,sizeof(name));
    name[namelength]='\0';
    char facet[offset];
    string useless;
    do
    {
        if(isstop)
        {
            return false;
        }
        strncpy(facet,buffer,sizeof(facet));
        facet[offset-1]='\0';
        //cout<<facet<<endl;
        stringstream ss(facet);
        ss >> useless;//facet
        ss >> useless >> x >> y >>z;//法向量
        normal=Point(x,y,z);
        normalList.push_back(Point(x, y, z));
        getline(ss, useless);
        getline(ss, useless);//outer loop
        for (int i = 0; i < 3; i++)
        {
            ss >> useless >> x >> y >> z;
            if(qAbs(x)<1e-12)x=0;
            if(qAbs(y)<1e-12)y=0;
            if(qAbs(z)<1e-12)z=0;
            //cout<<x<<" "<<y<<" "<<z<<endl;
            strx = QString::number(x, 10, 15);
            stry = QString::number(y, 10, 15);
            strz = QString::number(z, 10, 15);
            //qDebug()<<strx<<" "<<stry<<" "<<strz;
            key="1"+strx+stry+strz;
            index=addPoint(key,Point(x,y,z));
            point[i]=index;
        }       
        Mesh::Vertex_index v0(point[0]);
        Mesh::Vertex_index v1(point[1]);
        Mesh::Vertex_index v2(point[2]);
        //cout<<v0<<" "<<v1<<" "<<v2<<endl;
        dataset->mesh.add_face(v0,v1,v2);
//        p0=dataset.mesh.point(v0);
//        p1=dataset.mesh.point(v1);
//        p2=dataset.mesh.point(v2);
//        ab=Point(p0.x()-p1.x(),p0.y()-p1.y(),p0.z()-p1.z());
//        bc=Point(p1.x()-p2.x(),p1.y()-p2.y(),p1.z()-p2.z());
//        nor=Point(ab.y()*bc.z()-bc.y()*ab.z(),bc.x()*ab.z()-ab.x()*bc.z(),ab.x()*bc.y()-bc.x()*ab.y());
//        if((normal.x()*nor.x()+normal.y()*nor.y()+normal.z()*nor.z())<0)
//            dataset.mesh.add_face(v0,v1,v2);
//        else
//            dataset.mesh.add_face(v0,v2,v1);
//        indices.push_back(point[0]);
//        indices.push_back(point[1]);
//        indices.push_back(point[2]);
        getline(ss, useless);//空行
        getline(ss, useless);//end loop
        getline(ss, useless);//end facet

        buffer=strstr(buffer,"normal");
        buffer=strstr(buffer,"facet normal");
        cur++;
        fraction=float(cur)/numberTriangles;
        emit progressReport(100*fraction,100.0f);
        QApplication::processEvents();
        //cout<<"facenumber:"<<numberTriangles<<" "<<dataset.mesh.num_faces()<<endl;
    }while(buffer!=NULL);
    delete[] point;
    return true;
}

void ReadSTLFile::ExitRead(){
    numberTriangles=0;
    numberVertices=0;
    modelsize=0;
    normalList.clear();
    verticesmap.clear();
    dataset->mesh.clear();
    isstop=true;
}
