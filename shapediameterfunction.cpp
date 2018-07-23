#include "shapediameterfunction.h"
#include <boost/foreach.hpp>

#include <CGAL/Surface_mesh.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
#include <CGAL/boost/graph/Face_filtered_graph.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/mesh_segmentation.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Kernel::Point_3 Point;
typedef Mesh::Vertex_index vertex_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
typedef Mesh::Property_map<face_descriptor,double> Facet_double_map;
typedef Mesh::Property_map<face_descriptor, size_t> Facet_int_map;
typedef CGAL::Face_filtered_graph<Mesh> Filtered_graph;
Mesh mesh;

using namespace std;

ShapeDiameterFunction::ShapeDiameterFunction(){
    cout<<"ShapeDiameterFunction"<<endl;
}
ShapeDiameterFunction::~ShapeDiameterFunction(){
    cout<<"ShapeDiameterFunction"<<endl;
}

vector<vector<double>> ShapeDiameterFunction::calculateSDF(vector <tableNode *> vertices, vector<vector<size_t>> faceList)
{
    vertex_descriptor v0,v1,v2;
    mesh.clear();
    for (int i=0;i<faceList.size();i++)
    {
        tableNode *vertex1 = vertices[faceList[i][0]];
        tableNode *vertex2 = vertices[faceList[i][1]];
        tableNode *vertex3 = vertices[faceList[i][2]];
        v0=mesh.add_vertex(Point(vertex1->point.x,vertex1->point.y,vertex1->point.z));
        v1=mesh.add_vertex(Point(vertex2->point.x,vertex2->point.y,vertex2->point.z));
        v2=mesh.add_vertex(Point(vertex3->point.x,vertex3->point.y,vertex3->point.z));
        mesh.add_face(v0,v1,v2);
    }
//    cout<<"number of vertices:"<<mesh.vertices().size()<<endl;
//    cout<<"number of faces:"<<mesh.faces().size()<<endl;


    Facet_double_map sdf_property_map = mesh.add_property_map<face_descriptor,double>("f:sdf").first;
    pair<double, double> min_max_sdf=CGAL::sdf_values(mesh, sdf_property_map);
    //cout<< "minimum SDF: " << min_max_sdf.first<< " maximum SDF: " << min_max_sdf.second <<endl;
    // print SDF values
    vector<vector<double>> charValue;
    vector<double> value(2);
    BOOST_FOREACH( face_descriptor fd, mesh.faces())
    {
        double sdfvalue=sdf_property_map[fd];
        double facearea=CGAL::Polygon_mesh_processing::face_area(fd,mesh);
        //cout << sdfvalue<<" "<<facearea <<endl;
        value[0]=sdfvalue;value[1]=facearea;
        charValue.push_back(value);
    }
    return charValue;
}
