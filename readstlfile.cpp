#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <QDebug>
#include <QFile>
#include <QtCore/qmath.h>
#include"readstlfile.h"

using namespace std;

bool ReadSTLFile::ReadStlFile(const QString filename)
{
    QFile file(filename);
    char* buffer;
    buffer=(char *) malloc(file.size());


    if(file.open(QIODevice::ReadOnly))
    {
        file.read(buffer,file.size());
    }

    ios::sync_with_stdio(false);
    if (buffer[0]=='s')//判断格式
    {
        std::cout<<"File is ASCII"<<endl;
        vector<Point>().swap(normalList);//清空vector
        vector<vector<int>>().swap(faceList);
        ReadASCII(buffer);
    }
    else
    {
        std::cout<<"Binary文件"<<endl;
        vector<Point>().swap(normalList);
        vector<vector<int>>().swap(faceList);
        ReadBinary(buffer);
    }
    ios::sync_with_stdio(true);
    file.close();
    free(buffer);//释放内存
    return true;
}

bool ReadSTLFile::ReadASCII(const char *buffer)
{
    unTriangles = 0;
    float x, y, z;
    int index;
    vector<int> point(3);
    hashtable = new HashTable;
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
            QString strx = QString::number(sqrt(qAbs(x)), 'f', 3);
            QString stry = QString::number(sqrt(qAbs(y)), 'f', 3);
            QString strz = QString::number(sqrt(qAbs(z)), 'f', 3);
            strx=strx.replace(".","");
            stry=stry.replace(".","");
            strz=strz.replace(".","");
            if(x>0)strx="1"+strx.left(3);
            else   strx="0"+strx.left(3);
            if(y>0)stry="1"+stry.left(3);
            else   stry="0"+stry.left(3);
            if(z>0)strz="1"+strz.left(3);
            else   strz="0"+strz.left(3);
            QString key="1"+strx+stry+strz;
            index=hashtable->addPoint(key,Point(x,y,z));
            point[i]=index;
        }
        faceList.push_back(point);
        unTriangles++;
        getline(ss, useless);//空行
        getline(ss, useless);//end loop
        getline(ss, useless);//end facet
    } while (1);
    return true;
}

bool ReadSTLFile::ReadBinary(const char *buffer)
{
    const char* p = buffer;
    float x,y,z;
    int index;
    char* name;
    vector<int> point(3);
    hashtable = new HashTable;
    memcpy(name, p, 80);//80字节文件头
    //cout<<name<<endl;
    p += 80;
    unTriangles= cpyint(p);//4字节三角面片个数
    //读取其他面片
    for (unsigned int i = 0; i < unTriangles; i++)
    {
        normalList.push_back(Point(cpyfloat(p), cpyfloat(p), cpyfloat(p)));//法向量
        for (int j = 0; j < 3; j++)//读取三顶点
        {
            x=cpyfloat(p);y=cpyfloat(p);z=cpyfloat(p);
            QString strx = QString::number(sqrt(qAbs(x)), 'f', 3);
            QString stry = QString::number(sqrt(qAbs(y)), 'f', 3);
            QString strz = QString::number(sqrt(qAbs(z)), 'f', 3);
            strx=strx.replace(".","");
            stry=stry.replace(".","");
            strz=strz.replace(".","");
            if(x>0)strx="1"+strx.left(3);
            else   strx="0"+strx.left(3);
            if(y>0)stry="1"+stry.left(3);
            else   stry="0"+stry.left(3);
            if(z>0)strz="1"+strz.left(3);
            else   strz="0"+strz.left(3);
            QString key="1"+strx+stry+strz;
            index=hashtable->addPoint(key,Point(x,y,z));
            point[j]=index;
        }
        faceList.push_back(point);
        p += 2;//跳过尾部标志
    }
    return true;
}

int ReadSTLFile::NumTri()
{
    return unTriangles;
    cout<<unTriangles<<endl;
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
