#include "Slice.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <boost/bind.hpp>
#include <algorithm>
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
        if(zheight>zmin && zheight <zmax)
        {
            intrsurfs.push_back(intersectFace(*f,true));
        }
        else if(zheight==zmin && zheight==zmax)
        {
            intrsurfs.push_back(intersectFace(*f,true));
        }
    }
    cout<<"number of intrsurfs:"<<intrsurfs.size()<<endl;
}

void Slice::intrPoints(double zmin,double zmax)
{
    double zheight=zmin;
    while(zheight<=zmax)
    {
        vector<Point> points;
        cout<<"layer of "<<layernumber+1<<":"<<endl;
        intrSurfs(zheight);
        for(int i=0;i<intrsurfs.size();i++)
        {
            if(intrsurfs[i].Flag)
            {
                face_descriptor fbegin=intrsurfs[i].fd;
                cout<<"face:"<<fbegin<<endl;
                face_descriptor fnext=fbegin;
                Mesh::Halfedge_index hpre;
                do{
                    vector<intersectFace>::iterator it =find_if(intrsurfs.begin (),intrsurfs.end (),boost::bind (&intersectFace::fd, _1 ) == fnext);
                    (*it).Flag=false;
                    int i=0;
                    CGAL::Halfedge_around_face_iterator<Mesh>hbegin,hend,hnext;
                    for(boost::tie(hbegin,hend)= halfedges_around_face(mesh.halfedge(fnext), mesh);hbegin!=hend;++hbegin)
                    {
                        i=i+1;
                        if(isIntr(hbegin,zheight,i))
                        {
                            if(hpre != mesh.opposite(*hbegin))
                            {
                               points.push_back(intersectPoint(hbegin,zheight,i));
                               vector<intersectFace>::iterator it =find_if(intrsurfs.begin (),intrsurfs.end (),boost::bind (&intersectFace::fd, _1 ) ==mesh.face(mesh.opposite(*hbegin)));
                               if(it != intrsurfs.end())
                               {
                                   hnext=hbegin;
                               }
                               else
                               {
                                   *hnext=mesh.opposite(hpre);
                               }
                            }
                        }
                    }
                    hpre=*hnext;
                    cout<<"hnext:"<<*hnext<<endl;
                    fnext=mesh.face(mesh.opposite(*hnext));
                    cout<<"fnext:"<<fnext<<endl;
                }while(fnext!=fbegin);
            }
        }
        intrpoints.push_back(points);
        zheight += thick;
        layernumber++;

    }

}

bool Slice::isIntr(CGAL::Halfedge_around_face_iterator<Mesh> e,double zheight,int i)
{
    Mesh::Edge_index edgeindex=mesh.edge(*e);
    Point p1,p2;
    if(i==1)
    {
        p1=mesh.point(mesh.vertex(edgeindex,1));
        p2=mesh.point(mesh.vertex(edgeindex,0));
    }
    else
    {
        p1=mesh.point(mesh.vertex(edgeindex,0));
        p2=mesh.point(mesh.vertex(edgeindex,1));
    }
    cout<<"p1:"<<p1.x()<<" "<<p1.y()<<" "<<p1.z()<<endl;
    cout<<"p2:"<<p2.x()<<" "<<p2.y()<<" "<<p2.z()<<endl;
    if(p1.z()<zheight && p2.z()>=zheight)
        return true;
    else if(p2.z()<zheight && p1.z()>=zheight)
        return true;
    else if(p2.z()==zheight && p1.z()==zheight)
        return true;
    else
        return false;
}

Point Slice::intersectPoint(CGAL::Halfedge_around_face_iterator<Mesh> e,double z,int i)
{
    Mesh::Edge_index edgeindex=mesh.edge(*e);
    Point p1,p2;
    if(i==1)
    {
        p1=mesh.point(mesh.vertex(edgeindex,1));
        p2=mesh.point(mesh.vertex(edgeindex,0));
    }
    else
    {
        p1=mesh.point(mesh.vertex(edgeindex,0));
        p2=mesh.point(mesh.vertex(edgeindex,1));
    }

    double x,y;
    if(p1.z()==z || p2.z()==z)
    {
        x=p1.x();
        y=p1.y();
    }
    else
    {
        x=p1.x()+(p2.x()-p1.x())*(z-p1.z())/(p2.z()-p1.z());
        y=p1.y()+(p2.y()-p1.y())*(z-p1.z())/(p2.z()-p1.z());
    }
    cout<<"intersectPoint:"<<x<<" "<<y<<" "<<z<<endl;
    return Point(x,y,z);
}
