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
            intrsurfs.push_back(intersectFace(*f,true,false));
        }
        else if(zheight==zmin && zheight==zmax)
        {
            intrsurfs.push_back(intersectFace(*f,true,true));
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
            if(intrsurfs[i].isSliced)
            {
                face_descriptor fbegin=intrsurfs[i].fd;
                //cout<<"face:"<<fbegin<<endl;
                if(intrsurfs[i].isParallel)
                {
                    vector<Mesh::Vertex_index>contour;
                    Mesh::Vertex_around_face_range vrange=mesh.vertices_around_face(mesh.halfedge(fbegin));
                    //cout<<*vrange.begin()<<mesh.point(*vrange.begin())<<endl;
                    Mesh::Vertex_index v=*vrange.begin(),vnext;
                    CGAL::Vertex_around_target_circulator<Mesh> vbegin(mesh.halfedge(v),mesh), done(vbegin);
                    contour.push_back(*vbegin);
                    contour.push_back(*vrange.begin());
                    do {
                        if(mesh.point(*vbegin).z()==zheight)
                        {
                            vnext=*vbegin;
                            //cout <<zheight<<":"<<*vbegin<<" "<<mesh.point(*vbegin)<<endl;
                        }
                        vbegin++;
                    } while(vbegin != done);
                    //contour.push_back(vnext);
                    v=vnext;
                    vector<Mesh::Vertex_index>::iterator result;
                    do{
                        contour.push_back(v);
                        CGAL::Vertex_around_target_circulator<Mesh> vbegin(mesh.halfedge(v),mesh), done(vbegin);
                        do {
                            if(mesh.point(*vbegin).z()==zheight)
                            {
                                vector<Mesh::Vertex_index>::iterator it = find( contour.begin( ),contour.end( ),*vbegin); //查找
                                if(it==contour.end( ))
                                {
                                    vnext=*vbegin;
                                    //cout<<*vbegin<<":"<<mesh.i<<endl;
                                    //cout <<zheight<<":"<<*vbegin<<" "<<mesh.point(*vbegin)<<endl;
                                }

                            }
                            vbegin++;
                        } while(vbegin != done);
                        v=vnext;
                        //cout<<"next:"<<vnext<<endl;
                        result = find( contour.begin( ),contour.end( ),vnext); //查找
                    }while(result==contour.end());
                    vector<Mesh::Vertex_index>::iterator it = contour.begin();
                    for(; it != contour.end(); ++it)
                    {
                        points.push_back(mesh.point(*it));
                        cout<<(*it)<<" ";
                    }
                    cout<<endl;
                    break;
                }
                else{
                    face_descriptor fnext=fbegin;
                    Mesh::Halfedge_index hpre;
                    do{
                        vector<intersectFace>::iterator it =find_if(intrsurfs.begin (),intrsurfs.end (),boost::bind (&intersectFace::fd, _1 ) == fnext);
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
                                    points.push_back(intersectPoint(hbegin,zheight));
                                    //cout<<"hnext:"<<*hnext<<endl;
                                }
                            }
                        }
                        hpre=*hnext;
                        //cout<<"hpre:"<<hpre<<endl;
                        fnext=mesh.face(mesh.opposite(*hnext));
                        //cout<<"fnext:"<<fnext<<endl;
                    }while(fnext!=fbegin);
                    points.pop_back();
                }

            }
        }
        intrpoints.push_back(points);
        zheight += thick;
        layernumber++;
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
    cout<<"intersectPoint:"<<x<<" "<<y<<" "<<z<<endl;
    return Point(x,y,z);
}
