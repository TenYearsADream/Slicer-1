#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <QDebug>
#include <QFile>
#include <QtCore/qmath.h>
#include <QProgressDialog>
#include <QMessageBox>
#include"readstlfile.h"

using namespace std;

bool ReadSTLFile::ReadStlFile(const QString filename)
{
    QFile file(filename);
    uchar* buffer;

    if(file.open(QIODevice::ReadOnly))
    {
        buffer=file.map(0,file.size());
    }
    if(buffer)
    {
        if (buffer[0]=='s')//判断格式
        {
            qDebug()<<"File is ASCII";
            normalList.clear();//清空vector
            dataset.mesh.clear();
            //ReadASCII((char*)buffer);
            if(MyReadASCII((char*)buffer))
            {
                file.unmap(buffer);
                file.close();
                return true;
            }
            else
            {
                file.unmap(buffer);
                file.close();
                return false;
            }
        }
        else
        {
            qDebug()<<"File is Binary";
            normalList.clear();//清空vector
            dataset.mesh.clear();
            if(ReadBinary((char*)buffer))
            {
                file.unmap(buffer);
                file.close();
                return true;
            }
            else
            {
                file.unmap(buffer);
                file.close();
                return false;
            }
        }

    }
    else
    {
        cout<<"out of memory error"<<endl;
        return false;
    }
}

bool ReadSTLFile::ReadASCII(const char *buffer)
{
    numberVertices=0;
    numberTriangles = 0;
    float x, y, z;
    int index;
    int point[3];
    string name, useless;
    stringstream ss(buffer);
    getline(ss, name);//文件路径及文件名
    ss.get();
    //读取面片
    do {
        ss >> useless;//facet
        if (useless != "facet")
            break;
        ss >> useless >> x >> y >>z;//法向量
        normalList.push_back(Point(x, y, z));
        getline(ss, useless);
        getline(ss, useless);//outer loop
        for (int i = 0; i < 3; i++)
        {
            ss >> useless >> x >> y >> z;
            //cout<<x<<" "<<y<<" "<<z<<endl;
            QString strx = QString::number(sqrt(qAbs(x)), 'f', 8);
            QString stry = QString::number(sqrt(qAbs(y)), 'f', 8);
            QString strz = QString::number(sqrt(qAbs(z)), 'f', 8);
            //qDebug()<<strx<<" "<<stry<<" "<<strz;
            strx=strx.replace(".","");
            stry=stry.replace(".","");
            strz=strz.replace(".","");
            if(x>0)strx="1"+strx.left(8);
            else   strx="0"+strx.left(8);
            if(y>0)stry="1"+stry.left(8);
            else   stry="0"+stry.left(8);
            if(z>0)strz="1"+strz.left(8);
            else   strz="0"+strz.left(8);
            QString key="1"+strx+stry+strz;
            index=addPoint(key,Point(x,y,z));
            point[i]=index;
        }
        Mesh::Vertex_index vx(point[0]);
        Mesh::Vertex_index vy(point[1]);
        Mesh::Vertex_index vz(point[2]);
        dataset.mesh.add_face(vx,vy,vz);
        numberTriangles++;
        getline(ss, useless);//空行
        getline(ss, useless);//end loop
        getline(ss, useless);//end facet
    } while (1);
    return true;
}

bool ReadSTLFile::ReadBinary(const char *buffer)
{
    const char* p = buffer;
    float x=0,y=0,z=0;
    int index=0;
    char* name;
    vector<int> point(3);
    memcpy(name, p, 80);//80字节文件头
    //qDebug()<<name<<endl;
    p += 80;
    numberVertices=0;
    numberTriangles= cpyint(p);//4字节三角面片个数
    //读取三角形面片
    for (unsigned int i = 0; i < numberTriangles; i++)
    {
        normalList.push_back(Point(cpyfloat(p), cpyfloat(p), cpyfloat(p)));//法向量
        for (int j = 0; j < 3; j++)//读取三顶点
        {
            x=cpyfloat(p);y=cpyfloat(p);z=cpyfloat(p);
            QString strx = QString::number(sqrt(qAbs(x)), 'f', 8);
            QString stry = QString::number(sqrt(qAbs(y)), 'f', 8);
            QString strz = QString::number(sqrt(qAbs(z)), 'f', 8);
            strx=strx.replace(".","");
            stry=stry.replace(".","");
            strz=strz.replace(".","");
            if(x>0)strx="1"+strx.left(8);
            else   strx="0"+strx.left(8);
            if(y>0)stry="1"+stry.left(8);
            else   stry="0"+stry.left(8);
            if(z>0)strz="1"+strz.left(8);
            else   strz="0"+strz.left(6);
            QString key="1"+strx+stry+strz;
            index=addPoint(key,Point(x,y,z));
            point[j]=index;
        }
        Mesh::Vertex_index vx(point[0]);
        Mesh::Vertex_index vy(point[1]);
        Mesh::Vertex_index vz(point[2]);
        dataset.mesh.add_face(vx,vy,vz);
        p += 2;//跳过尾部标志
    }
    return true;
}

int ReadSTLFile::cpyint(const char*& p)
{
    int cpy;
    memwriter = (char*)&cpy;
    memcpy(memwriter, p, 4);
    p += 4;
    return cpy;
}

float ReadSTLFile::cpyfloat(const char*& p)
{
    float cpy;
    memwriter = (char*)&cpy;
    memcpy(memwriter, p, 4);
        p += 4;
    return cpy;
}

int ReadSTLFile::addPoint(QString key,Point point){
    int index;
    auto it = verticesmap.find(key);
    if(it != verticesmap.end())
    {
        index=it.value();
        //cout<<"索引："<<it.value()<<endl;
    }
    else
    {
        Mesh::Vertex_index v0=dataset.mesh.add_vertex(point);
        verticesmap.insert(key,numberVertices);
        index=numberVertices;
        numberVertices++;
    }
    return index;
}

bool ReadSTLFile::MyReadASCII(const char *buf)
{
//    int size=strlen(buf);
//    QProgressDialog *progressDlg=new QProgressDialog();
//    progressDlg->setWindowModality(Qt::WindowModal);
//    progressDlg->setMinimumDuration(0);
//    progressDlg->setAttribute(Qt::WA_DeleteOnClose, true);
//    progressDlg->setWindowTitle("导入模型");
//    progressDlg->setLabelText("正在建立拓扑关系......");
//    progressDlg->setRange(0,size);

    int offset=280;
    numberVertices=0;
    numberTriangles = 0;
    float x, y, z;
    int point[3],index;
    char *buffer=strstr(buf,"facet normal");
    int namelength=buffer-buf;
    char name[namelength];
    strncpy(name,buf,sizeof(name));
    name[namelength]='\0';
    char facet[offset];
    string useless;
    do
    {
//        progressDlg->setValue(numberTriangles*offset);
//        if(progressDlg->wasCanceled())
//        {
//            dataset.mesh.clear();
//            QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("取消导入"));
//            return false;
//        }
        strncpy(facet,buffer,sizeof(facet));
        facet[offset]='\0';
        //cout<<facet<<endl;
        stringstream ss(facet);
        ss >> useless;//facet
        ss >> useless >> x >> y >>z;//法向量
        normalList.push_back(Point(x, y, z));
        getline(ss, useless);
        getline(ss, useless);//outer loop
        for (int i = 0; i < 3; i++)
        {
            ss >> useless >> x >> y >> z;
            //cout<<x<<" "<<y<<" "<<z<<endl;
            QString strx = QString::number(sqrt(qAbs(x)), 'f', 8);
            QString stry = QString::number(sqrt(qAbs(y)), 'f', 8);
            QString strz = QString::number(sqrt(qAbs(z)), 'f', 8);
            //qDebug()<<strx<<" "<<stry<<" "<<strz;
            strx=strx.replace(".","");
            stry=stry.replace(".","");
            strz=strz.replace(".","");
            if(x>0)strx="1"+strx.left(8);
            else   strx="0"+strx.left(8);
            if(y>0)stry="1"+stry.left(8);
            else   stry="0"+stry.left(8);
            if(z>0)strz="1"+strz.left(8);
            else   strz="0"+strz.left(8);
            QString key="1"+strx+stry+strz;
            index=addPoint(key,Point(x,y,z));
            point[i]=index;
        }
        Mesh::Vertex_index vx(point[0]);
        Mesh::Vertex_index vy(point[1]);
        Mesh::Vertex_index vz(point[2]);
        dataset.mesh.add_face(vx,vy,vz);
        getline(ss, useless);//空行
        getline(ss, useless);//end loop
        getline(ss, useless);//end facet

        buffer=strstr(buffer,"normal");
        buffer=strstr(buffer,"facet normal");
        numberTriangles++;
    }while(buffer!=NULL);
    //progressDlg->close();
    return true;
}
