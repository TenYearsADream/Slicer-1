#include "Slice.h"
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
    lines.reserve(1000);
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
        sliceByGpu(vertex,halfedge,surroundBox,intrpoints);
        QString fileName="C:/Users/Administrator/Desktop/gpuSlice.slc";
        if(genSlicesFile(fileName,intrpoints,surroundBox))
        {
            cout<<"slc file generated successfully."<<endl;
        }
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
        cout<<"gpu compute time:"<<comptime<<"ms"<<endl;
        //cout<<"time of parallel computing:"<<time.elapsed()<<"ms"<<endl;
    }
    else
    {
        //sliceByHeight(mesh,zmin,zmax);
        //sliceOnGpu(vertex,halfedge,surroundBox,intrpoints);
        sliceByCpu(vertex,halfedge,surroundBox,intrpoints);
        QString fileName="C:/Users/Administrator/Desktop/cpuSlice.slc";
        if(genSlicesFile(fileName,intrpoints,surroundBox))
        {
            cout<<"slc file generated successfully."<<endl;
        }
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
    cl_float3 v1,v2;
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
    vector<Lines>().swap(polylines);
    vector<Point>().swap(lines);
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
    vector<vector<uint>>edges;
    edges.resize(layernumber+1);
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
    cl_int err;
    cl::Buffer resultbuf(opencl.context,CL_MEM_WRITE_ONLY,total*sizeof(cl_float3),0,&err);
    {
        cl::Buffer edgebuf(opencl.context,CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,total*sizeof(uint),0,&err);
        uint *edgeset=(uint*)opencl.queue.enqueueMapBuffer(edgebuf,CL_TRUE,CL_MAP_WRITE,0,total *sizeof(uint),0,0,&err);
        cout<<"test1"<<endl;
        for(uint j=0;j<edges[0].size();j++)
        {
            edgeset[j]=edges[0][j];
        }
        size_t num=edges[0].size();
        for(uint i=1;i<edges.size();i++)
        {
            for(uint j=0;j<edges[i].size();j++)
            {
                edgeset[num+j]=edges[i][j];
            }
            num+=edges[i].size();
        }
        opencl.queue.enqueueUnmapMemObject(edgebuf,edgeset);
        opencl.executeKernel(vertexbuf,halfedgebuf,edgebuf,resultbuf,total,layernumber,z.data(),linesnumber);
        cout<<"test2"<<endl;
    }
    cl_float3 *result=(cl_float3*)opencl.queue.enqueueMapBuffer(resultbuf,CL_TRUE,CL_MEM_WRITE_ONLY,0,total*sizeof(cl_float3),0,0,&err);
    if(err<0)
    {
        cout<<"fail to map reuslt "<<err<<endl;
    }
    for(uint i=0;i<10;i++)
    {
        cout<<result[i].x<<" "<<result[i].y<<" "<<result[i].z<<endl;
    }
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
                float x=result[num+location[i][j][k]].x;
                float y=result[num+location[i][j][k]].y;
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
    z.clear();
    float zmin=surroundBox[4];
    float zmax=surroundBox[5];
    time.start();
    zheight=zmin;
    layernumber=0;
    cl::Buffer vertexbuf(opencl.context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,vertex.size()*sizeof(cl_float3),vertex.data());
    cl::Buffer halfedgebuf(opencl.context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,halfedge.size()*sizeof(cl_uint3),halfedge.data());
    cl::Buffer resultbuf(opencl.context,CL_MEM_WRITE_ONLY,halfedge.size()*sizeof(cl_float4),NULL,NULL);
    while(zheight<=zmin)
    {
        opencl.intersect.setArg(0,vertexbuf);
        opencl.intersect.setArg(1,halfedgebuf);
        opencl.intersect.setArg(2,sizeof(float),&zheight);
        opencl.intersect.setArg(3,resultbuf);
        cl::NDRange globalSize(halfedge.size());
        opencl.queue.enqueueNDRangeKernel(opencl.intersect,cl::NullRange,globalSize,cl::NullRange,NULL,NULL);
        opencl.queue.finish();
        cl_float4*result=(cl_float4*)opencl.queue.enqueueMapBuffer(resultbuf,CL_TRUE,CL_MEM_WRITE_ONLY,0,halfedge.size()*sizeof(cl_float4));
        for(uint i=0;i<halfedge.size();i++)
        {
            cout<<result[i].x<<" "<<result[i].y<<" "<<result[i].z<<" "<<result[i].w<<endl;
        }
        zheight += thick;
        layernumber++;
    }
    findtime +=time.elapsed();
    cout<<"group edges done!"<<endl;
}
