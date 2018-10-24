#include "shapediameterfunction.h"
#include <boost/foreach.hpp>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
#include <CGAL/boost/graph/Face_filtered_graph.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/mesh_segmentation.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Kernel::Point_3 Point;
typedef Mesh::Vertex_index vertex_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
typedef Mesh::Property_map<face_descriptor,double> Facet_double_map;
typedef Mesh::Property_map<face_descriptor, size_t> Facet_int_map;
typedef CGAL::Face_filtered_graph<Mesh> Filtered_graph;
using namespace std;
ShapeDiameterFunction::ShapeDiameterFunction(){
    //cout<<"ShapeDiameterFunction"<<endl;
}
ShapeDiameterFunction::~ShapeDiameterFunction(){
    //cout<<"ShapeDiameterFunction"<<endl;
}

vector<vector<double>> ShapeDiameterFunction::calculateSDF(Mesh mesh)
{
    //Mesh mesh=constructMesh(vertices,faceList);
    Facet_double_map sdf_property_map = mesh.add_property_map<face_descriptor,double>("f:sdf").first;
    pair<double, double> min_max_sdf=CGAL::sdf_values(mesh, sdf_property_map);
    //cout<< "minimum SDF: " << min_max_sdf.first<< " maximum SDF: " << min_max_sdf.second <<endl;
    // print SDF values
    vector<vector<double>> chardata;
    vector<double> value(2);
    //charvalue = new vector<sdfValue>[12];
    for(int i=0;i< mesh.number_of_faces();i++)
    {
        face_descriptor fd(i);
//        cout << "vertices around face " << fd <<":";
//        CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
//        for(boost::tie(vbegin, vend) = vertices_around_face(mesh.halfedge(fd), mesh);
//            vbegin != vend;
//            ++vbegin){
//          cout << *vbegin <<" ";
//        }
//        cout<<endl;
        double sdfvalue=sdf_property_map[fd];
        double facearea=CGAL::Polygon_mesh_processing::face_area(fd,mesh);
        //cout << sdfvalue<<" "<<facearea <<endl;
        value[0]=sdfvalue;value[1]=facearea;
        //charvalue[i].push_back(sdfValue(sdfvalue,facearea));
        chardata.push_back(value);
    }
    //cout<<charvalue.size()<<endl;
    return normalize(chardata);
}

vector<vector<double>> ShapeDiameterFunction::normalize(vector<vector<double> > dataset)
{
    vector<double> featuredata;
    for(int i=0;i<dataset.size();i++)
    {
        featuredata.push_back(dataset[i][1]);
    }
    double maxValue=*max_element(featuredata.begin(),featuredata.end());
    double minValue=*min_element(featuredata.begin(),featuredata.end());
    for(int i=0;i<featuredata.size();i++)
    {
        featuredata[i]=(featuredata[i]-minValue)/(maxValue-minValue+1e-8);
        dataset[i][1]=featuredata[i];
    }
    return dataset;
}
