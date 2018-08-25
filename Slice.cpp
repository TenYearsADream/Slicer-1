#include "Slice.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <boost/bind.hpp>
#include <algorithm>
#include <QtCore/qmath.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Mesh::Vertex_index vertex_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
typedef Kernel::Point_3 Point;
using namespace std;
Slice::Slice()
{
    thick=10;
    layernumber=0;
}

Slice::~Slice()
{

}

void Slice::intrSurfs(double zheight)
{
    vector<intersectFace>().swap(intrsurfs);
    vector<float>().swap(normalangle);
    for(Mesh::Face_iterator f=mesh.faces_begin();f!=mesh.faces_end();f++)
    {
        CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
        vector<double>height;
        for(boost::tie(vbegin, vend) = vertices_around_face(mesh.halfedge(*f), mesh);vbegin != vend;++vbegin)\
        {
            height.push_back(mesh.point(*vbegin).z());
        }
        double zmin=*min_element(height.begin(),height.end());
        double zmax=*max_element(height.begin(),height.end());
        //cout<<zmin<<" "<<zmax<<" "<<zheight<<endl;
        if((zheight-zmin)>1e-4 && (zmax-zheight)>1e-4)
        {
            float cos=normalAngle(*f);
            normalangle.push_back(sqrt(1-cos*cos));
            intrsurfs.push_back(intersectFace(*f,true,false));
        }
        else if((qAbs(zheight-zmin)<=1e-4) && (qAbs(zheight-zmax)<=1e-4))
        {
            float cos=normalAngle(*f);
            normalangle.push_back(sqrt(1-cos*cos));
            intrsurfs.push_back(intersectFace(*f,true,true));
        }
    }
    //cout<<"number of intrsurfs:"<<intrsurfs.size()<<endl;
}

void Slice::intrPoints(double zmin,double zmax)
{
    double zheight=zmin;
    while(zheight<=zmax)
    {
        vector<vector<Point>> points;
        //cout<<"layer of "<<layernumber+1<<":"<<endl;
        intrSurfs(zheight);
        vector<Mesh::Vertex_index>contour;
        for(int i=0;i<intrsurfs.size();i++)
        {
            if(intrsurfs[i].isSliced)
            {
                Mesh::Face_index fbegin=intrsurfs[i].Faceindex;
                //cout<<"face:"<<fbegin<<endl;
                vector<Mesh::Vertex_index>contourpoints;
                vector<Point>point;
                if(intrsurfs[i].isParallel)
                {                   
                    Mesh::Vertex_index v,vnext;
                    CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
                    for(boost::tie(vbegin, vend) = vertices_around_face(mesh.halfedge(fbegin), mesh);vbegin != vend;++vbegin)
                    {
                        vector<Mesh::Vertex_index>::iterator it = find(contour.begin( ),contour.end( ),*vbegin); //查找
                        if(it==contour.end( ))
                        {
                            v=*vbegin;
                            //cout<<"start point "<<*vbegin<<":"<<mesh.point(*vbegin)<<endl;
                        }
                    }
                    Mesh::Halfedge_index h0,h1;
                    Mesh::Face_index f0,f1;
                    vector<Mesh::Vertex_index>::iterator result;
                    do{
                        contourpoints.push_back(v);
                        contour.push_back(v);
                        CGAL::Vertex_around_target_circulator<Mesh> vbegin(mesh.halfedge(v),mesh), done(vbegin);
                        do {
                            //cout<<*vbegin<<endl;
                            if(qAbs(mesh.point(*vbegin).z()-zheight)<1e-4)
                            {
                                vector<Mesh::Vertex_index>::iterator it = find(contourpoints.begin( ),contourpoints.end( ),*vbegin); //查找
                                if(it==contourpoints.end( ))
                                {
                                    h0=mesh.halfedge(v,*vbegin);
                                    h1=mesh.opposite(h0);
                                    //cout<<h0<<h1<<endl;
                                    f0=mesh.face(h0);
                                    f1=mesh.face(h1);
                                    //cout<<f0<<f1<<endl;
                                    vector<intersectFace>::iterator it0 =find_if(intrsurfs.begin (),intrsurfs.end (),boost::bind (&intersectFace::Faceindex, _1 ) == f0);
                                    vector<intersectFace>::iterator it1 =find_if(intrsurfs.begin (),intrsurfs.end (),boost::bind (&intersectFace::Faceindex, _1 ) == f1);
                                    if(it0 == intrsurfs.end() || it1 == intrsurfs.end())
                                    {
                                        if(it0 != intrsurfs.end())
                                        {
                                            (*it0).isSliced=false;
                                        }
                                        else
                                        {
                                            (*it1).isSliced=false;
                                        }
                                        vnext=*vbegin;
                                        //cout <<zheight<<":"<<*vbegin<<" "<<mesh.point(*vbegin)<<endl;
                                    }
                                }
                            }
                            vbegin++;
                        } while(vbegin != done);
                        v=vnext;
                        //cout<<"next:"<<vnext<<endl;
                        result = find(contourpoints.begin( ),contourpoints.end( ),vnext); //查找
                    }while(result==contourpoints.end());
                    for(int i=0;i<contourpoints.size();i++)
                    {
                        point.push_back(mesh.point(contourpoints[i]));
                        //cout<<contourpoints[i]<<" ";
                    }
                    //cout<<endl;
                    points.push_back(point);
                }
                else{
                    face_descriptor fnext=fbegin;
                    Mesh::Halfedge_index hpre;
                    do{
                        vector<intersectFace>::iterator it =find_if(intrsurfs.begin (),intrsurfs.end (),boost::bind (&intersectFace::Faceindex, _1 ) == fnext);
                        (*it).isSliced=false;
                        CGAL::Halfedge_around_face_iterator<Mesh>hbegin,hend,hnext;
                        for(boost::tie(hbegin,hend)= halfedges_around_face(mesh.halfedge(fnext), mesh);hbegin!=hend;++hbegin)
                        {
                            //cout<<*hbegin<<endl;
                            if(isIntr(hbegin,zheight))
                            {
                                if(hpre != mesh.opposite(*hbegin))
                                {
                                    hnext=hbegin;
                                    point.push_back(intersectPoint(hbegin,zheight));
                                    //cout<<"hnext:"<<*hnext<<endl;
                                }
                            }
                        }
                        hpre=*hnext;
                        //cout<<"hpre:"<<hpre<<endl;
                        fnext=mesh.face(mesh.opposite(*hnext));
                        //cout<<"fnext:"<<fnext<<" fpre："<<mesh.face(*hnext)<<endl;
                    }while(fnext!=fbegin);
                    point.pop_back();
                    points.push_back(point);
                }
            }
        }
        if(!points.empty())
        {
            //cout<<points[0].size()<<endl;
            intrpoints.push_back(sliceData(areaSort(points)));
            layernumber++;
//            for(int i=0;i<normalangle.size();i++)
//            {
//                cout<<normalangle[i]<<" ";
//            }
//            cout<<endl;
            float minnormalangle=*min_element(normalangle.begin(),normalangle.end());
            //cout<<minnormalangle<<endl;
            if(minnormalangle>0.99)
                adaptthick=0.3;
            else
                adaptthick=0.05/(minnormalangle+1e-3);
        }
        if(adaptthick<0.1)adaptthick=0.1;
        if(adaptthick>0.3)adaptthick=0.3;
        //cout<<adaptthick<<endl;
        if(isAdapt)
            zheight += adaptthick;
        else
            zheight += thick;
    }
}

bool Slice::isIntr(CGAL::Halfedge_around_face_iterator<Mesh> e,double zheight)
{
    Mesh::Edge_index edgeindex=mesh.edge(*e);
    Point p1=mesh.point(mesh.vertex(edgeindex,0));
    Point p2=mesh.point(mesh.vertex(edgeindex,1));
    //cout<<"p1:"<<p1.x()<<" "<<p1.y()<<" "<<p1.z()<<endl;
    //cout<<"p2:"<<p2.x()<<" "<<p2.y()<<" "<<p2.z()<<endl;
    if(p1.z()<zheight && p2.z()>=zheight)
        return true;
    else if(p2.z()<zheight && p1.z()>=zheight)
        return true;
    else if(p2.z()==zheight && p1.z()==zheight)
        return true;
    else
        return false;
}

Point Slice::intersectPoint(CGAL::Halfedge_around_face_iterator<Mesh> e,double z)
{
    Mesh::Edge_index edgeindex=mesh.edge(*e);
    Point p1=mesh.point(mesh.vertex(edgeindex,0));
    Point p2=mesh.point(mesh.vertex(edgeindex,1));
    double x=p1.x()+(p2.x()-p1.x())*(z-p1.z())/(p2.z()-p1.z());
    double y=p1.y()+(p2.y()-p1.y())*(z-p1.z())/(p2.z()-p1.z());
    //cout<<"intersectPoint:"<<x<<" "<<y<<" "<<z<<endl;
    return Point(x,y,z);
}

float Slice::normalAngle(Mesh::Face_index f0)
{
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
    //cout<<cos<<endl;
    return cos;
}

vector<vector<Point>> Slice::areaSort(vector<vector<Point>> points)
{
    vector<float>area;
    for(int i=0;i<points.size();i++)
    {
        float S=0;
        for(int j=1;j<points[i].size()-1;j++)
        {
            float x1,x2,x3,y1,y2,y3;
            x1=points[i][0].x();y1=points[i][0].y();
            x2=points[i][j].x();y2=points[i][j].y();
            x3=points[i][j+1].x();y3=points[i][j+1].y();
            S +=qAbs((x1*y2+x2*y3+x3*y1-x1*y3-x2*y1-x3*y2)/2.0);
        }
        //cout<<"面积："<<S<<endl;
        area.push_back(S);
    }
    auto max = max_element(area.begin(), area.end());
    vector<Point>tmp;
    tmp=points[distance(area.begin(), max)];
    points[distance(area.begin(), max)]=points[0];
    points[0]=tmp;
//    for(int i=0;i<points.size();i++)
//    {
//        cout<<"第"<<i+1<<"个圈"<<endl;
//        for(int j=0;j<points[i].size();j++)
//        {
//            cout<<points[i][j].x()<<" "<<points[i][j].y()<<" "<<points[i][j].z()<<endl;
//        }

//    }
    return points;
}
