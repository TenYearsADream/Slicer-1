#include "shapediameterfunction.h"
#include <boost/foreach.hpp>

#include <CGAL/Surface_mesh.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
#include <CGAL/boost/graph/Face_filtered_graph.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/boost/graph/copy_face_graph.h>

//#include <CGAL/mesh_segmentation.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Kernel::Point_3 Point;
typedef Mesh::Vertex_index vertex_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
Mesh m;

using namespace std;

ShapeDiameterFunction::ShapeDiameterFunction(){
    cout<<"ShapeDiameterFunction"<<endl;
}
ShapeDiameterFunction::~ShapeDiameterFunction(){
    cout<<"ShapeDiameterFunction"<<endl;
}

void ShapeDiameterFunction::ConstructMesh(vector<Point3f> pointList, int nFaceCount){
    // Add the points as vertices
    vertex_descriptor v0,v1,v2;
    for (int i=0;i<nFaceCount;i++)
    {
        v0 = m.add_vertex(Point(pointList.at(3*i+0).x,pointList.at(3*i+0).y,pointList.at(3*i+0).z));
        v1 = m.add_vertex(Point(pointList.at(3*i+1).x,pointList.at(3*i+1).y,pointList.at(3*i+1).z));
        v2 = m.add_vertex(Point(pointList.at(3*i+2).x,pointList.at(3*i+2).y,pointList.at(3*i+2).z));
        m.add_face(v0, v1, v2);
    }
    {
      cout << "vertices around vertex " << v0 << endl;
      CGAL::Vertex_around_target_circulator<Mesh> vbegin(m.halfedge(v0),m), done(vbegin);
      do {
        cout << *vbegin++ << endl;
      } while(vbegin != done);
    }
}

void ShapeDiameterFunction::show(){
    cout<<"number of vertices:"<<m.vertices().size()<<endl;
    cout<<"number of faces:"<<m.faces().size()<<endl;
}

