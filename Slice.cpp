﻿#include "Slice.h"
//#include <CGAL/Polygon_mesh_slicer.h>
using namespace std;
Slice::Slice()
{
    thick=10;
    layernumber=0;
    zheight=0.0f;
    isParaComp=true;
    lines.reserve(1000);
    findtime=0;
    comptime=0;
}

Slice::~Slice()
{

}

void Slice::startSlice(vector<EdgeNode> &halfedge,float zmin,float zmax,vector<Polylines> &intrpoints)
{
    findtime=0;
    comptime=0;
    sorttime=0;
    if(isParaComp)
    {
        sliceByGpu(halfedge,zmin,zmax,intrpoints);
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
        cout<<"gpu compute time:"<<comptime<<"ms"<<endl;
        //cout<<"time of parallel computing:"<<time.elapsed()<<"ms"<<endl;
    }
    else
    {
        //sliceByHeight(mesh,zmin,zmax);
        sliceByCpu(halfedge,zmin,zmax,intrpoints);
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
        cout<<"cpu compute time:"<<comptime<<"ms"<<endl;
        //cout<<"time of cpu computing:"<<time.elapsed()<<"ms"<<endl;
    }
}

void Slice::sliceByHeight(Mesh mesh,float zmin,float zmax,vector<Polylines> &intrpoints)
{
    layernumber=0;
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

void Slice::sliceByCpu(vector<EdgeNode> halfedge,float zmin,float zmax,vector<Polylines> &intrpoints)
{
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
    vector<vector<EdgeNode>>edges;
    edges.resize(layernumber+1);
    for(uint i=0;i<halfedge.size();i++)
    {
        float z1=qMin(halfedge[i].z1,halfedge[i].z2);
        float z2=qMax(halfedge[i].z1,halfedge[i].z2);
        int num1=int(ceil((z1-zmin)/thick));
        int num2=int((z2-zmin)/thick);
        if(qAbs(z2-z1)<1e-8f)num2=num1-1;
        //cout<<z1<<" "<<z2<<" "<<num1<<" "<<num2<<" "<<zmin<<" "<<thick<<endl;
        for(int j=num1;j<=num2;j++)
        {
            edges[j].push_back(halfedge[i]);
        }
    }
    findtime =time.elapsed();
    cout<<"group edges done!"<<endl;
    halfedge.clear();

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
                facesmap.insert(edges[i][j].f,j);
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
                        face.push_back(edges[i][index+1].f);
                    }
                    else
                    {
                        loc.push_back(index-1);
                        face.push_back(edges[i][index-1].f);
                    }
                    facesmap.remove(find_index.key());
                };
                if(loc.front()==loc.back())
                {
                    loc.pop_back();
                    //cout<<loc.size()<<endl;
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
    intrpoints.clear();
    intrpoints.reserve(layernumber+1);
    vector<Point>point;
    vector<vector<Point>>points;
    points.reserve(layernumber);
    for(uint i=0;i<layernumber;i++)
    {
        point.clear();
        point.reserve(edges[i].size());
        for(uint j=0;j<edges[i].size();j++)
        {
            float diffx=edges[i][j].x1-edges[i][j].x2;
            float diffy=edges[i][j].y1-edges[i][j].y2;
            float diffz=edges[i][j].z1-edges[i][j].z2;
            float x=edges[i][j].x1+diffx*(z[i]-edges[i][j].z1)/diffz;
            float y=edges[i][j].y1+diffy*(z[i]-edges[i][j].z1)/diffz;
            point.push_back(Point(x,y,z[i]));
            //cout<<p1<<" "<<p2<<endl;
        }
        points.push_back(point);
    }
    vector<vector<EdgeNode>>().swap(edges);
    for(uint i=0;i<layernumber;i++)
    {
        polylines.clear();
        //cout<<"The "<<i<<" layer:"<<edges[i].size()/2<<endl;
        for(size_t j=0;j<location[i].size();j++)
        {
            lines.clear();
            //cout<<"loop "<<j<<":"<<endl;
            for(size_t k=0;k<location[i][j].size();k++)
            {
                //cout<<k<<":"<<points[i][location[i][j][k]]<<endl;
                lines.push_back(points[i][location[i][j][k]]);
            }
            polylines.push_back(lines);
        }
        intrpoints.push_back(polylines);
    }
    comptime +=time.elapsed();
    vector<vector<Point>>().swap(points);
    vector<Lines>().swap(polylines);
    vector<Point>().swap(lines);
    cout<<"intersect edges done!"<<endl;
}

void Slice::sliceByGpu(vector<EdgeNode> &halfedge,float zmin,float zmax,vector<Polylines> &intrpoints)
{ 
    z.clear();
    zheight=zmin;
    layernumber=uint(ceil((zmax-zmin)/thick));
    //将所有半边分组
    time.start();
    vector<vector<EdgeNode>>edges;
    edges.resize(layernumber+1);
    {
        vector<int> buf;
        buf.resize(halfedge.size()*3,-1);
        cl::Buffer halfedgebuf(opencl.context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,halfedge.size()*sizeof(EdgeNode),halfedge.data());
        opencl.executeKernel(halfedgebuf,buf,zmin,thick,halfedge.size());
        for(size_t i=0;i<halfedge.size();i++)
        {
            //cout<<buf[3*i+0]<<" "<<buf[3*i+1]<<" "<<buf[3*i+2]<<endl;
            int num1=buf[3*i+0];
            int num2=buf[3*i+1];
            //cout<<z1<<" "<<z2<<" "<<num1<<" "<<num2<<endl;
            for(int j=num1;j<=num2;j++)
            {
                edges[j].push_back(halfedge[i]);
            }
        }
    }
    if(edges[layernumber].empty())edges.pop_back();
    layernumber=edges.size();
    for(uint i=0;i<layernumber;i++)
    {
        z.push_back(zheight);
        zheight += thick;
    }
    findtime +=time.elapsed();
    cout<<"group edges done!"<<endl;
    halfedge.clear();

    //同组中的半边进行重排序，组成首尾相连的轮廓
    time.restart();
    vector<vector<vector<uint>>> location;
    vector<uint>linesnumber;
    location.resize(layernumber);
    linesnumber.resize(layernumber);
    {
        vector<vector<uint>>locs;
        locs.reserve(10);
        vector<uint> face,loc;
        QMultiHash<uint,uint>facesmap;
        uint index=0;
        for(uint i=0;i<edges.size();i++)
        {
            //cout<<"The "<<i<<" layer:"<<edge[i].size()<<endl;
            locs.clear();
            facesmap.clear();
            if(edges[i].empty())
            {
                linesnumber[i]=uint(edges[i].size());
                continue;
            }
            if(i>0)
                linesnumber[i]=uint(edges[i].size())+linesnumber[i-1];
            else
                linesnumber[i]=uint(edges[i].size());
            facesmap.reserve(int(edges[i].size()));
            loc.reserve(edges[i].size());
            face.reserve(edges[i].size());
            for(uint j=0;j<edges[i].size();j++)
            {
                facesmap.insert(edges[i][j].f,j);
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
                        face.push_back(edges[i][index+1].f);
                    }
                    else
                    {
                        loc.push_back(index-1);
                        face.push_back(edges[i][index-1].f);
                    }
                    facesmap.remove(find_index.key());
                };
                if(loc.front()==loc.back())
                {
                    loc.pop_back();
                    //cout<<loc.size()<<endl;
                    locs.push_back(loc);
                }
            }
            location[i]=locs;
        }
    }
    sorttime =time.elapsed();
    cout<<"sort edges done!"<<endl;

    time.restart();
    size_t total=size_t(*max_element(linesnumber.begin(),linesnumber.end()));
    //cout<<layernumber<<" "<<total<<endl;
    cl::Buffer resultbuf(opencl.context,CL_MEM_WRITE_ONLY,total*3*sizeof(float));
    {
        vector<EdgeNode> edgeset;
        edgeset.reserve(total);
        for(uint i=0;i<edges.size();i++)
        {
            //cout<<"The "<<i<<" layer:"<<edges[i].size()<<" "<<num<<endl;
            for(uint j=0;j<edges[i].size();j++)
            {
                edgeset.push_back(edges[i][j]);
            }
        }
        cout<<"test1"<<endl;
        cl::Buffer edgebuf(opencl.context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,edgeset.size()*sizeof(EdgeNode),edgeset.data());
        opencl.executeKernel(edgebuf,resultbuf,total,layernumber,z.data(),linesnumber);
        cout<<"test2"<<endl;
    }
    vector<vector<EdgeNode>>().swap(edges);
    vector<float>result(total*3,0);
    opencl.queue.enqueueReadBuffer(resultbuf, CL_TRUE, 0,total*3* sizeof(float),&result[0], 0, NULL);
    cout<<"test3"<<endl;
    uint num=0;
    intrpoints.clear();
    intrpoints.resize(layernumber);
    for(uint i=0;i<layernumber;i++)
    {
        polylines.clear();
        //cout<<"The "<<i<<" layer:"<<edge[i].size()/2<<" "<<location[i].size()<<endl;
        for(size_t j=0;j<location[i].size();j++)
        {
            lines.clear();
            for(size_t k=0;k<location[i][j].size();k++)
            {
                //cout<<location[i][j][k]<<" "<<3*num+3*location[i][j][k]+0<<" "<<3*num+3*location[i][j][k]+1<<endl;
                float x=result[3*num+3*location[i][j][k]+0];
                float y=result[3*num+3*location[i][j][k]+1];
                //cout<<Point(x,y,z[i])<<endl;
                lines.push_back(Point(x,y,z[i]));
            }
            polylines.push_back(lines);
        }
        if(i==0)
            num=linesnumber[i];
        else
            num +=linesnumber[i]-linesnumber[i-1];
        intrpoints[i]=polylines;
    }
    vector<Lines>().swap(polylines);
    vector<Point>().swap(lines);
    comptime +=time.elapsed();
}
