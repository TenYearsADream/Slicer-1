#pragma once
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <QDebug>
#include <QtCore/qmath.h>
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

    free(buffer);//释放内存
    return true;
}

bool ReadSTLFile::ReadASCII(const char *buffer)
{
    unTriangles = 0;
    float x, y, z;
    char idx[10],idy[10],idz[10];
    int index;
    vector<int> point(3);
    hashtable = new HashTable;
    string name, useless,key;
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
            sprintf_s(idx, "%.3f", sqrt(qAbs(x)));sprintf_s(idy, "%.3f",sqrt(qAbs(y)));sprintf_s(idz, "%.3f",sqrt(qAbs(z)));
            string strx=(string)idx;string stry=(string)idy;string strz=(string)idz;
            strx=strx.replace(strx.find("."),1,"");
            stry=stry.replace(stry.find("."),1,"");
            strz=strz.replace(strz.find("."),1,"");
            if(x>0)strx="1"+strx.substr(0,3);
            else   strx="0"+strx.substr(0,3);
            if(y>0)stry="1"+stry.substr(0,3);
            else   stry="0"+stry.substr(0,3);
            if(z>0)strz="1"+strz.substr(0,3);
            else   strz="0"+strz.substr(0,3);
            //cout<<strx<<" "<<stry<<" "<<strz<<endl;
            key="1"+strx+stry+strz;
            //cout<<"id:"<<id<<endl;
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
    char name[80],idx[10],idy[10],idz[10];
    float x,y,z;
    int index;
    string key;
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
            sprintf_s(idx, "%.3f", sqrt(qAbs(x)));sprintf_s(idy, "%.3f",sqrt(qAbs(y)));sprintf_s(idz, "%.3f",sqrt(qAbs(z)));
            string strx=(string)idx;string stry=(string)idy;string strz=(string)idz;
            strx=strx.replace(strx.find("."),1,"");
            stry=stry.replace(stry.find("."),1,"");
            strz=strz.replace(strz.find("."),1,"");
            if(x>0)strx="1"+strx.substr(0,4);
            else   strx="0"+strx.substr(0,4);
            if(y>0)stry="1"+stry.substr(0,4);
            else   stry="0"+stry.substr(0,4);
            if(z>0)strz="1"+strz.substr(0,4);
            else   strz="0"+strz.substr(0,4);
            //cout<<strx<<" "<<stry<<" "<<strz<<endl;
            key="1"+strx+stry+strz;
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
