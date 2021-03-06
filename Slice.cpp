﻿#include "Slice.h"
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include "loadprogressbar.h"
#include <QApplication>
using namespace std;
Slice::Slice(QObject *parent):QObject(parent)
{
    thick=10;
    layernumber=0;
    zheight=0.0f;
    findtime=0;
    comptime=0;
    createtime=0;
}

Slice::~Slice()
{
    delete(progressbar);
}

void Slice::startSlice(vector<cl_float3> &vertex,vector<cl_uint4> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints)
{    
    findtime=0;
    comptime=0;
    sorttime=0;
    progressbar=new loadProgressBar("slice...");
    connect(this,SIGNAL(progressReport(float,float)),progressbar,SLOT(setProgressBar(float,float)));
    if(sliceType=="CPU")
    {
        //sliceByHeight(mesh,zmin,zmax);
        sliceOnCpu(vertex,halfedge,surroundBox,intrpoints);
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
        cout<<"compute intersect time:"<<comptime<<"ms"<<endl;
        cout<<"time of CPU computing:"<<findtime+sorttime+comptime<<"ms"<<endl;
        emit outputMsg("find edge time: "+QString::number(findtime)+"ms");
        emit outputMsg("sort edge time: "+QString::number(sorttime)+"ms");
        emit outputMsg("compute intersect time: "+QString::number(comptime)+"ms");
        emit outputMsg("CPU total time: "+QString::number(findtime+sorttime+comptime)+"ms");
        bool success=genSlicesFile(slicepath[0],intrpoints,surroundBox);
        if(success)
        {
            emit outputMsg("slc file generated successfully to"+slicepath[0]+".");
            cout<<"slc file generated successfully."<<endl;
        }
    }
    if(sliceType=="GPU")
    {
        //sliceByGpu(vertex,halfedge,surroundBox,intrpoints);
        bool success=sliceOnGpu(vertex,halfedge,surroundBox,intrpoints);
        if(!success)
        {
            emit outputMsg("failed to execute kernel on GPU");
            return;
        }
        else
        {
            cout<<"find edge time:"<<findtime<<"ms"<<endl;
            cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
            cout<<"compute intersect time:"<<comptime<<"ms"<<endl;
            cout<<"time of parallel computing:"<<findtime+sorttime+comptime<<"ms"<<endl;
            emit outputMsg("find edge time: "+QString::number(findtime)+"ms");
            emit outputMsg("sort edge time: "+QString::number(sorttime)+"ms");
            emit outputMsg("compute intersect time: "+QString::number(comptime)+"ms");
            emit outputMsg("GPU total time: "+QString::number(findtime+sorttime+comptime)+"ms");
            if(genSlicesFile(slicepath[1],intrpoints,surroundBox))
            {
                emit outputMsg("slc file generated successfully to"+slicepath[1]+".");
                cout<<"slc file generated successfully."<<endl;
            }
        }
    }
    if(sliceType=="CPU2")
    {
        //sliceByHeight(mesh,zmin,zmax);
        sliceByCpu(vertex,halfedge,surroundBox,intrpoints);
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
        cout<<"compute intersect time:"<<comptime<<"ms"<<endl;
        cout<<"time of CPU2 computing:"<<findtime+sorttime+comptime<<"ms"<<endl;
        emit outputMsg("find edge time: "+QString::number(findtime)+"ms");
        emit outputMsg("sort edge time: "+QString::number(sorttime)+"ms");
        emit outputMsg("compute intersect time: "+QString::number(comptime)+"ms");
        emit outputMsg("CPU2 total time: "+QString::number(findtime+sorttime+comptime)+"ms");
        if(genSlicesFile(slicepath[0],intrpoints,surroundBox))
        {
            emit outputMsg("slc file generated successfully to"+slicepath[0]+".");
            cout<<"slc file generated successfully."<<endl;
        }
    }
}

void Slice::sliceByCpu(vector<cl_float3> &vertex,vector<cl_uint4> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints)
{
    float zmin=surroundBox[4];
    float zmax=surroundBox[5];
    z.clear();
    zheight=zmin;
    layernumber=0;
    while(zheight<=zmax)
    {
        z.push_back(zheight);
        zheight += thick;
        layernumber++;
    }
    float fraction=0.05f;
    emit progressReport(100*fraction,100.0f);
    QApplication::processEvents();

    //将所有半边分组
    time.start();
    vector<vector<uint>>edges;
    edges.resize(layernumber+1);
    for(uint i=0;i<halfedge.size();i++)
    {
        float z1=qMin(vertex[halfedge[i].x].z,vertex[halfedge[i].y].z);
        float z2=qMax(vertex[halfedge[i].x].z,vertex[halfedge[i].y].z);
        int num1=int(ceil((z1-zmin)/thick));
        int num2=int((z2-zmin)/thick);
        if(qAbs(z2-z1)<1e-8f)num2=num1-1;
        //cout<<z1<<" "<<z2<<" "<<num1<<" "<<num2<<" "<<zmin<<" "<<thick<<endl;
        for(int j=num1;j<=num2;j++)
        {
            edges[uint(j)].push_back(i);
        }
//        fraction=0.05f+float(i+1)/halfedge.size()*0.05f;
//        emit progressReport(100*fraction,100.0f);
//        QApplication::processEvents();
    }
    while(edges.back().empty())
    {
        edges.pop_back();
    }
    layernumber=edges.size();
    findtime =time.elapsed();
    fraction=0.1f;
    emit progressReport(100*fraction,100.0f);
    progressbar->setLabelText("group edges done!");
    QApplication::processEvents();
    cout<<"group edges done!"<<endl;

    //同组中的半边进行重排序，组成首尾相连的轮廓
    time.restart();
    size_t linesnumber=0;
    vector<vector<vector<uint>>> location;
    location.resize(layernumber);
    {
        QMultiHash<uint,uint>facesmap;
        vector<vector<uint>>locs;
        locs.reserve(100);
        vector<uint> loc;
        uint current=0;
        uint index=0;
        for(uint i=0;i<edges.size();i++)
        {
            //cout<<"The "<<i<<" layer:"<<edges[i].size()<<endl;
            locs.clear();
            facesmap.clear();
            if(edges[i].empty())
            {
                location[i]=locs;
                continue;
            }
            if(edges[i].size()>linesnumber)linesnumber=edges[i].size();
            facesmap.reserve(int(edges[i].size()*2));
            loc.reserve(edges[i].size());
            for(uint j=0;j<edges[i].size();j++)
            {
                //cout<<halfedge[edges[i][j]].z<<" ";
                facesmap.insert(halfedge[edges[i][j]].z,j);
                facesmap.insert(halfedge[edges[i][j]].w,j);
            }
            //cout<<endl;
//            qDebug()<<facesmap.count();
//            QHash<uint,uint>::const_iterator it;
//            for(it=facesmap.begin();it!=facesmap.end();it++)
//            {
//                qDebug()<<it.key()<<it.value();
//            }
            while(!facesmap.empty())
            {
                loc.clear();
                current=facesmap.constBegin().key();
                loc.push_back(facesmap.value(current));
                while(1)
                {
                    auto find_index = facesmap.find(current);
                    if(find_index==facesmap.end())
                    {
                        break;
                    }
                    if(find_index.value()==loc.back())find_index++;
                    if(find_index.key()!=current)
                    {
                        facesmap.remove(current);
                        break;
                    }
                    index=find_index.value();
                    loc.push_back(index);
                    if(halfedge[edges[i][index]].z != current)
                    {
                        current=halfedge[edges[i][index]].z;
                    }
                    else
                    {
                        current=halfedge[edges[i][index]].w;
                    }             
                    facesmap.remove(find_index.key());
                };
                if(loc.front()==loc.back() &&loc.size()>2)
                {
                    //cout<<loc.size()<<endl;
                    loc.pop_back();
                    locs.push_back(loc);
                }
            }
            //cout<<"The "<<i<<" layer:"<<locs.size()<<endl;
            location[i]=locs;
//            fraction=0.1f+float(i+1)/edges.size()*0.8f;
//            emit progressReport(100*fraction,100.0f);
//            QApplication::processEvents();
        }
    }
    sorttime =time.elapsed();
    fraction=0.9f;
    emit progressReport(100*fraction,100.0f);
    progressbar->setLabelText("sort edges done!");
    QApplication::processEvents();
    cout<<"sort edges done!"<<endl;

    //计算每组中的半边和相应的z平面求交
    time.restart();
    Lines lines;
    Polylines polylines;
    intrpoints.clear();
    intrpoints.reserve(layernumber);
    cl_float3 v1,v2;
    for(uint i=0;i<layernumber;i++)
    {
        polylines.clear();
        polylines.reserve(location[i].size());
        //cout<<"The "<<i<<" layer:"<<edges[i].size()/2<<endl;
        for(size_t j=0;j<location[i].size();j++)
        {
            lines.clear();
            lines.reserve(location[i][j].size());
            //cout<<"loop "<<j<<":"<<endl;
            for(size_t k=0;k<location[i][j].size();k++)
            {
                v1=vertex[halfedge[edges[i][location[i][j][k]]].x];
                v2=vertex[halfedge[edges[i][location[i][j][k]]].y];
                float diffx=v1.x-v2.x;
                float diffy=v1.y-v2.y;
                float diffz=v1.z-v2.z;
                float x=v1.x+diffx*(z[i]-v1.z)/diffz;
                float y=v1.y+diffy*(z[i]-v1.z)/diffz;
                lines.push_back(Point(x,y,z[i]));
            }
            polylines.push_back(lines);
        }
        intrpoints.push_back(polylines);
//        fraction=0.9f+float(i+1)/layernumber*0.1f;
//        emit progressReport(100*fraction,100.0f);
//        QApplication::processEvents();
    }
    comptime +=time.elapsed();
    fraction=1.0f;
    emit progressReport(100*fraction,100.0f);
    progressbar->setLabelText("intersect edges done!");
    QApplication::processEvents();
    cout<<"intersect edges done!"<<endl;
}

bool Slice::genSlicesFile(const QString fileName,const vector<Polylines> intrpoints,float surroundBox[6])
{
    char buff[4];
    char charbuf[256];
    FILE *stream;
    QByteArray ba = fileName.toLocal8Bit();
    float value[2];
    QTime startTime = QTime::currentTime();
    if((stream=fopen(ba.data(),"wb"))==NULL)
    {
        cout<<"Cannot open output slc file."<<endl;
        return false;
    }

    QString slcHeader = "-SLCVER " + QString::number(2.0, 'f', 1)
            + " -UNIT " + "MM"
            + " -TYPE "  + "PART"
            + " -PACKAGE "  + "MATERIALISE C-TOOLS 2.xx"
            + " -EXTENTS " + QString::number(double(surroundBox[0]), 'f', 6) + ","
            + QString::number(double(surroundBox[1]), 'f', 6) + " "
            + QString::number(double(surroundBox[2]), 'f', 6) + ","
            + QString::number(double(surroundBox[3]), 'f', 6) + " "
            + QString::number(double(surroundBox[4]), 'f', 6) + ","
            + QString::number(double(surroundBox[5]), 'f', 6) + " ";

    fwrite(slcHeader.toLocal8Bit().data(), 1,slcHeader.toLocal8Bit().size(),stream);
    buff[0] = 0x0D;
    buff[1] = 0x0A;
    buff[2] = 0x1A;
    fwrite(buff,1,3,stream);
    memset(charbuf, 0x20, sizeof(charbuf));
    fwrite(charbuf, 1, sizeof(charbuf), stream);
    buff[0] =1;
    fwrite(buff, 1, 1, stream);
    float linewidecompen=0.025f;
    float reserved=0.025f;
    fwrite(&surroundBox[0], sizeof(float), 1, stream);
    fwrite(&thick, sizeof(float), 1, stream);
    fwrite(&linewidecompen, sizeof(float), 1, stream);
    fwrite(&reserved, sizeof(float), 1,  stream);
    for (uint i = 0; i < intrpoints.size(); i++) {
        value[0] =z[i];
        fwrite(&value[0], sizeof(float), 1, stream);
        uint numb_outerBoundaries=uint(intrpoints[i].size());
        fwrite(&numb_outerBoundaries, sizeof(numb_outerBoundaries), 1, stream);
        for(uint j = 0; j < intrpoints[i].size(); j++) {
            uint numb_verts=uint(intrpoints[i][j].size()+1);
            uint numb_gaps=0;
            fwrite(&numb_verts, sizeof(unsigned), 1, stream);
            fwrite(&numb_gaps, sizeof(unsigned), 1, stream);
            for(uint k = 0; k < intrpoints[i][j].size(); k++) {
                value[0] = float(intrpoints[i][j][k].x());
                fwrite(&value[0], sizeof(float), 1, stream);
                value[0] = float(intrpoints[i][j][k].y());
                fwrite(&value[0], sizeof(float), 1,  stream);
            }
            value[0] = float(intrpoints[i][j][0].x());
            fwrite(&value[0], sizeof(float), 1, stream);
            value[0] = float(intrpoints[i][j][0].y());
            fwrite(&value[0], sizeof(float), 1,  stream);
        }
    }
    float  maxZlays = z.back();
    fwrite(&maxZlays, sizeof(float), 1, stream);
    memset(buff, 0xff, 4);
    fwrite(buff,1,4,stream);
    fclose(stream);
    emit outputMsg("genSlicesFile time: "+QString::number(startTime.msecsTo(QTime::currentTime()))+"ms");
    qDebug()<<"genSlicesFile time:"<< startTime.msecsTo(QTime::currentTime())<<" ms";
    return true;
}

bool Slice::sliceOnGpu(vector<cl_float3> &vertex,vector<cl_uint4> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints)
{
    float zmin=surroundBox[4];
    float zmax=surroundBox[5];
    z.clear();
    zheight=zmin;
    layernumber=0;
    while(zheight<=zmax)
    {
        z.push_back(zheight);
        zheight += thick;
        layernumber++;
//        if(layernumber>179700)
//        {
//            cout<<layernumber<<endl;
//        }
    }
    float fraction=0.05f;
    emit progressReport(100*fraction,100.0f);
    QApplication::processEvents();
    //将所有半边分组
    time.start();
    size_t total=0;
    vector<vector<uint>>edges;
    edges.resize(layernumber+1);
    for(uint i=0;i<halfedge.size();i++)
    {
        float z1=qMin(vertex[halfedge[i].x].z,vertex[halfedge[i].y].z);
        float z2=qMax(vertex[halfedge[i].x].z,vertex[halfedge[i].y].z);
        int num1=int(ceil((z1-zmin)/thick));
        int num2=int((z2-zmin)/thick);
        if(qAbs(z2-z1)<1e-8f)num2=num1-1;
        //cout<<z1<<" "<<z2<<" "<<num1<<" "<<num2<<" "<<zmin<<" "<<thick<<endl;
        for(int j=num1;j<=num2;j++)
        {
            edges[uint(j)].push_back(i);
            total++;
        }
//        fraction=0.05f+float(i+1)/halfedge.size()*0.05f;
//        emit progressReport(100*fraction,100.0f);
//        QApplication::processEvents();
    }
    while(edges.back().empty())
    {
        edges.pop_back();
    }
    layernumber=edges.size();
    findtime =time.elapsed();
//    cout<<"total: "<<total<<endl;
    fraction=0.1f;
    emit progressReport(100*fraction,100.0f);
    progressbar->setLabelText("group edges done!");
    QApplication::processEvents();
    cout<<"group edges done!"<<endl;

    QTime time2;
    time.restart();
    Lines lines;
    Polylines polylines;
    intrpoints.clear();
    intrpoints.resize(layernumber);
    cl_float3 v1,v2;

    uint startlayer=0,endlayer=0,layerwidth=uint(layernumber);
    vector<cl_uint2> faceset;
    cl_uint2 singleedge;
    vector<uint>linesnumber;
    linesnumber.reserve(layernumber);
    faceset.reserve(total);
    const uint LOOPs=20000;
    const uint MAXFACESETSIZE=uint(opencl.deviceinfo.maxMemAllocSize)*1024*1024/(16*sizeof(uint));
//    QFile f("C:/Users/xjtu_/Desktop/cpu.txt");
//    if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        cout << "Open failed." << endl;
//    }
//    QTextStream txtOutput(&f);
    while(endlayer<layernumber-1)
    {
        time2.start();
        linesnumber.clear();
        faceset.clear();
        //cout<<"startlayer: "<<startlayer<<endl;
        for(uint i=startlayer;i<layernumber;i++)
        {
            if(i>startlayer)
            {
                if(uint(edges[i].size())+linesnumber[i-startlayer-1]>MAXFACESETSIZE)
                {
                    endlayer=i;
                    break;
                }
                else {
                    linesnumber.push_back(uint(edges[i].size())+linesnumber[i-startlayer-1]);
                }
            }
            else
            {
                linesnumber.push_back(uint(edges[i].size()));
            }
//            if(i<2)
            {
            //cout<<edges[i].size()<<endl;
            //txtOutput<<2*edges[i].size()<<endl;
            for(uint j=0;j<edges[i].size();j++)
            {
                //txtOutput<<halfedge[edges[i][j]].z<<" ";
                //txtOutput<<halfedge[edges[i][j]].w<<" ";
                //cout<<halfedge[edges[i][j]].z<<" ";
                singleedge.x=halfedge[edges[i][j]].z;
                singleedge.y=halfedge[edges[i][j]].w;
                faceset.push_back(singleedge);
            }
            //txtOutput<<endl;
            //cout<<endl;
            }
            endlayer=i;
        }
        //f.close();

//        fraction=0.3f;
//        emit progressReport(100*fraction,100.0f);
//        progressbar->setLabelText("prepare faceset data done!");
//        QApplication::processEvents();

        layerwidth=uint(linesnumber.size());
        createtime+=time2.elapsed();
        vector<uint>locationdata(uint(faceset.size()+layerwidth*LOOPs));
        vector<uint>loopcount(layerwidth);//每层的轮廓环个数
        vector<uint>loopnumber(layerwidth*LOOPs);//每层每个轮廓环的点数
        vector<cl_int4>hashTable(2*faceset.size(),{{-2,-2,-2,-2}});
        cout <<"edges : "<< faceset.size() << " memory: " << sizeof(uint)*faceset.size() / 1024/1024 << "MB" << endl;
        cout << "linesnumber : " <<linesnumber.size() << " memory: " << sizeof(uint)*linesnumber.size() / 1048576 << "M" << endl;
        cout << "hashTable : " <<hashTable.size() << " memory: " << sizeof(cl_int4)*hashTable.size() / 1048576 << "M" << endl;
        cout <<"locationdata : "<< locationdata.size() << " memory: " << sizeof(uint)*locationdata.size() / 1048576 << "M" << endl;
        cout <<"loopcount : "<< loopcount.size() << " memory: " << sizeof(uint)*loopcount.size() / 1048576 << "M" << endl;
        cout <<"loopnumber : "<< loopnumber.size() << " memory: " << sizeof(uint)*loopnumber.size() / 1048576 << "M" << endl;
        bool success=opencl.executeKernel(faceset,linesnumber,hashTable,layerwidth,locationdata,loopcount,loopnumber);
        //writeHash(hashTable);
        if(!success)
        {
            cout<<"failed to execute kernel on GPU"<<endl;
            fraction=1.0f;
            emit progressReport(100*fraction,100.0f);
            progressbar->setLabelText("error!");
            QApplication::processEvents();
            return false;
        }
        sorttime +=time.elapsed();
        fraction=0.8f;
        emit progressReport(100*fraction,100.0f);
        progressbar->setLabelText("sort edges done!");
        QApplication::processEvents();
        cout<<"sort edges done!"<<endl;
        time.restart();
        uint hashoffset=0,edgeoffset=0;
//        for(uint i=0;i<1;i++)
//        {
//            if(i==0)hashoffset=0;
//            else hashoffset=linesnumber[i-1];
//            cout<<"elements in hashTable of layer "<<i<<": "<<linesnumber[i]-hashoffset<<endl;
//            for(uint j=0;j<linesnumber[i]-hashoffset;j++)
//            {
//                if(hashTable[hashoffset+j].y>0)
//                    qDebug()<<hashTable[hashoffset+j].x*(linesnumber[i]-hashoffset)+hashTable[hashoffset+j].w<<hashTable[hashoffset+j].y<<hashTable[hashoffset+j].z;
//            }
//        }
//        cout<<"layerwidth: "<<layerwidth<<endl;
//        for(uint i=0;i<7;i++)
//        {
//            cout<<"count of the loops in the layer "<<i<<":";
//            for(uint k=0;k<loopcount[i];k++)
//            {
//                cout<<loopnumber[i*1000+k]<<" ";
//            }
//            cout<<endl;
//            if(i==0)edgeoffset=0;
//            else edgeoffset=linesnumber[i-1];
//            uint offset=0;
//            for(uint k=0;k<loopcount[i];k++)
//            {
//                if(k==0)offset=0;
//                else offset +=loopnumber[i*1000+k-1];
//                cout<<"    ";
//                for(uint t=0;t<loopnumber[i*1000+k];t++)
//                {
//                    cout<<locationdata[edgeoffset+offset+t]<<" ";
//                }
//                cout<<endl;
//            }
//        }
        vector<cl_int4>().swap(hashTable);
        vector<cl_uint2>().swap(faceset);
        for(uint i=startlayer;i<endlayer;i++)
        {
//            cout<<"count of the loops in the layer "<<i<<": "<<loopcount[i-startlayer]<<endl;
//            for(uint j=0;j<loopcount[i-startlayer];j++)
//            {
//                cout<<loopnumber[(i-startlayer)*LOOPs+j]<<" ";
//            }
//            cout<<endl;
            if(i==startlayer)edgeoffset=0;
            else edgeoffset=linesnumber[i-startlayer-1]+(i-startlayer)*LOOPs;
            polylines.clear();
            polylines.reserve(loopcount[i-startlayer]);
            uint offset=0,tmp=0;
            for(uint j=0;j<loopcount[i-startlayer];j++)
            {
                lines.clear();
                lines.reserve(loopnumber[(i-startlayer)*LOOPs+j]);
                if(j==0)offset=0;
                else offset +=loopnumber[(i-startlayer)*LOOPs+j-1];
                //cout<<"    ";
                //cout<<endl;
                if(locationdata[edgeoffset+offset]==locationdata[edgeoffset+offset+loopnumber[(i-startlayer)*LOOPs+j]-1] && loopnumber[(i-startlayer)*LOOPs+j]>2)
//                if(loopnumber[(i-startlayer)*LOOPs+j]>2)
                {
                    //cout<<"loopnumber: "<<loopnumber[(i-startlayer)*LOOPs+j]-1<<endl;
                    for(size_t k=0;k<loopnumber[(i-startlayer)*LOOPs+j]-1;k++)
                    {
                        tmp=locationdata[edgeoffset+offset+k];
                        v1=vertex[halfedge[edges[i][tmp]].x];
                        v2=vertex[halfedge[edges[i][tmp]].y];
                        float diffx=v1.x-v2.x;
                        float diffy=v1.y-v2.y;
                        float diffz=v1.z-v2.z;
                        float x=v1.x+diffx*(z[i]-v1.z)/diffz;
                        float y=v1.y+diffy*(z[i]-v1.z)/diffz;
                        //cout<<Point(x,y,z[i])<<endl;
                        lines.push_back(Point(x,y,z[i]));
                    }
                    polylines.push_back(lines);
                    vector<Point>().swap(lines);
                }
            }
            intrpoints[i]=polylines;
        }
        comptime +=time.elapsed();
        startlayer=endlayer;
    }
    cout<<"creat faceset time: "<<createtime<<"ms"<<endl;
    cout<<"intersect edges done!"<<endl;
    fraction=1.0f;
    emit progressReport(100*fraction,100.0f);
    progressbar->setLabelText("intersect edges done!");
    QApplication::processEvents();
    return true;
}

void Slice::hashInsert(vector<cl_int4>& hashTable,uint key,uint value,uint length)
{
    uint hashAddr = key % length;
    //cout<<"key:"<<key<<", value:"<<value<<", hashAddr:"<<hashAddr<<endl;
    for(uint i=0;i<length;i++)    //循环，最大哈希表长度
    {
        if(hashTable[hashAddr].x!=-1 && hashTable[hashAddr].x!=key)    //冲突
            hashAddr = (hashAddr+1) % length;   //开放定址法的线性探测法,查找下一个可存放数据的空间
        else
            break;
    }
    //cout<<"hashAddr: "<<hashAddr<<endl;
    if(hashTable[hashAddr].x==-1)
    {
        hashTable[hashAddr].x = key;
        hashTable[hashAddr].y = value;
    }
    else if(hashTable[hashAddr].x==key)
    {
        hashTable[hashAddr].z =value;
    }
}

int Slice::hashSearch(vector<cl_int4>hashTable,uint key,uint length)
{
    uint hashAddr = key % length;
    while(key!=uint(hashTable[hashAddr].x))
    {
        hashAddr = (hashAddr+1) % length;
        if(hashTable[hashAddr].x==-1 || hashAddr == key % length)  //如果探测到下一个地址为空，还没有找到，或者循环找了一遍又回到最开始的hashAddr
        {
            return -1;
        }
    }
    return hashAddr;
}

void Slice::writeHash(vector<cl_int4>hashTable)
{
    QFile f("C:/Users/xjtu_/Desktop/HashTable.txt");
    if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        cout << "Open failed." << endl;
    }

    QTextStream txtOutput(&f);
    for(uint i=0;i<hashTable.size();i++)
    {
        txtOutput <<hashTable[i].x<<" "<<hashTable[i].y<<" "<<hashTable[i].z<<" "<<hashTable[i].w << endl;
    }
    f.close();

}

void Slice::sliceOnCpu(vector<cl_float3> &vertex,vector<cl_uint4> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints)
{
    float zmin=surroundBox[4];
    float zmax=surroundBox[5];
    z.clear();
    zheight=zmin;
    layernumber=0;
    while(zheight<=zmax)
    {
        z.push_back(zheight);
        zheight += thick;
        layernumber++;
    }
    float fraction=0.05f;
    emit progressReport(100*fraction,100.0f);
    QApplication::processEvents();

    //将所有半边分组
    time.start();
    vector<vector<uint>>edges;
    edges.resize(layernumber);
    for(uint i=0;i<halfedge.size();i++)
    {
        float z1=qMin(vertex[halfedge[i].x].z,vertex[halfedge[i].y].z);
        float z2=qMax(vertex[halfedge[i].x].z,vertex[halfedge[i].y].z);
        int num1=int(ceil((z1-zmin)/thick));
        int num2=int((z2-zmin)/thick);
        if(qAbs(z2-z1)<1e-8f)num2=num1-1;
        //cout<<z1<<" "<<z2<<" "<<num1<<" "<<num2<<" "<<zmin<<" "<<thick<<endl;
        for(int j=num1;j<=num2;j++)
        {
            edges[uint(j)].push_back(i);
        }
//        fraction=0.05f+float(i+1)/halfedge.size()*0.05f;
//        emit progressReport(100*fraction,100.0f);
//        QApplication::processEvents();
    }
    while(edges.back().empty())
    {
        edges.pop_back();
    }
    layernumber=edges.size();
    findtime =time.elapsed();
    fraction=0.1f;
    emit progressReport(100*fraction,100.0f);
    progressbar->setLabelText("group edges done!");
    QApplication::processEvents();
    cout<<"group edges done!"<<endl;

    //同组中的半边进行重排序，组成首尾相连的轮廓
    time.restart();
    vector<vector<vector<uint>>> location;
    location.resize(layernumber);
    vector<vector<uint>>locs;
    locs.reserve(1000);
    vector<uint> loc;
    for(uint i=0;i<layernumber;i++)
    {
        //cout<<"The "<<i<<" layer:"<<edges[i].size()<<endl;
        locs.clear();
        if(edges[i].empty())
        {
            //location[i]=locs;
            continue;
        }
        //if(edges[i].size()>linesnumber)linesnumber=edges[i].size();
        loc.reserve(2*edges[i].size());
        uint length=2*uint(edges[i].size());
        vector<cl_int4> hashTable(length,{{-1,-1,-1,-1}});
        for(uint j=0;j<edges[i].size();j++)
        {
            hashInsert(hashTable,halfedge[edges[i][j]].z,j,length);
            hashInsert(hashTable,halfedge[edges[i][j]].w,j,length);
        }
//        if(i<2)
//        {
//            cout<<"elements in hashTable of layer "<<i<<": "<<length<<endl;
//            for(uint idx=0;idx!=length;++idx)
//            {
//                cout<<hashTable[idx].x<<": "<<hashTable[idx].y<<","<<hashTable[idx].z<<endl;
//            }
//            cout<<endl;
//        }
        int index=1,current=0,start=1,ret=-1;
        for(uint k=0;k<length;k++)
        {
            if(hashTable[k].y!=-1)
            {
                loc.clear();
                index = hashTable[k].y;
                start = hashTable[k].y;
                current= hashTable[k].x;
                loc.push_back(uint(index));
                for(uint iter=0;iter<length;iter++)
                {
                    ret=hashSearch(hashTable,uint(current),length);
                    if(ret == -1)
                    {
                        break;
                    }
                    if(hashTable[uint(ret)].y!=index)
                    {
                        index=hashTable[ret].y;
                    }
                    else
                    {
                        index=hashTable[ret].z;
                    }
                    if(start!=index && index>=0)
                    {
                        hashTable[ret].y=-1;
                        hashTable[ret].z=-1;
                        current=(halfedge[edges[i][index]].z!=current)?halfedge[edges[i][index]].z:halfedge[edges[i][index]].w;
                        loc.push_back(uint(index));
                    }
                    else if(start==index)
                    {
                        hashTable[uint(ret)].y=-1;
                        hashTable[uint(ret)].z=-1;
                        loc.push_back(uint(index));
                        break;
                    }
                }
                if(loc.front()==loc.back() && loc.size()>2)
                {
                    //cout<<loc.size()<<endl;
                    loc.pop_back();
                    locs.push_back(loc);
                }
            }
        }
        if(!locs.empty())location[i]=locs;
//        fraction=0.1f+float(i+1)/edges.size()*0.8f;
//        emit progressReport(100*fraction,100.0f);
//        QApplication::processEvents();
    }
    sorttime =time.elapsed();
    fraction=0.9f;
    emit progressReport(100*fraction,100.0f);
    progressbar->setLabelText("sort edges done!");
    QApplication::processEvents();
    cout<<"sort edges done!"<<endl;

    //计算每组中的半边和相应的z平面求交
    time.restart();
    Lines lines;
    Polylines polylines;
    intrpoints.clear();
    intrpoints.reserve(layernumber);
    cl_float3 v1,v2;
    for(uint i=0;i<layernumber;i++)
    {
        polylines.clear();
        polylines.reserve(location[i].size());
        //cout<<"The "<<i<<" layer:"<<edges[i].size()/2<<endl;
        for(size_t j=0;j<location[i].size();j++)
        {
            lines.clear();
            lines.reserve(location[i][j].size());
            //cout<<"loop "<<j<<":"<<endl;
            for(size_t k=0;k<location[i][j].size();k++)
            {
                v1=vertex[halfedge[edges[i][location[i][j][k]]].x];
                v2=vertex[halfedge[edges[i][location[i][j][k]]].y];
                float diffx=v1.x-v2.x;
                float diffy=v1.y-v2.y;
                float diffz=v1.z-v2.z;
                float x=v1.x+diffx*(z[i]-v1.z)/diffz;
                float y=v1.y+diffy*(z[i]-v1.z)/diffz;
                lines.push_back(Point(x,y,z[i]));
            }
            polylines.push_back(lines);
        }
        intrpoints.push_back(polylines);
//        fraction=0.9f+float(i+1)/layernumber*0.1f;
//        emit progressReport(100*fraction,100.0f);
//        QApplication::processEvents();
    }
    comptime +=time.elapsed();
    fraction=1.0f;
    emit progressReport(100*fraction,100.0f);
    progressbar->setLabelText("intersect edges done!");
    QApplication::processEvents();
    cout<<"intersect edges done!"<<endl;
}
