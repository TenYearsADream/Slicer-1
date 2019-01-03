#include "meshfix.h"
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/stitch_borders.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <boost/function_output_iterator.hpp>
#include <boost/property_map/property_map.hpp>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <map>
using namespace std;

MeshFix::MeshFix(Mesh *_mesh)
{
    mesh=_mesh;
    repair();
}
MeshFix::~MeshFix()
{

}

void MeshFix::repair()
{
    //去除小的连通域
    fixConnectivity();
    //去除自相交面片
    selfIntersect();
    //填补洞
    holeFill();
    //修复法向
    if(CGAL::is_closed(*mesh))
    {
        normalRepair();
    }
}

void MeshFix::fixConnectivity()
{
    Mesh::Property_map<Mesh::face_index,size_t> fccmap =mesh->add_property_map<Mesh::face_index,size_t>("f:CC").first;
    size_t num =CGAL::Polygon_mesh_processing::connected_components(*mesh,fccmap);    
    //keep only components which have at least 4 faces
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
}

void MeshFix::holeFill()
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
        ++nb_holes;
      }
    }
    cout << "filling holes done : " <<endl;
    cout <<"\t "<<nb_holes << " holes have been filled" <<endl;
    cout << "\t Number of vertices  :\t" << mesh->num_vertices()<<endl;
    cout << "\t Number of halfedges :\t" << mesh->num_halfedges() <<endl;
    cout << "\t Number of facets    :\t" << mesh->num_faces()<<endl;
}

void MeshFix::normalRepair()
{
    bool oriented=CGAL::Polygon_mesh_processing::is_outward_oriented(*mesh);
    if(!oriented)
    {
        CGAL::Polygon_mesh_processing::orient(*mesh);
        cout<<"normal error!"<<endl;
    }
    else
    {
        cout<<"normal right!"<<endl;
    }
}

void MeshFix::selfIntersect()
{
    bool intersecting;
    intersecting =CGAL::Polygon_mesh_processing::does_self_intersect(*mesh,CGAL::Polygon_mesh_processing::parameters::vertex_point_map(get(CGAL::vertex_point,*mesh)));
    //cout<< (intersecting ? "There are self-intersections." : "There is no self-intersection.")<<endl;
    while(intersecting)
    {
        vector<pair<Mesh::Face_index, Mesh::Face_index> > intersected_tris;
        CGAL::Polygon_mesh_processing::self_intersections(*mesh,back_inserter(intersected_tris));
        cout << intersected_tris.size() << " pairs of triangles intersect." << std::endl;

        vector<Mesh::Halfedge_index>H0(3),H1(3);
        Mesh::Vertex_index v,v4,v5,v6,v7;
        Mesh::Halfedge_index h,h0,h1,h2;
        Mesh::Face_index f;
        vector<Mesh::Vertex_index> repairfaces;
        //cout<<intersected_tris[0].first<<" "<<intersected_tris[0].second<<endl;
        Mesh::Face_index f0=intersected_tris[0].first;
        Mesh::Face_index f1=intersected_tris[0].second;
        H0[0]=mesh->halfedge(f0);
        H0[1]=mesh->next(H0[0]);
        H0[2]=mesh->prev(H0[0]);
        H1[0]=mesh->halfedge(f1);
        H1[1]=mesh->next(H1[0]);
        H1[2]=mesh->prev(H1[0]);
        Point v0,v1,v2,v3;
        Mesh::Edge_index e;
        bool flag=false;
        for(int i=0;i<3;i++)
        {
            for(int j=0;j<3;j++)
            {
                e=mesh->edge(H0[i]);
                v4=mesh->vertex(e,0);
                v5=mesh->vertex(e,1);
                v0=mesh->point(v4);
                v1=mesh->point(v5);
                e=mesh->edge(H1[j]);
                v6=mesh->vertex(e,0);
                v7=mesh->vertex(e,1);
                v2=mesh->point(v6);
                v3=mesh->point(v7);
                if(CGAL::collinear(v0,v1,v3))
                {
                    h0=H0[i];
                    h1=H1[j];
                    flag=true;
                    break;
                }
            }
            if(flag)
                break;
        }
        //cout<<h0<<" "<<h1<<" "<<mesh->opposite(h1)<<endl;
        //cout<<v4<<v5<<v6<<v7<<endl;
        if(CGAL::squared_distance(v0,v1)<CGAL::squared_distance(v2,v3))
        {
            h=h1;
            if((v3.x()<v0.x()<v2.x())||(v2.x()<v0.x()<v3.x()))
            {
                v=v4;
            }
            else
                v=v5;
        }
        else
        {
            h=h0;
            //mesh.set_face(h1,mesh.null_face());
            if((v0.x()<v3.x()<v1.x())||(v1.x()<v3.x()<v0.x()))
            {
                v=v6;
            }
            else
                v=v7;
        }
        //cout<<h<<" "<<v<<" "<<h2<<endl;
        f=mesh->face(h);
        v4=mesh->vertex(mesh->edge(h),0);
        v5=mesh->vertex(mesh->edge(h),1);
        v6=mesh->vertex(mesh->edge(mesh->next(h)),1);
        if(!mesh->is_removed(f))mesh->remove_face(f);
        //cout<<v4<<v5<<v6<<endl;
        repairfaces.push_back(v4);
        repairfaces.push_back(v);
        repairfaces.push_back(v6);
        repairfaces.push_back(v5);
        repairfaces.push_back(v);
        repairfaces.push_back(v6);
        mesh->collect_garbage();

//        for(uint i=0;i<intersected_tris.size();i++)
//        {
//            Mesh::Face_index f0=intersected_tris[i].first;
//            Mesh::Face_index f1=intersected_tris[i].second;
//            mesh->remove_face(f0);
//            mesh->remove_face(f1);
//        }
//        mesh->collect_garbage();

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

//        for(uint i=0;i<repairfaces.size()/3;i++)
//        {
//            Mesh::Vertex_index v0(repairfaces[3*i+0]);
//            Mesh::Vertex_index v1(repairfaces[3*i+1]);
//            Mesh::Vertex_index v2(repairfaces[3*i+2]);
//            cout<<v0<<" "<<v1<<" "<<v2<<endl;
//            if(mesh->add_face(v0,v1,v2)==mesh->null_face())
//            {
//                if(mesh->add_face(v0,v2,v1)==mesh->null_face())
//                    cout<<"can't add face!"<<endl;
//            }
//        }
        intersecting = CGAL::Polygon_mesh_processing::does_self_intersect(*mesh,
                                                CGAL::Polygon_mesh_processing::parameters::vertex_point_map(get(CGAL::vertex_point, *mesh)));
        //cout<< (intersecting ? "There are self-intersections." : "There is no self-intersection.")<<endl;
    }
    cout << "repairing selfintersection done : " <<endl;
    cout << "\t Number of vertices  :\t" << mesh->number_of_vertices()<<endl;
    cout << "\t Number of halfedges :\t" << mesh->number_of_halfedges() <<endl;
    cout << "\t Number of facets    :\t" << mesh->number_of_faces()<<endl;
}
