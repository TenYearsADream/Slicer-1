﻿#include "Slice.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
//#include <CGAL/Polygon_mesh_slicer.h>
using namespace std;
Slice::Slice()
{
    thick=10;
    layernumber=0;
    zheight=0.0f;
    isParaComp=true;
    findtime=0;
    comptime=0;
}

Slice::~Slice()
{

}

void Slice::startSlice(vector<cl_float3> &vertex,vector<cl_uint3> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints)
{
    findtime=0;
    comptime=0;
    sorttime=0;
    if(isParaComp)
    {
        //sliceByGpu(vertex,halfedge,surroundBox,intrpoints);
        sliceOnGpu(vertex,halfedge,surroundBox,intrpoints);
        if(genSlicesFile(slicepath[1],intrpoints,surroundBox))
        {
            cout<<"slc file generated successfully."<<endl;
        }
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
        cout<<"gpu compute time:"<<comptime<<"ms"<<endl;
        cout<<"time of parallel computing:"<<findtime+sorttime+comptime<<"ms"<<endl;
    }
    else
    {
        //sliceByHeight(mesh,zmin,zmax);
//        sliceByCpu(vertex,halfedge,surroundBox,intrpoints);
        sliceOnCpu(vertex,halfedge,surroundBox,intrpoints);
        if(genSlicesFile(slicepath[0],intrpoints,surroundBox))
        {
            cout<<"slc file generated successfully."<<endl;
        }
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
        cout<<"cpu compute time:"<<comptime<<"ms"<<endl;
        cout<<"time of cpu computing:"<<findtime+sorttime+comptime<<"ms"<<endl;
    }
}

void Slice::sliceByHeight(Mesh mesh,float zmin,float zmax,vector<Polylines> &intrpoints)
{
    layernumber=0;
    Polylines polylines;
    //CGAL::Polygon_mesh_slicer<Mesh, Kernel> slicer(mesh);
    zheight=zmin;
    while(zheight<=zmax)
    {
//        progressDlg->setValue(zheight);
//        if(progressDlg->wasCanceled())
//        {
//            layernumber=1;
//            intrpoints.clear();
//            QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("取消切片"));
//            return;
//        }
        //cout<<"layer of "<<layernumber<<":"<<endl;
        polylines.clear();
        time.start();
        //slicer(Kernel::Plane_3(0, 0, 1, double(-zheight)),back_inserter(polylines));
        comptime +=time.elapsed();
        layernumber++;
        intrpoints.push_back(polylines);
        zheight += thick;
    }
}

void Slice::sliceByCpu(vector<cl_float3> &vertex,vector<cl_uint3> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints)
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
    }
    findtime =time.elapsed();
    cout<<"group edges done!"<<endl;

    //同组中的半边进行重排序，组成首尾相连的轮廓
    time.restart();
    size_t linesnumber=0;
    vector<vector<vector<uint>>> location;
    location.resize(layernumber+1);
    {
        QMultiHash<uint,uint>facesmap;
        vector<vector<uint>>locs;
        locs.reserve(100);
        vector<uint> face,loc;
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
            facesmap.reserve(int(edges[i].size()));
            loc.reserve(edges[i].size());
            face.reserve(edges[i].size());
            for(uint j=0;j<edges[i].size();j++)
            {
                facesmap.insert(halfedge[edges[i][j]].z,j);
            }
            while(!facesmap.empty())
            {
                face.clear();
                loc.clear();
                face.push_back(facesmap.constBegin().key());
                loc.push_back(facesmap.value(face.back()));
                while(1)
                {
                    auto find_index = facesmap.find(face.back());
                    if(find_index==facesmap.end())
                    {
                        break;
                    }
                    if(find_index.value()==loc.back())find_index++;
                    index=find_index.value();
                    if((index & 1) == 0)
                    {
                        loc.push_back(index+1);
                        face.push_back(halfedge[edges[i][index+1]].z);
                    }
                    else
                    {
                        loc.push_back(index-1);
                        face.push_back(halfedge[edges[i][index-1]].z);
                    }
                    facesmap.remove(find_index.key());
                };
                if(loc.front()==loc.back())
                {
                    //cout<<loc.size()<<endl;
                    loc.pop_back();
                    locs.push_back(loc);
                }
            }
            //cout<<"The "<<i<<" layer:"<<locs.size()<<endl;
            location[i]=locs;
        }
    }
    sorttime =time.elapsed();
    cout<<"sort edges done!"<<endl;

    //计算每组中的半边和相应的z平面求交
    time.restart();
    Lines lines;
    Polylines polylines;
    intrpoints.clear();
    intrpoints.reserve(layernumber+1);
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
    }
    comptime +=time.elapsed();
    cout<<"intersect edges done!"<<endl;
}

void Slice::sliceByGpu(vector<cl_float3> &vertex,vector<cl_uint3> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints)
{ 
    z.clear();
    float zmin=surroundBox[4];
    float zmax=surroundBox[5];
    zheight=zmin;
    layernumber=uint(ceil((zmax-zmin)/thick));
    //将所有半边分组
    time.start();
    size_t total=0;
    vector<vector<uint>>edges;
    edges.resize(layernumber+1);
    for(uint i=0;i<layernumber+1;i++)edges[i].reserve(1000);
    cl::Buffer vertexbuf(opencl.context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,vertex.size()*sizeof(cl_float3),vertex.data());
    cl::Buffer halfedgebuf(opencl.context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,halfedge.size()*sizeof(cl_uint3),halfedge.data());
    {
        vector<int> buf;
        buf.resize(halfedge.size()*3,-1);
        opencl.executeKernel(vertexbuf,halfedgebuf,buf,zmin,thick,halfedge.size());
        for(uint i=0;i<halfedge.size();i++)
        {
            //cout<<buf[3*i+0]<<" "<<buf[3*i+1]<<" "<<buf[3*i+2]<<endl;
            int num1=buf[3*i+0];
            int num2=buf[3*i+1];
            //cout<<z1<<" "<<z2<<" "<<num1<<" "<<num2<<endl;    
            for(int j=num1;j<=num2;j++)
            {
                edges[size_t(j)].push_back(i);
                total++;
            }
        }
    }
    if(edges[layernumber].empty())edges.pop_back();
    layernumber=edges.size();
    z.reserve(layernumber);
    for(uint i=0;i<layernumber;i++)
    {
        z.push_back(zheight);
        zheight += thick;
    }
    findtime +=time.elapsed();
//    cout<<"total: "<<total<<endl;
    cout<<"group edges done!"<<endl;

        time.restart();
        //同组中的半边进行重排序，组成首尾相连的轮廓
    //    QFile f("C:/Users/xjtu_/Desktop/test.txt");
    //    if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
    //    {
    //        cout << "Open failed." << endl;
    //    }

       // QTextStream txtOutput(&f);
        vector<cl_float3>result(total);
        vector<uint> faceset;
        vector<uint>linesnumber;
        linesnumber.resize(layernumber);
        faceset.reserve(total);
        vector<uint>edgeset;
        edgeset.reserve(total);
        for(uint i=0;i<layernumber;i++)
        {
            //cout<<"The "<<i<<" layer:"<<edges[i].size()<<endl;
            //cout<<edges[i].size()<<endl;
            //txtOutput<<edges[i].size()<<endl;
            if(edges[i].empty())
            {
                linesnumber[i]=uint(edges[i].size());
                continue;
            }
            if(i>0)
                linesnumber[i]=uint(edges[i].size())+linesnumber[i-1];
            else
                linesnumber[i]=uint(edges[i].size());
            for(uint j=0;j<edges[i].size();j++)
            {
                //cout<<halfedge[edges[i][j]].z<<" ";
                //txtOutput<<halfedge[edges[i][j]].z<<" ";
                faceset.push_back(halfedge[edges[i][j]].z);
                edgeset.push_back(edges[i][j]);
            }
            //cout<<endl;
            //txtOutput<<endl;
        }
        //f.close();
        opencl.executeKernel(vertexbuf,halfedgebuf,edgeset,result,total,layernumber,z.data(),linesnumber);
        //    for(uint i=0;i<10;i++)
        //    {
        //        cout<<result[i].x<<" "<<result[i].y<<" "<<result[i].z<<endl;
        //    }
        comptime +=time.elapsed();
        cout<<"intersect edges done!"<<endl;

        time.restart();
        Lines lines;
        Polylines polylines;
        uint num=0;
        intrpoints.clear();
        intrpoints.resize(layernumber);
        vector<uint> loc;
        vector<uint>locationdata(faceset.size());
        vector<uint>loopcount(layernumber);//每层的轮廓环个数
        vector<uint>loopnumber(layernumber*100);//每层每个轮廓环的点数
        vector<cl_int3>hashTable(faceset.size()/2,{{-1,-1,-1}});
        cout <<"total number of edges in all layers: "<< faceset.size() << " memory: " << sizeof(uint)*faceset.size() / 1048576 << "M" << endl;
        cout << "total number of hashTable in all layers: " <<hashTable.size() << " memory: " << sizeof(cl_int3)*hashTable.size() / 1048576 << "M" << endl;
        opencl.executeKernel(faceset,linesnumber,hashTable,uint(layernumber),locationdata,loopcount,loopnumber);
        sorttime =time.elapsed();
        uint tmp=0;
        for(uint i=0;i<layernumber;i++)
        {
//            cout<<"count of the loops in the layer "<<i<<":";
//            for(uint j=0;j<loopcount[i];j++)
//            {
//                cout<<loopnumber[i*100+j]<<" ";
//            }
//            cout<<endl;
            polylines.clear();
            polylines.reserve(loopcount[i]);
            if(i==0)tmp=0;
            else tmp=linesnumber[i-1];
            uint offset=0;
            for(uint k=0;k<loopcount[i];k++)
            {
                loc.clear();
                loc.reserve(loopnumber[i*100+k]);
                lines.clear();
                lines.reserve(loopnumber[i*100+k]);
                if(k==0)offset=0;
                else offset +=loopnumber[i*100+k-1];
                //cout<<"    ";
                for(uint t=0;t<loopnumber[i*100+k];t++)
                {
                    //cout<<locationdata[tmp+offset+t]<<" ";
                    loc.push_back(locationdata[tmp+offset+t]);
                }
                //cout<<endl;
                if(loc.front()==loc.back())
                {
                    //cout<<loc.size()<<endl;
                    loc.pop_back();
                    for(size_t k=0;k<loc.size();k++)
                    {
                        //cout<<location[i][j][k]<<" "<<3*num+3*location[i][j][k]+0<<" "<<3*num+3*location[i][j][k]+1<<endl;
                        float x=result[num+loc[k]].x;
                        float y=result[num+loc[k]].y;
                        //cout<<Point(x,y,z[i])<<endl;
                        lines.push_back(Point(x,y,z[i]));
                    }
                    polylines.push_back(lines);
                }
            }
            if(i==0)
                num=linesnumber[i];
            else
                num +=linesnumber[i]-linesnumber[i-1];
            intrpoints[i]=polylines;
        }

    cout<<"sort edges done!"<<endl;

}

bool Slice::genSlicesFile(const QString& fileName,const vector<Polylines> intrpoints,float surroundBox[6])
{
    char g_arrFileBuff[1*1024*1024];
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
    setvbuf(stream , g_arrFileBuff, _IOFBF , sizeof(g_arrFileBuff));

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
    qDebug()<<"genSlicesFile time:"<< startTime.msecsTo(QTime::currentTime())<<" ms";
    return true;
}

void Slice::sliceOnGpu(vector<cl_float3> &vertex,vector<cl_uint3> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints)
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
    }
    while(edges.back().empty())
    {
        edges.pop_back();
    }
    layernumber=edges.size();
    findtime =time.elapsed();
//    cout<<"total: "<<total<<endl;
    cout<<"group edges done!"<<endl;

    QTime time2;
    time.restart();
    //同组中的半边进行重排序，组成首尾相连的轮廓
    vector<uint> faceset;
    vector<uint>linesnumber;
    linesnumber.resize(layernumber);
    faceset.reserve(total);
    const uint LOOPs=1000;
    time2.start();
    for(uint i=0;i<layernumber;i++)
    {
        //cout<<"The "<<i<<" layer:"<<edges[i].size()<<endl;
        //cout<<edges[i].size()<<endl;
        if(i>0)
            linesnumber[i]=uint(edges[i].size())+linesnumber[i-1];
        else
            linesnumber[i]=uint(edges[i].size());
        for(uint j=0;j<edges[i].size();j++)
        {
            //cout<<halfedge[edges[i][j]].z<<" ";
            faceset.push_back(halfedge[edges[i][j]].z);
        }
        //cout<<endl;
    }
    cout<<"creat faceset time: "<<time2.elapsed()<<endl;
    vector<uint>locationdata(uint(faceset.size()*1.2),0);
    vector<uint>loopcount(layernumber);//每层的轮廓环个数
    vector<uint>loopnumber(layernumber*LOOPs);//每层每个轮廓环的点数
    vector<cl_int3>hashTable(faceset.size()/2,{{-1,-1,-1}});
    cout <<"edges : "<< faceset.size() << " memory: " << sizeof(uint)*faceset.size() / 1024/1024 << "MB" << endl;
    cout << "linesnumber : " <<linesnumber.size() << " memory: " << sizeof(uint)*linesnumber.size() / 1048576 << "M" << endl;
    cout << "hashTable : " <<hashTable.size() << " memory: " << sizeof(cl_int3)*hashTable.size() / 1048576 << "M" << endl;
    cout <<"locationdata : "<< locationdata.size() << " memory: " << sizeof(uint)*locationdata.size() / 1048576 << "M" << endl;
    cout <<"loopcount : "<< loopcount.size() << " memory: " << sizeof(uint)*loopcount.size() / 1048576 << "M" << endl;
    cout <<"loopnumber : "<< loopnumber.size() << " memory: " << sizeof(uint)*loopnumber.size() / 1048576 << "M" << endl;
    opencl.executeKernel(faceset,linesnumber,hashTable,uint(layernumber),locationdata,loopcount,loopnumber);
    sorttime =time.elapsed();
    vector<cl_int3>().swap(hashTable);
    vector<uint>().swap(faceset);
    cout<<"sort edges done!"<<endl;

    //计算每组中的半边和相应的z平面求交
    time.restart();
    Lines lines;
    Polylines polylines;
    intrpoints.clear();
    intrpoints.resize(layernumber);
    uint tmp=0;
    cl_float3 v1,v2;
    for(uint i=0;i<layernumber;i++)
    {
//        cout<<"count of the loops in the layer "<<i<<":";
//        for(uint j=0;j<loopcount[i];j++)
//        {
//            cout<<loopnumber[i*100+j]<<" ";
//        }
//        cout<<endl;
        polylines.clear();
        polylines.reserve(loopcount[i]);
        if(i==0)tmp=0;
        else tmp=linesnumber[i-1];
        uint offset=0;
        for(uint j=0;j<loopcount[i];j++)
        {
            lines.clear();
            lines.reserve(loopnumber[i*LOOPs+j]);
            if(j==0)offset=0;
            else offset +=loopnumber[i*LOOPs+j-1];
            //cout<<"    ";
            //cout<<endl;
            if(locationdata[tmp+offset]==locationdata[tmp+offset+loopnumber[i*LOOPs+j]-1] && loopnumber[i*LOOPs+j]>2)
            {
                for(size_t k=0;k<loopnumber[i*LOOPs+j]-1;k++)
                {
                    v1=vertex[halfedge[edges[i][locationdata[tmp+offset+k]]].x];
                    v2=vertex[halfedge[edges[i][locationdata[tmp+offset+k]]].y];
                    float diffx=v1.x-v2.x;
                    float diffy=v1.y-v2.y;
                    float diffz=v1.z-v2.z;
                    float x=v1.x+diffx*(z[i]-v1.z)/diffz;
                    float y=v1.y+diffy*(z[i]-v1.z)/diffz;
                    //cout<<Point(x,y,z[i])<<endl;
                    lines.push_back(Point(x,y,z[i]));
                }
                polylines.push_back(lines);
            }
        }
        intrpoints[i]=polylines;
    }
    comptime +=time.elapsed();
    cout<<"intersect edges done!"<<endl;
}

void Slice::sliceOnCpu(vector<cl_float3> &vertex,vector<cl_uint3> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints)
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
    }
    while(edges.back().empty())
    {
        edges.pop_back();
    }
    layernumber=edges.size();
    findtime =time.elapsed();
    cout<<"group edges done!"<<endl;

    //同组中的半边进行重排序，组成首尾相连的轮廓
    time.restart();
    vector<vector<vector<uint>>> location;
    location.resize(layernumber);
    {
        vector<vector<uint>>locs;
        locs.reserve(100);
        vector<uint> face,loc;
        for(uint i=0;i<layernumber;i++)
        {
            //cout<<"The "<<i<<" layer:"<<edges[i].size()<<endl;
            locs.clear();
            if(edges[i].empty())
            {
                location[i]=locs;
                continue;
            }
//            if(edges[i].size()>linesnumber)linesnumber=edges[i].size();
            loc.reserve(edges[i].size());
            uint length=uint(edges[i].size()/2);
            vector<cl_int3> hashTable(length,{{-1,-1,-1}});
            for(uint j=0;j<edges[i].size();j++)
            {
                hashInsert(hashTable,halfedge[edges[i][j]].z,j,length);
            }

//            cout<<"elements in hashtable: "<<endl;
//            for(uint idx=0;idx!=length;++idx)
//            {
//                cout<<hashTable[idx].x<<": "<<hashTable[idx].y<<","<<hashTable[idx].z<<endl;
//            }
//            cout<<endl;

            int index=1,current=0,start=1;
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
                        int ret=hashSearch(hashTable,uint(current),length);
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
                        if((index & 1) == 0)
                        {
                            index +=1;
                        }
                        else{
                            index -=1;
                        }
                        if(start!=index && index>=0)
                        {
                            hashTable[ret].y=-1;
                            hashTable[ret].z=-1;
                            current=halfedge[edges[i][index]].z;
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
                    if(loc.front()==loc.back())
                    {
                        //cout<<loc.size()<<endl;
                        loc.pop_back();
                        locs.push_back(loc);
                    }
                }
            }
            location[i]=locs;
        }
    }
    sorttime =time.elapsed();
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
    }
    comptime +=time.elapsed();
    cout<<"intersect edges done!"<<endl;
}

void Slice::hashInsert(vector<cl_int3>& hashTable,uint key,uint value,uint length)
{
    uint hashAddr = key % length;
    //cout<<"key:"<<key<<", value:"<<value<<", hashAddr:"<<hashAddr<<endl;
    for(uint i=0;i<length;i++)    //循环，最大哈希表长度
    {
        if(hashTable[hashAddr].x!=-1 && hashTable[hashAddr].x!=int(key))    //冲突
            hashAddr = (hashAddr+1) % length;   //开放定址法的线性探测法,查找下一个可存放数据的空间
        else
            break;
    }
    //cout<<"hashAddr: "<<hashAddr<<endl;
    if(hashTable[hashAddr].x==-1)
    {
        hashTable[hashAddr].x = int(key);
        hashTable[hashAddr].y = int(value);
    }
    else if(hashTable[hashAddr].x==int(key))
    {
        hashTable[hashAddr].z = int(value);
    }
}

int Slice::hashSearch(vector<cl_int3>hashTable,uint key,uint length)
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
    return int(hashAddr);
}
