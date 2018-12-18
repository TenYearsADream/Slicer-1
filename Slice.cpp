#include "Slice.h"
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

void Slice::startSlice(Mesh mesh,vector<float> halfedge,float zmin,float zmax)
{
    findtime=0;
    comptime=0;
    sorttime=0;
    intrpoints.clear();
    //sliceByEdge(mesh,zmin,zmax);
    if(isParaComp)
    {
        sliceByGpu(halfedge,zmin,zmax);
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
        cout<<"gpu compute time:"<<comptime<<"ms"<<endl;
        //cout<<"time of parallel computing:"<<time.elapsed()<<"ms"<<endl;
    }
    else
    {
        //sliceByHeight(mesh,zmin,zmax);
        sliceByCpu(halfedge,zmin,zmax);
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
        cout<<"cpu compute time:"<<comptime<<"ms"<<endl;
        //cout<<"time of cpu computing:"<<time.elapsed()<<"ms"<<endl;
    }
}

void Slice::sliceByHeight(Mesh mesh,float zmin,float zmax)
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

void Slice::sliceByEdge(Mesh mesh,float zmin,float zmax)
{
    vector<vector<Mesh::Edge_index>> edges;
    z.clear();
    edges.clear();
    zheight=zmin;
    layernumber=0;
    while(zheight<=zmax)
    {
        z.push_back(zheight);
        zheight += thick;
        layernumber++;
    }
    edges.resize(layernumber);
//    for(vector<float>::const_iterator it=z.begin();it!=z.end();it++)
//    {
//        cout<<(*it)<<" "<<endl;
//    }
    time.start();
    for(Mesh::Edge_index ei:mesh.edges())
    {
        //cout<<ei<<endl;
        Point p1=mesh.point(mesh.vertex(ei,0));
        Point p2=mesh.point(mesh.vertex(ei,1));
        double z1=qMin(p1.z(),p2.z());
        double z2=qMax(p1.z(),p2.z());
        vector<float>::iterator location_index=find_if(z.begin(),z.end(),bind2nd(greater<double>(),z1));
        size_t num1=size_t(location_index-z.begin());
        location_index=find_if(z.begin(),z.end(),bind2nd(greater<double>(),z2));
        size_t num2=size_t(location_index-z.begin())-1;
        for(size_t i=num1;i<=num2;i++)
        {
            edges[i].push_back(ei);
        }
    }
    findtime =time.elapsed();
    time.restart();
    vector<size_t> size;
    size.reserve(edges.size());
    for(size_t i=0;i<edges.size();i++)
    {
        //cout<<"The "<<i<<" layer:"<<edges[i].size()<<endl;
        if(edges[i].empty())continue;
        size.push_back(edges[i].size());
//        for(vector<Mesh::Edge_index>::const_iterator it=(edges[i]).begin();it!=(edges[i]).end();it++)
//        {
//            Mesh::halfedge_index h1=mesh.halfedge(*it);
//            Mesh::halfedge_index h2=mesh.opposite(h1);
//            Mesh::face_index f1=mesh.face(h1);
//            Mesh::face_index f2=mesh.face(h2);
//            cout<<(*it)<<" "<<" "<<f1<<" "<<f2<<endl;
//        }
//        cout<<endl;

        vector<Mesh::Edge_index> edge(edges[i]);
        vector<Mesh::Edge_index> lines;
        lines.push_back(edge.front());
        edge.erase(edge.begin());
        while(lines.size()<edges[i].size())
        {
            for(vector<Mesh::Edge_index>::const_iterator it=edge.begin();it!=edge.end();it++)
            {
                //cout<<lines.back()<<endl;
                Mesh::halfedge_index h1=mesh.halfedge(lines.back());
                Mesh::halfedge_index h2=mesh.opposite(h1);
                Mesh::face_index f1=mesh.face(h1);
                Mesh::face_index f2=mesh.face(h2);

                Mesh::halfedge_index h3=mesh.halfedge(*it);
                Mesh::halfedge_index h4=mesh.opposite(mesh.halfedge(*it));
                Mesh::face_index f3=mesh.face(h3);
                Mesh::face_index f4=mesh.face(h4);
                //cout<<f1<<" "<<f2<<" "<<f3<<" "<<f4<<endl;
                if(f1 == f3 || f1 == f4 || f2==f3 || f2==f4)
                {
                    //cout<<*it<<endl;
                    lines.push_back(*it);
                    edge.erase(it);
                    break;
                }
            }
        }
        edges[i]=lines;
//        for(vector<Mesh::Edge_index>::const_iterator it=lines.begin();it!=lines.end();it++)
//        {
//            cout<<(*it)<<endl;
//        }
    }
    sorttime =time.elapsed();

    time.restart();
    size_t linesnumber=*max_element(size.begin(),size.end());
    float *interSection1,*interSection2,*result;
    interSection1 =new float[layernumber*linesnumber *3];
    interSection2 =new float[layernumber*linesnumber *3];
    result =new float[layernumber*linesnumber *3];

    for(size_t i=0;i<layernumber;i++)
    {
        for(size_t j=0;j<edges[i].size();j++)
        {
            Point p1=mesh.point(mesh.vertex(edges[i][j],0));
            Point p2=mesh.point(mesh.vertex(edges[i][j],1));
            interSection1[i*3*linesnumber+3*j+0]=float(p1.x());
            interSection1[i*3*linesnumber+3*j+1]=float(p1.y());
            interSection1[i*3*linesnumber+3*j+2]=float(p1.z());
            interSection2[i*3*linesnumber+3*j+0]=float(p2.x());
            interSection2[i*3*linesnumber+3*j+1]=float(p2.y());
            interSection2[i*3*linesnumber+3*j+2]=float(p2.z());
        }
    }
    opencl.executeKernel(interSection1,interSection2,result,layernumber,linesnumber,z.data());

    for(size_t i=0;i<layernumber;i++)
    {
        polylines.clear();
        //cout<<"The "<<i<<" layer:"<<edges[i].size()<<endl;
        lines.clear();
        for(size_t j=0;j<edges[i].size();j++)
        {
            float x=result[i*3*linesnumber+3*j+0];
            float y=result[i*3*linesnumber+3*j+1];
            lines.push_back(Point(x,y,z[i]));
            //cout<<x<<" "<<y<<" "<<z[i]<<endl;
        }
        polylines.push_back(lines);
        intrpoints.push_back(polylines);
    }

    free(interSection1);
    free(interSection2);
    free(result);
    comptime +=time.elapsed();
}

void Slice::sliceByCpu(vector<float> halfedge,float zmin,float zmax)
{
    intrpoints.clear();
    vector<vector<pair<Point,Point>>> edges;
    vector<vector<uint>> faces;
    QMultiHash<uint,uint>facesmap;
    vector<vector<vector<uint>>> location;
    z.clear();
    zheight=zmin;
    layernumber=0;
    while(zheight<=zmax)
    {
        z.push_back(zheight);
        zheight += thick;
        layernumber++;
    }
    edges.resize(layernumber+1);
    faces.resize(layernumber+1);
    location.resize(layernumber+1);
    time.start();
    size_t edgenumber=halfedge.size()/7;
    for(uint i=0;i<edgenumber;i++)
    {
        Point p1=Point(halfedge[7*i+0],halfedge[7*i+1],halfedge[7*i+2]);
        Point p2=Point(halfedge[7*i+3],halfedge[7*i+4],halfedge[7*i+5]);
        double z1=qMin(p1.z(),p2.z());
        double z2=qMax(p1.z(),p2.z());
        int num1=int(ceil((float(z1)-zmin)/thick));
        int num2=int((float(z2)-zmin)/thick);
        if(qAbs(z2-z1)<1e-8)num2=num1-1;
        //cout<<z1<<" "<<z2<<" "<<num1<<" "<<num2<<endl;
        for(int j=num1;j<=num2;j++)
        {
            edges[size_t(j)].push_back(make_pair(p1,p2));
            faces[size_t(j)].push_back(uint(halfedge[7*i+6]));
        }
    }
    findtime =time.elapsed();

    time.restart();
    size_t linesnumber=0;
    vector<vector<uint>>locs;
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
        for(uint j=0;j<faces[i].size();j++)
        {
            facesmap.insert(faces[i][j],j);
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
                    face.push_back(faces[i][index+1]);
                }
                else
                {
                    loc.push_back(index-1);
                    face.push_back(faces[i][index-1]);
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
    sorttime =time.elapsed();

    time.restart();
    vector<Point>point;
    vector<vector<Point>>points;
    points.reserve(layernumber);
    for(uint i=0;i<layernumber;i++)
    {
        point.clear();
        for(uint j=0;j<edges[i].size();j++)
        {
            Point p1=edges[i][j].first;
            Point p2=edges[i][j].second;
            float diffx=float(p1.x()-p2.x());
            float diffy=float(p1.y()-p2.y());
            float diffz=float(p1.z()-p2.z());
            float x=float(p1.x())+diffx*(z[i]-float(p1.z()))/diffz;
            float y=float(p1.y())+diffy*(z[i]-float(p1.z()))/diffz;
            point.push_back(Point(x,y,z[i]));
            //cout<<p1<<" "<<p2<<endl;
        }
        points.push_back(point);
    }
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
}

void Slice::sliceByGpu(vector<float> halfedge,float zmin,float zmax)
{
    intrpoints.clear();
    vector<vector<pair<Point,Point>>> edges;
    vector<vector<uint>> faces;
    vector<vector<vector<uint>>> location;
    z.clear();
    zheight=zmin;
    layernumber=0;
    while(zheight<=zmax)
    {
        z.push_back(zheight);
        zheight += thick;
        layernumber++;
    }
    edges.resize(layernumber+1);
    faces.resize(layernumber+1);
    location.reserve(layernumber);

//    time.start();
//    size_t edgenumber=halfedge.size()/7;
//    vector<float> buf;
//    buf.reserve(edgenumber*4);
//    buf.resize(edgenumber*4);
//    /* Create a buffer to hold data */
//    cl::Buffer edgebuf(opencl.context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,halfedge.size()*sizeof(float),halfedge.data());
//    cl::Buffer clbuf(opencl.context, CL_MEM_WRITE_ONLY, edgenumber*4*sizeof(float),buf.data());
//    for(size_t i=0;i<layernumber;i++)
//    {
//        edges[i].reserve(10000);
//        faces[i].reserve(10000);
//        //cout<<i<<":"<<endl;
//        cl::Buffer clz(opencl.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float),&z[i]);
//        /* Create kernel argument */
//        opencl.groupedge2.setArg(0,edgebuf);
//        opencl.groupedge2.setArg(1,clz);
//        opencl.groupedge2.setArg(2,clbuf);
//        cl::NDRange globalSize(edgenumber);
//        opencl.queue.enqueueNDRangeKernel(opencl.groupedge2,cl::NullRange,globalSize,cl::NullRange,NULL,NULL);
//        opencl.queue.finish();
//        opencl.queue.enqueueReadBuffer(clbuf,CL_TRUE,0,edgenumber*4* sizeof(float),&buf[0],NULL,NULL);
//        for(uint j=0;j<edgenumber;j++)
//        {
//            if((buf[j*4+3]+1)<1e-4f)continue;
//            if(isnan(buf[j*4+0]))continue;
//            edges[i].push_back(Point(buf[4*j+0],buf[4*j+1],buf[4*j+2]));
//            faces[i].push_back(uint(buf[4*j+3]));
//        }
//    }
//    findtime +=time.elapsed();
//    vector<float>().swap(buf);

    time.start();
    size_t edgenumber=halfedge.size()/7;
    vector<int> buf;
    buf.resize(edgenumber*3,-1);
    opencl.executeKernel(halfedge,buf,zmin,thick,edgenumber);
    for(size_t i=0;i<edgenumber;i++)
    {
//        cout<<buf[3*i+0]<<" "<<buf[3*i+1]<<" "<<buf[3*i+2]<<endl;
        size_t num1=size_t(buf[3*i+0]);
        size_t num2=size_t(buf[3*i+1]);
        Point p1=Point(halfedge[7*i+0],halfedge[7*i+1],halfedge[7*i+2]);
        Point p2=Point(halfedge[7*i+3],halfedge[7*i+4],halfedge[7*i+5]);
        for(size_t j=num1;j<=num2;j++)
        {
            edges[j].push_back(make_pair(p1,p2));
            faces[j].push_back(uint(buf[3*i+2]));
        }
    }
    findtime +=time.elapsed();

    time.restart();
    vector<vector<uint>>locs;
    vector<uint> face,loc;
    size_t linesnumber=0;
    QMultiHash<uint,uint>facesmap;
    uint index=0,num=0;
    for(uint i=0;i<edges.size();i++)
    {
        //cout<<"The "<<i<<" layer:"<<edges[i].size()<<endl;
        locs.clear();
        facesmap.clear();
        if(edges[i].empty())
        {
            location.push_back(locs);
            continue;
        }
        if(edges[i].size()>linesnumber)linesnumber=edges[i].size();
        facesmap.reserve(int(edges[i].size()));
        for(uint j=0;j<faces[i].size();j++)
        {
            facesmap.insert(faces[i][j],j);
        }
        num=0;
        while(num<edges[i].size()/2)
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
                    face.push_back(faces[i][index+1]);
                }
                else
                {
                    loc.push_back(index-1);
                    face.push_back(faces[i][index-1]);
                }
                facesmap.remove(find_index.key());
            };
            loc.pop_back();
            locs.push_back(loc);
            num +=loc.size();
        }
        location.push_back(locs);
    }
    sorttime =time.elapsed();

    time.restart();
    float *interSection1 =new float[layernumber*linesnumber *3];
    float *interSection2 =new float[layernumber*linesnumber *3];
    float *result =new float[layernumber*linesnumber *3];

    for(size_t i=0;i<layernumber;i++)
    {
        for(size_t j=0;j<edges[i].size();j++)
        {
            Point p1=edges[i][j].first;
            Point p2=edges[i][j].second;
            interSection1[i*3*linesnumber+3*j+0]=float(p1.x());
            interSection1[i*3*linesnumber+3*j+1]=float(p1.y());
            interSection1[i*3*linesnumber+3*j+2]=float(p1.z());
            interSection2[i*3*linesnumber+3*j+0]=float(p2.x());
            interSection2[i*3*linesnumber+3*j+1]=float(p2.y());
            interSection2[i*3*linesnumber+3*j+2]=float(p2.z());
        }
    }
    opencl.executeKernel(interSection1,interSection2,result,layernumber,linesnumber,z.data());
    for(uint i=0;i<layernumber;i++)
    {
        polylines.clear();
        //cout<<"The "<<i<<" layer:"<<edges[i].size()/2<<endl;
        for(size_t j=0;j<location[i].size();j++)
        {
            lines.clear();
            for(size_t k=0;k<location[i][j].size();k++)
            {
                float x=result[i*3*linesnumber+3*location[i][j][k]+0];
                float y=result[i*3*linesnumber+3*location[i][j][k]+1];
                lines.push_back(Point(x,y,z[i]));
            }
            polylines.push_back(lines);
        }
        intrpoints.push_back(polylines);
    }
    free(interSection1);
    free(interSection2);
    free(result);
    comptime +=time.elapsed();

}
