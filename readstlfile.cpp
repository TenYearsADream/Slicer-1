#pragma once
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <QDebug>
#include <QtCore/qmath.h>
#include"Point3f.h"
#include"readstlfile.h"

using namespace std;

bool ReadSTLFile::ReadStlFile(const char *cfilename)
{
    FILE * pFile;
    long lSize;
    char* buffer;
    size_t result;

    /* 若要一个byte不漏地读入整个文件，只能采用二进制方式打开 */
    fopen_s(&pFile,cfilename, "rb");//fopen_s(指针,文件名,使用文件方式);其中指针是用来接收<指向打开的文件的指针>的指针
    if (pFile == NULL)
    {
        fputs("File error", stderr);
        //exit(1);
    }

    /* 获取文件大小 */
    fseek(pFile, 0, SEEK_END);//控制件指针偏移
    lSize = ftell(pFile);//得到文件位置指针当前位置相对于文件首的偏移字节数
    rewind(pFile);//文件位置为给定流文件的开头

    /* 分配内存存储整个文件 */
    buffer = (char*)malloc(sizeof(char)*lSize);
    if (buffer == NULL)
    {
        fputs("Memory error", stderr);
        //exit(2);
    }

    /* 将文件拷贝到buffer中 */
    result = fread(buffer, 1, lSize, pFile);//从给定流pFile读取数据到Buffer所指向的数组中
    if (result != lSize)
    {
        fputs("Reading error", stderr);
        //exit(3);
    }


    /* 结束演示，关闭文件并释放内存 */
    fclose(pFile);

    ios::sync_with_stdio(false);
    if (buffer[0]=='s')//判断格式
    {
        std::cout<<"ASCII文件"<<endl;
        vector<Point3f>().swap(normalList);//清空vector
        vector<vector<size_t>>().swap(faceList);
        ReadASCII(buffer);
    }
    else
    {
        std::cout<<"Binary文件"<<endl;
        vector<Point3f>().swap(normalList);
        vector<vector<size_t>>().swap(faceList);
        ReadBinary(buffer);
    }
    ios::sync_with_stdio(true);

    free(buffer);//释放内存
    return true;
}

bool ReadSTLFile::ReadASCII(const char *buffer)
{
    unTriangles = 0;
    float x, y, z;
    char idx[10],idy[10],idz[10];
    size_t index;
    long long ID;
    vector<size_t> point(3);
    hashtable = new HashTable;
    string name, useless,id;
    stringstream ss(buffer),stream;
    getline(ss, name);//文件路径及文件名
    ss.get();
    //提取第一个面片，得到最大最小值初始值
    ss >> useless;//facet
    ss >> useless >> x >> y >>z;//法向量
    normalList.push_back(Point3f(x, y, z));
    getline(ss, useless);
    getline(ss, useless);//outer loop
    for (int i = 0; i < 3; i++)
    {
        ss >> useless >> x >> y >> z;
        sprintf_s(idx, "%.2f", qAbs(x));sprintf_s(idy, "%.2f",qAbs(y));sprintf_s(idz, "%.2f", qAbs(z));
        id=(string)idx+(string)idy+(string)idz;
        id=id.replace(id.find("."),1,"");
        id=id.replace(id.find("."),1,"");
        id=id.replace(id.find("."),1,"");
        stream.clear();
        stream<<id;stream>>ID;
        index=hashtable->addPoint(ID,Point3f(x,y,z));
        point[i]=index;
    }
    faceList.push_back(point);
    surroundBox[0]=x;
    surroundBox[1]=x;
    surroundBox[2]=y;
    surroundBox[3]=y;
    surroundBox[4]=z;
    surroundBox[5]=z;
    unTriangles++;
    getline(ss, useless);//空行
    getline(ss, useless);//end loop
    getline(ss, useless);//end facet

    //读取其他面片
    do {
        ss >> useless;//facet
        if (useless != "facet")
            break;
        ss >> useless >> x >> y >>z;//法向量
        normalList.push_back(Point3f(x, y, z));
        getline(ss, useless);
        getline(ss, useless);//outer loop
        for (int i = 0; i < 3; i++)
        {
            ss >> useless >> x >> y >> z;
            surroundBox[0]=qMin(surroundBox[0],x);
            surroundBox[1]=qMax(surroundBox[1],x);
            surroundBox[2]=qMin(surroundBox[2],y);
            surroundBox[3]=qMax(surroundBox[3],y);
            surroundBox[4]=qMin(surroundBox[4],z);
            surroundBox[5]=qMax(surroundBox[5],z);
            sprintf_s(idx, "%.2f", qAbs(x));sprintf_s(idy, "%.2f",qAbs(y));sprintf_s(idz, "%.2f", qAbs(z));
            id=(string)idx+(string)idy+(string)idz;
            id=id.replace(id.find("."),1,"");
            id=id.replace(id.find("."),1,"");
            id=id.replace(id.find("."),1,"");
            stream.clear();stream<<id;stream>>ID;//string转long
            index=hashtable->addPoint(ID,Point3f(x,y,z));
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
    char name[80],idx[10],idy[10],idz[10];
    float x,y,z;
    size_t index;
    long long ID;
    string id;
    vector<size_t> point(3);
    stringstream stream;
    hashtable = new HashTable;
    memcpy(name, p, 80);//80字节文件头
    //cout<<name<<endl;
    p += 80;
    unTriangles= cpyint(p);//4字节三角面片个数
    //提取第一个面片，得到最大最小值初始值
    normalList.push_back(Point3f(cpyfloat(p), cpyfloat(p), cpyfloat(p)));//法向量
    for (int j = 0; j < 3; j++)//读取三顶点
    {
        x=cpyfloat(p);y=cpyfloat(p);z=cpyfloat(p);
        sprintf_s(idx, "%.2f", qAbs(x));sprintf_s(idy, "%.2f",qAbs(y));sprintf_s(idz, "%.2f", qAbs(z));
        id=(string)idx+(string)idy+(string)idz;
        id=id.replace(id.find("."),1,"");
        id=id.replace(id.find("."),1,"");
        id=id.replace(id.find("."),1,"");
        stream.clear();stream<<id;stream>>ID;//string转long
        //cout<<"ID:"<<ID<<endl;
        index=hashtable->addPoint(ID,Point3f(x,y,z));
        point[j]=index;
    }
    faceList.push_back(point);
    surroundBox[0]=x;
    surroundBox[1]=x;
    surroundBox[2]=y;
    surroundBox[3]=y;
    surroundBox[4]=z;
    surroundBox[5]=z;
    p += 2;//跳过尾部标志
    //读取其他面片
    for (unsigned int i = 1; i < unTriangles; i++)
    {
        normalList.push_back(Point3f(cpyfloat(p), cpyfloat(p), cpyfloat(p)));//法向量
        for (int j = 0; j < 3; j++)//读取三顶点
        {
            x=cpyfloat(p);y=cpyfloat(p);z=cpyfloat(p);
            surroundBox[0]=qMin(surroundBox[0],x);
            surroundBox[1]=qMax(surroundBox[1],x);
            surroundBox[2]=qMin(surroundBox[2],y);
            surroundBox[3]=qMax(surroundBox[3],y);
            surroundBox[4]=qMin(surroundBox[4],z);
            surroundBox[5]=qMax(surroundBox[5],z);
            sprintf_s(idx, "%.2f", qAbs(x));sprintf_s(idy, "%.2f",qAbs(y));sprintf_s(idz, "%.2f", qAbs(z));
            id=(string)idx+(string)idy+(string)idz;
            id=id.replace(id.find("."),1,"");
            id=id.replace(id.find("."),1,"");
            id=id.replace(id.find("."),1,"");
            stream.clear();stream<<id;stream>>ID;//string转long
            //cout<<"ID:"<<ID<<endl;
            index=hashtable->addPoint(ID,Point3f(x,y,z));
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
