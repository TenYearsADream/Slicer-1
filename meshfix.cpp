#include "meshfix.h"
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/stitch_borders.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <boost/function_output_iterator.hpp>
#include <boost/property_map/property_map.hpp>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <map>
#include "loadprogressbar.h"
#include <QApplication>
using namespace std;

MeshFix::MeshFix(QObject *parent):QObject(parent)
{
}
MeshFix::~MeshFix()
{

}

void MeshFix::repair(Mesh &mesh)
{
    loadProgressBar progressbar("mesh repairing...");
    connect(this,SIGNAL(progressReport(float,float)),&progressbar,SLOT(setProgressBar(float,float)));
    //缝合边界
    progressbar.setLabelText("stitchBorders");
    stitchBorders(&mesh);
    float fraction=0.1f;
    emit progressReport(100*fraction,100.0f);
    QApplication::processEvents();

    //去除小的连通域
    progressbar.setLabelText("fixConnectivity");
    fixConnectivity(&mesh);
    fraction=0.3f;
    emit progressReport(100*fraction,100.0f);
    QApplication::processEvents();

    //去除自相交面片
    progressbar.setLabelText("selfIntersect");
    selfIntersect(&mesh);
    fraction=0.6f;
    emit progressReport(100*fraction,100.0f);
    QApplication::processEvents();

    //填补洞
    progressbar.setLabelText("holeFill");
    holeFill(&mesh);
    fraction=0.9f;
    emit progressReport(100*fraction,100.0f);
    QApplication::processEvents();

    //修复法向
    if(CGAL::is_closed(mesh))
    {
        normalRepair(&mesh);
    }
    fraction=1.0f;
    emit progressReport(100*fraction,100.0f);
    QApplication::processEvents();
}

void MeshFix::fixConnectivity(Mesh *mesh)
{
    Mesh::Property_map<Mesh::face_index,size_t> fccmap =mesh->add_property_map<Mesh::face_index,size_t>("f:CC").first;
    size_t num =CGAL::Polygon_mesh_processing::connected_components(*mesh,fccmap);    
    //keep only components which have at least 10 faces
    CGAL::Polygon_mesh_processing::keep_large_connected_components(*mesh,10);
    //keep the one largest components
    CGAL::Polygon_mesh_processing::keep_largest_connected_components(*mesh,1);
    if(mesh->has_garbage())
    {
        mesh->collect_garbage();
    }
    cout << "fixing connectivity done : " <<endl;
    cout<<"\t "<<num-1<<" small connected components have been removed."<<endl;
    cout << "\t Number of vertices  :\t" << mesh->num_vertices()<<endl;
    cout << "\t Number of halfedges :\t" << mesh->num_halfedges() <<endl;
    cout << "\t Number of facets    :\t" << mesh->num_faces()<<endl;

    emit outputMsg("fixing connectivity done :");
    emit outputMsg("\t remove"+QString::number(num-1)+"small connectivities.");
    emit outputMsg("\t Number of vertices  :\t"+QString::number(mesh->num_vertices()));
    emit outputMsg("\t Number of halfedges :\t"+QString::number(mesh->num_halfedges()));
    emit outputMsg("\t Number of facets    :\t" +QString::number(mesh->num_faces()));
    QApplication::processEvents();
}

void MeshFix::holeFill(Mesh *mesh)
{
    int nb_holes = 0;
    for(Mesh::Halfedge_index h:mesh->halfedges())
    {
      if(mesh->is_border(h))
      {
        vector<Mesh::Face_index>  patch_facets;
        vector<Mesh::Vertex_index> patch_vertices;
        bool success = CGAL::cpp11::get<0>(CGAL::Polygon_mesh_processing::triangulate_refine_and_fair_hole
                (*mesh,h,back_inserter(patch_facets), back_inserter(patch_vertices),
                CGAL::Polygon_mesh_processing::parameters::vertex_point_map(get(CGAL::vertex_point, *mesh)).geom_traits(Kernel())));

//        cout << " Number of facets in constructed patch: " << patch_facets.size() <<endl;
//        cout << " Number of vertices in constructed patch: " << patch_vertices.size() <<endl;
//        cout << " Fairing : " << (success ? "succeeded" : "failed") <<endl;
        if(success)
        {
            ++nb_holes;
        }
      }
    }
    cout << "filling holes done : " <<endl;
    cout <<"\t "<<nb_holes << " holes have been filled" <<endl;
    cout << "\t Number of vertices  :\t" << mesh->num_vertices()<<endl;
    cout << "\t Number of halfedges :\t" << mesh->num_halfedges() <<endl;
    cout << "\t Number of facets    :\t" << mesh->num_faces()<<endl;

    emit outputMsg("filling holes done : ");
    emit outputMsg("repair "+QString::number(nb_holes)+" holes.");
    emit outputMsg( "\t Number of vertices  :\t" +QString::number(mesh->num_vertices()));
    emit outputMsg("\t Number of halfedges :\t"+QString::number(mesh->num_halfedges()));
    emit outputMsg("\t Number of facets    :\t"+QString::number(mesh->num_faces()));
    QApplication::processEvents();
}

void MeshFix::normalRepair(Mesh *mesh)
{
    bool oriented=CGAL::Polygon_mesh_processing::is_outward_oriented(*mesh);
    if(!oriented)
    {
        CGAL::Polygon_mesh_processing::orient(*mesh);
        cout<<"normal error!"<<endl;
        emit outputMsg("normal error!");
        QApplication::processEvents();
    }
    else
    {
        cout<<"normal right!"<<endl;
        emit outputMsg("normal right!");
        QApplication::processEvents();
    }
}

void MeshFix::selfIntersect(Mesh *mesh)
{
    bool intersecting;
    intersecting =CGAL::Polygon_mesh_processing::does_self_intersect(*mesh,CGAL::Polygon_mesh_processing::parameters::vertex_point_map(get(CGAL::vertex_point,*mesh)));
    cout<< (intersecting ? "There are self-intersections." : "There is no self-intersection.")<<endl;
    if(intersecting)
    {
        emit outputMsg("There are self-intersections.");
        QApplication::processEvents();
    }
    else
    {
        emit outputMsg("There is no self-intersection.");
        QApplication::processEvents();
    }
    while(intersecting)
    {
        vector<pair<Mesh::Face_index, Mesh::Face_index> > intersected_tris;
        CGAL::Polygon_mesh_processing::self_intersections(*mesh,back_inserter(intersected_tris));
        cout << intersected_tris.size() << " pairs of triangles intersect." << std::endl;
        emit outputMsg(QString::number(intersected_tris.size())+"对三角形自相交.");
        QApplication::processEvents();
        for(uint i=0;i<intersected_tris.size();i++)
        {
            Mesh::Face_index f0=intersected_tris[i].first;
            Mesh::Face_index f1=intersected_tris[i].second;
            mesh->remove_face(f0);
            mesh->remove_face(f1);
        }
        mesh->collect_garbage();

        vector<Point> vertices;
        vector<Mesh::Vertex_index> faces;
        vertices.reserve(mesh->number_of_vertices());
        faces.reserve(3*mesh->number_of_faces());
        for(Mesh::Vertex_index v:mesh->vertices())
        {
            vertices.push_back(mesh->point(v));
        }
        for(Mesh::Face_index f:mesh->faces())
        {
            CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
            for(boost::tie(vbegin, vend) = vertices_around_face(mesh->halfedge(f), *mesh);vbegin != vend; ++vbegin){
                faces.push_back(*vbegin);
            }
        }
        mesh->clear();
        for(uint i=0;i<vertices.size();i++)
        {
            mesh->add_vertex(vertices[i]);
        }
        for(uint i=0;i<faces.size()/3;i++)
        {
            Mesh::Vertex_index v0(faces[3*i+0]);
            Mesh::Vertex_index v1(faces[3*i+1]);
            Mesh::Vertex_index v2(faces[3*i+2]);
            mesh->add_face(v0,v1,v2);
        }
//        fixConnectivity();
//        holeFill();
        intersecting = CGAL::Polygon_mesh_processing::does_self_intersect(*mesh,
                                                CGAL::Polygon_mesh_processing::parameters::vertex_point_map(get(CGAL::vertex_point, *mesh)));
        cout<< (intersecting ? "There are self-intersections." : "There is no self-intersection.")<<endl;
        if(intersecting)
        {
            emit outputMsg("存在自相交.");
            QApplication::processEvents();
        }
        else
        {
            emit outputMsg("There is no self-intersection.");
            QApplication::processEvents();
        }
    }
    cout << "repairing selfintersection done : " <<endl;
    cout << "\t Number of vertices  :\t" << mesh->number_of_vertices()<<endl;
    cout << "\t Number of halfedges :\t" << mesh->number_of_halfedges() <<endl;
    cout << "\t Number of facets    :\t" << mesh->number_of_faces()<<endl;

    emit outputMsg("repairing selfintersection done : ");
    emit outputMsg("\t Number of vertices  :"+QString::number(mesh->number_of_vertices()));
    emit outputMsg("\t Number of halfedges :\t"+QString::number(mesh->number_of_halfedges()));
    emit outputMsg("\t Number of facets    :\t"+QString::number(mesh->number_of_faces()));
    QApplication::processEvents();
}

void MeshFix::stitchBorders(Mesh *mesh)
{
    std::cout << "Before stitching : " << std::endl;
    std::cout << "\t Number of vertices  :\t" << mesh->number_of_vertices()<< std::endl;
    std::cout << "\t Number of halfedges :\t" << mesh->number_of_halfedges()<< std::endl;
    std::cout << "\t Number of facets    :\t" << mesh->number_of_faces() << std::endl;
    CGAL::Polygon_mesh_processing::stitch_borders(*mesh);
    std::cout << "Stitching done : " << std::endl;
    std::cout << "\t Number of vertices  :\t" << mesh->number_of_vertices()<< std::endl;
    std::cout << "\t Number of halfedges :\t" << mesh->number_of_halfedges()<< std::endl;
    std::cout << "\t Number of facets    :\t" << mesh->number_of_faces() << std::endl;
}
