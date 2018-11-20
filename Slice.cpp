#include "Slice.h"
#include "Polygon_mesh_slicer_mine.h"
#include <boost/bind.hpp>
#include <CGAL/intersections.h>

#include <algorithm>
#include <QtCore/qmath.h>
#include <QDebug>
#include <windows.h>
#include <QProgressDialog>
#include <QMessageBox>
#include <QFile>
#include <QString>
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
    linesnumber=0;
}

Slice::~Slice()
{

}

void Slice::startSlice(Mesh mesh,float zmin,float zmax)
{  
    findtime=0;
    comptime=0;
    sorttime=0;
    //    QProgressDialog *progressDlg=new QProgressDialog();
    //    progressDlg->setWindowModality(Qt::WindowModal);
    //    progressDlg->setMinimumDuration(0);
    //    progressDlg->setAttribute(Qt::WA_DeleteOnClose, true);
    //    progressDlg->setWindowTitle("切片");
    //    progressDlg->setLabelText("正在切片......");
    //    progressDlg->setRange(zmin,zmax);

    sliceByHeight(mesh,zmin,zmax);

    //sliceByEdge(mesh,zmin,zmax);
    if(isParaComp)
    {
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"gpu compute time:"<<comptime<<"ms"<<endl;
        //cout<<"time of parallel computing:"<<time.elapsed()<<"ms"<<endl;
    }
    else
    {
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
        cout<<"cpu compute time:"<<comptime<<"ms"<<endl;
        //cout<<"time of cpu computing:"<<time.elapsed()<<"ms"<<endl;
    }
    //progressDlg->close();
}

void Slice::startSlice(vector<Vertex> vertexset,multimap<float,Edge>edgeset,vector<Face> faceset,float zmin,float zmax)
{
    sliceByGpu(vertexset,edgeset,faceset,zmin,zmax);
    cout<<"find edge time:"<<findtime<<"ms"<<endl;
    cout<<"sort edge time:"<<sorttime<<"ms"<<endl;
    cout<<"gpu compute time:"<<comptime<<"ms"<<endl;
}

float Slice::adaptSlice(Mesh mesh,Intredges intredges)
{
    vector<float> angle;
    angle.reserve(2000);
    for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
    {
        //cout<<(*iter).size()<<endl;
        for(vector<boost::any>::iterator it=(*iter).begin();it!=(*iter).end();it++)
        {
            try
            {
                Mesh::edge_index ed=boost::any_cast<Mesh::edge_index>(*it);
                Mesh::face_index f0=mesh.face(mesh.halfedge(ed,0));
                //cout<<f<<" ";
                CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
                vector<Point> point;
                for(boost::tie(vbegin, vend) = vertices_around_face(mesh.halfedge(f0), mesh);vbegin != vend;++vbegin)
                {
                    point.push_back(mesh.point(*vbegin));
                    //cout << *vbegin<<":"<<mesh.point(*vbegin)<<endl;
                }
                //cout<<point.size()<<endl;
                float x1,x2,x3,y1,y2,y3,z1,z2,z3,nx,ny,nz;
                //求面f0的法向量
                x1=point[0].x();y1=point[0].y();z1=point[0].z();
                x2=point[1].x();y2=point[1].y();z2=point[1].z();
                x3=point[2].x();y3=point[2].y();z3=point[2].z();
                nx=(y2-y1)*(z3-z1)-(z2-z1)*(y3-y1);
                ny=(z2-z1)*(x3-x1)-(z3-z1)*(x2-x1);
                nz=(x2-x1)*(y3-y1)-(x3-x1)*(y2-y1);
                //cout<<nx<<" "<<ny<<" "<<nz<<endl;
                float dist=sqrt(nx*nx+ny*ny+nz*nz);
                float cos=nz/dist;
                angle.push_back(cos);
            }
            catch(boost::bad_any_cast & ex)
            {
                angle.push_back(1.0);
                //cout<<"cast error:"<<ex.what()<<endl;
            }
        }
        //cout<<endl;
    }
    float minangle=*min_element(angle.begin(),angle.end());
    return minangle;
}

void Slice::sliceByEdge(Mesh mesh,float zmin,float zmax)
{
    vector<vector<Mesh::Edge_index>> edges;
    z.clear();
    intrpoints.clear();
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
    linesnumber=*max_element(size.begin(),size.end());
    float *interSection1,*interSection2,*result;
    interSection1 = (float *)malloc(layernumber*linesnumber *3* sizeof(float));
    interSection2 = (float *)malloc(layernumber*linesnumber *3* sizeof(float));
    result = (float *)malloc(layernumber*linesnumber *3* sizeof(float));

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

void Slice::sliceByHeight(Mesh mesh,float zmin,float zmax)
{
    QFile file("C:/Users/魏超/Desktop/edges.txt");
    if(!file.open(QIODevice::ReadWrite|QIODevice::Text))
    {
        cout<<"can't open"<<endl;
    }
    QTextStream in(&file);
    layernumber=0;
    intrpoints.clear();
    CGAL::Polygon_mesh_slicer<Mesh, Kernel> slicer(mesh);
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
        intredges.clear();
        time.start();
        slicer(Kernel::Plane_3(0, 0, 1, double(-zheight)),back_inserter(intredges));
        findtime +=time.elapsed();

        if(!isParaComp)
        {
            time.restart();
            for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
            {
                lines.clear();
                (*iter).pop_back();
                //cout<<(*iter).size()<<endl;
                in<<"layer of "<<layernumber<<":"<<(*iter).size()<<"\n";
                for(vector<boost::any>::iterator it=(*iter).begin();it!=(*iter).end();it++)
                {
                    try
                    {
                        Mesh::edge_index ed=boost::any_cast<Mesh::edge_index>(*it);
                        in<<ed<<"\n";
                        //cout<<ed<<endl;
                        Point p1=mesh.point(mesh.vertex(ed,0));
                        Point p2=mesh.point(mesh.vertex(ed,1));
                        Segment s(p1,p2);
                        CGAL::cpp11::result_of<Kernel::Intersect_3(Kernel::Plane_3, Segment)>::type
                                  inter = intersection(Kernel::Plane_3(0, 0, 1, -zheight), s);
                        CGAL_assertion(inter != boost::none);
                        const Point* pt_ptr = boost::get<Point>(&(*inter));
                        lines.push_back(*pt_ptr);                       
                    }
                    catch(boost::bad_any_cast & ex)
                    {
                        //cout<<"cast error:"<<ex.what()<<endl;
                        Point point=boost::any_cast<Point>(*it);
                        lines.push_back(point);
                        //cout<<vd<<endl;
                    }
                }
//                for(size_t i=0;i<lines.size();i++)
//                {
//                    cout<<lines[i].x()<<" "<<lines[i].y()<<" "<<lines[i].z()<<endl;
//                }
                polylines.push_back(lines);
            }
            intrpoints.push_back(polylines);
            comptime +=time.elapsed();
        }
        else
        {
            time.restart();
            int lineNum=0;
            for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
            {
                (*iter).pop_back();
                //cout<<(*iter).size()<<endl;
                for(vector<boost::any>::iterator it=(*iter).begin();it!=(*iter).end();it++)
                {
                    try
                    {
                        Mesh::edge_index ed=boost::any_cast<Mesh::edge_index>(*it);
                        lineNum +=(*iter).size();
                        break;
                    }
                    catch(boost::bad_any_cast & ex)
                    {
                        cout<<"cast error:"<<ex.what()<<endl;
                    }
                }
            }
            if(lineNum==0)
            {
                for(list<Outline>::iterator iter=intredges.begin();iter !=intredges.end();iter++)
                {
                    lines.clear();
                    //cout<<(*iter).size()<<endl;
                    for(vector<boost::any>::iterator it=(*iter).begin();it!=(*iter).end();it++)
                    {
                        try
                        {
                            Point point=boost::any_cast<Point>(*it);
                            lines.push_back(point);
                        }
                        catch(boost::bad_any_cast & ex)
                        {
                            //cout<<"cast error:"<<ex.what()<<endl;
                        }
                    }
                    polylines.push_back(lines);
                }
            }
            else
            {
                float *interSection1,*interSection2,*result;
                result  = (float *)malloc(lineNum *3* sizeof(float));
                interSection1 = (float *)malloc(lineNum *3* sizeof(float));
                interSection2 = (float *)malloc(lineNum *3* sizeof(float));
                uint num=0;
                for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
                {
                    for(uint j=0;j<(*iter).size();j++)
                    {
                        try
                        {
                            Mesh::edge_index ed=boost::any_cast<Mesh::edge_index>((*iter)[j]);
                            //cout<<ed<<endl;
                            Point p1=mesh.point(mesh.vertex(ed,0));
                            Point p2=mesh.point(mesh.vertex(ed,1));
                            //cout<<p1.x()<<" "<<p1.y()<<" "<<p1.z()<<endl;
                            interSection1[3*(num+j)+0]=float(p1.x());
                            interSection1[3*(num+j)+1]=float(p1.y());
                            interSection1[3*(num+j)+2]=float(p1.z());
                            interSection2[3*(num+j)+0]=float(p2.x());
                            interSection2[3*(num+j)+1]=float(p2.y());
                            interSection2[3*(num+j)+2]=float(p2.z());
                        }
                        catch(boost::bad_any_cast & ex)
                        {
                            //cout<<"cast error:"<<ex.what()<<endl;
                        }
                    }
                    num +=(*iter).size();
                }
                opencl.executeKernel(interSection1,interSection2,result,lineNum,zheight);
                num=0;
                for(list<Outline>::iterator iter= intredges.begin();iter !=intredges.end();iter++)
                {
                    lines.clear();
                    for(uint j=0;j<(*iter).size();j++)
                    {
                        try
                        {
                            Mesh::edge_index ed=boost::any_cast<Mesh::edge_index>((*iter)[j]);
                            float x=result[3*(num+j)];
                            float y=result[3*(num+j)+1];
                            float z=result[3*(num+j)+2];
                            lines.push_back(Point(x,y,z));
                        }
                        catch(boost::bad_any_cast & ex)
                        {
                            //cout<<"cast error:"<<ex.what()<<endl;
                            Point point=boost::any_cast<Point>((*iter)[j]);
                            lines.push_back(point);
                        }
                    }
                    num +=(*iter).size();
                    polylines.push_back(lines);
                }
                free(interSection1);
                free(interSection2);
                free(result);
            }
            intrpoints.push_back(polylines);
            comptime +=time.elapsed();
        }
        layernumber++;
        if(isAdapt)
        {
            float adaptthick;
            float minthick=0.1f,maxthick=0.3f;
            float minangle=adaptSlice(mesh,intredges);
            if(minangle>0.99f)
                adaptthick=maxthick;
            else
                adaptthick=minthick/(minangle+0.001f);
            if(adaptthick<minthick)adaptthick=minthick;
            if(adaptthick>maxthick)adaptthick=maxthick;
            //cout<<adaptthick<<endl;
            zheight += adaptthick;
        }
        else
            zheight += thick;
    }
    file.close();
}

void Slice::sliceByGpu(vector<Vertex> vertexset,multimap<float,Edge>edgeset,vector<Face> faceset,float zmin,float zmax)
{
    findtime=0;
    comptime=0;
    sorttime=0;
    vector<vector<pair<Point,Point>>> edges;
    vector<vector<uint>> faces;
    vector<vector<vector<uint>>> location;
    z.clear();
    intrpoints.clear();
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
    faces.resize(layernumber);
    location.reserve(layernumber);
    time.start();
    for(multimap<float,Edge>::const_iterator it=edgeset.begin();it!=edgeset.end();it++)
    {

        Point p1=vertexset[it->second.v0].point;
        Point p2=vertexset[it->second.v1].point;
        //cout<<it->first<<":"<<it->second.id<<" "<<p1<<" "<<p2<<endl;
        float z1=it->second.zmin;
        float z2=it->second.zmax;
        vector<float>::iterator location_index=find_if(z.begin(),z.end(),bind2nd(greater<float>(),z1));
        size_t num1=size_t(location_index-z.begin());
        location_index=find_if(z.begin(),z.end(),bind2nd(greater<float>(),z2));
        size_t num2=size_t(location_index-z.begin())-1;
        for(size_t i=num1;i<=num2;i++)
        {
            edges[i].push_back(make_pair(p1,p2));
            faces[i].push_back(it->second.f);
        }
    }
    findtime =time.elapsed();

    time.restart();
    vector<size_t> size;
    size.reserve(edges.size());
    for(size_t i=0;i<edges.size();i++)
    {
        vector<vector<uint>>locs;
        vector<uint> face,loc;
        vector<uint>::iterator it;
        uint index,num=0;
        //cout<<"The "<<i<<" layer:"<<edges[i].size()/2<<endl;
        if(edges[i].empty())
        {
            locs.push_back(loc);
            location.push_back(locs);
            continue;
        }
        size.push_back(edges[i].size());

        while(num<faces[i].size()/2)
        {
            face.clear();
            loc.clear();
            it=find_if(faces[i].begin(),faces[i].end(),bind2nd(not_equal_to<uint>(),INT_MAX));
            index=uint(it-faces[i].begin());
            face.push_back(faces[i][index]);
            loc.push_back(index);
            faces[i][index]=INT_MAX;
            faces[i][index+1]=INT_MAX;
            while(1)
            {
                it=find(faces[i].begin(),faces[i].end(),face.back());
                if(it==faces[i].end())
                {
                    break;
                }
                index=uint(it-faces[i].begin());
                faces[i][index]=INT_MAX;
                if((index & 1) == 0)
                {
                    loc.push_back(index+1);
                    face.push_back(faces[i][index+1]);
                    faces[i][index+1]=INT_MAX;
                }
                else
                {
                    loc.push_back(index-1);
                    face.push_back(faces[i][index-1]);
                    faces[i][index-1]=INT_MAX;
                }
            };
            locs.push_back(loc);
            num +=loc.size();
        }
        location.push_back(locs);

//        vector<pair<Point,Point>>edge;
//        edge.reserve(lines.size());
//        for(size_t j=0;j<loc.size();j++)
//        {
//            edge.push_back(edges[i][loc[j]]);
//        }
//        edges[i]=edge;

//        for(size_t j=0;j<edges[i].size();j++)
//        {
//            cout<<edges[i][j].first<<" "<<edges[i][j].second<<" "<<lines[j]<<endl;
//        }
//        cout<<endl;
    }
    sorttime =time.elapsed();

    time.restart();
    linesnumber=*max_element(size.begin(),size.end());
    float *interSection1,*interSection2,*result;
    interSection1 = (float *)malloc(layernumber*linesnumber *3* sizeof(float));
    interSection2 = (float *)malloc(layernumber*linesnumber *3* sizeof(float));
    result = (float *)malloc(layernumber*linesnumber *3* sizeof(float));

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
    for(size_t i=0;i<layernumber;i++)
    {
        polylines.clear();
        //cout<<"The "<<i<<" layer:"<<edges[i].size()/2<<endl;
        for(size_t j=0;j<location[i].size();j++)
        {
            lines.clear();
            for(size_t k=0;k<location[i][j].size();k++)
            {
                //cout<<location[i][j][k]<<endl;
                float x=result[i*3*linesnumber+3*(location[i][j][k])+0];
                float y=result[i*3*linesnumber+3*(location[i][j][k])+1];
                lines.push_back(Point(x,y,z[i]));
                //cout<<x<<" "<<y<<" "<<z[i]<<endl;
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
