#include "meshfix.h"
#include <vector>
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/stitch_borders.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
using namespace std;

MeshFix::MeshFix(Mesh *_mesh)
{
    mesh=_mesh;
    fixConnectivity();
}
MeshFix::~MeshFix()
{

}

bool MeshFix::fixConnectivity()
{
    bool retval = true;
    int i=0;

    //去除孤立点
    if ((i = removeVertices()))
    {
        retval = false;
        cout<<i<<" isolated vertices have been removed."<<endl;
    }
    //缝合裂缝
    stitchBorders();
    //填补洞
    if ((i = holeFill()))
    {
        retval = false;
        cout <<i << " holes have been filled" <<endl;
    }

    //修复法向
    normalRepair();
    return retval;
}

int MeshFix::removeVertices()
{
    int r = 0;
    for(Mesh::vertex_index v:mesh->vertices())
    {
        if(mesh->is_isolated(v))
        {
            mesh->remove_vertex(v);
            r++;
        }
    }
    return r;
}

int MeshFix::holeFill()
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
        cout << " Fairing : " << (success ? "succeeded" : "failed") <<endl;
        ++nb_holes;
      }
    }
    return nb_holes;
}

void MeshFix::stitchBorders()
{
    cout << "Before stitching : " <<endl;
    cout << "\t Number of vertices  :\t" << mesh->num_vertices()<<endl;
    cout << "\t Number of halfedges :\t" << mesh->num_halfedges() <<endl;
    cout << "\t Number of facets    :\t" << mesh->num_faces()<<endl;
    CGAL::Polygon_mesh_processing::stitch_borders(*mesh);
    cout << "Stitching done : " <<endl;
    cout << "\t Number of vertices  :\t" << mesh->num_vertices()<<endl;
    cout << "\t Number of halfedges :\t" << mesh->num_halfedges() <<endl;
    cout << "\t Number of facets    :\t" << mesh->num_faces()<<endl;
}

void MeshFix::normalRepair()
{
    bool oriented=CGAL::Polygon_mesh_processing::is_outward_oriented(*mesh);
    if(!oriented)
    {
        cout<<"normal error!"<<endl;
    }
    else
        cout<<"normal right!"<<endl;
}
