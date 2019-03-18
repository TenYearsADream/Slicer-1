#include "dataset.h"
#include <QMatrix4x4>
#include <QTime>
#include <QFile>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>

// Simplification function
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
// Stop-condition policy
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>


dataSet::dataSet()
{
    mesh.clear();
}

dataSet::~dataSet()
{

}

void dataSet::getIndices()
{
    vector<float>X,Y,Z;
    X.reserve(mesh.number_of_vertices());
    Y.reserve(mesh.number_of_vertices());
    Z.reserve(mesh.number_of_vertices());
    for(Mesh::Vertex_index v:mesh.vertices())
    {
        X.push_back(float(mesh.point(v).x()));
        Y.push_back(float(mesh.point(v).y()));
        Z.push_back(float(mesh.point(v).z()));
    }
    surroundBox[0]=*min_element(X.begin(),X.end());
    surroundBox[1]=*max_element(X.begin(),X.end());
    surroundBox[2]=*min_element(Y.begin(),Y.end());
    surroundBox[3]=*max_element(Y.begin(),Y.end());
    surroundBox[4]=*min_element(Z.begin(),Z.end());
    surroundBox[5]=*max_element(Z.begin(),Z.end());

    Mesh meshforGL(mesh);
    bool success;
//    cout<<meshforGL.number_of_faces()<<endl;
    // This is a stop predicate (defines when the algorithm terminates).
    // In this example, the simplification stops when the number of undirected edges
    // left in the surface mesh drops below the specified number (50000)
    QTime time;
    time.start();
    CGAL::Surface_mesh_simplification::Count_stop_predicate<Mesh> stop(10000);
    try
    {
        int r=CGAL::Surface_mesh_simplification::edge_collapse
                (meshforGL,stop
                 ,CGAL::parameters::vertex_index_map(get(CGAL::vertex_point,meshforGL))
                                   .halfedge_index_map  (get(CGAL::halfedge_index  ,meshforGL))
                                   .get_cost (CGAL::Surface_mesh_simplification::Edge_length_cost <Mesh>())
                                   .get_placement(CGAL::Surface_mesh_simplification::Midpoint_placement<Mesh>())
                );
        cout << "Finished in "<<time.elapsed()<<"ms. " << r << " edges removed.  "<<meshforGL.number_of_halfedges()/2<< " final edges."<<endl;
        if(meshforGL.number_of_halfedges()/2>10000)
        {
            success=false;
        }
        else
        {
            success=true;
        }

    }
    catch(exception& e)
    {
        cout<<"can't simplify mesh."<<e.what()<<endl;
        success=false;
    }
    if(success)
    {
        vertexnormals.clear();
        if(CGAL::is_closed(meshforGL))
        {
            computeVertexnormals();
        }
        vertices.clear();
        indices.clear();
        vertices.reserve(3*meshforGL.number_of_vertices());
        indices.reserve(3*meshforGL.number_of_faces());
        for(Mesh::Vertex_index v:meshforGL.vertices())
        {
            vertices.push_back(float(meshforGL.point(v).x()));
            vertices.push_back(float(meshforGL.point(v).y()));
            vertices.push_back(float(meshforGL.point(v).z()));
        }
        for(Mesh::Face_index f:meshforGL.faces())
        {
            CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
            for(boost::tie(vbegin, vend) = vertices_around_face(meshforGL.halfedge(f), meshforGL);vbegin != vend;++vbegin)
            {
                indices.push_back(uint(*vbegin));
    //            cout<<ushort(*vbegin)<<" ";
            }
    //        cout<<endl;
        }
    }
}

void dataSet::rotateModel(int x,int y,int z)
{
    //cout<<x<<" "<<y<<" "<<z<<endl;
    QMatrix4x4 rotateMatrix;
    QQuaternion rotation;
    rotation = QQuaternion::fromAxisAndAngle(QVector3D(1.0f,0.0f,0.0f),x) * rotation;
    rotation = QQuaternion::fromAxisAndAngle(QVector3D(0.0f,1.0f,0.0f),y) * rotation;
    rotation = QQuaternion::fromAxisAndAngle(QVector3D(0.0f,0.0f,1.0f),z) * rotation;
    rotateMatrix.rotate(rotation);
//    for(int i=0;i<4;i++)
//    {
//        cout<<rotateMatrix.row(i).x()<<" ";
//        cout<<rotateMatrix.row(i).y()<<" ";
//        cout<<rotateMatrix.row(i).z()<<" ";
//        cout<<rotateMatrix.row(i).w()<<" ";
//        cout<<endl;
//    }
    vector<Point> points;
    for(size_t i=0;i<mesh.number_of_vertices();i++)
    {
        Mesh::Vertex_index v=Mesh::Vertex_index(Mesh::size_type(i));
        //cout<<v<<" "<<mesh.point(v)<<endl;
        QVector4D point=QVector4D(float(mesh.point(v).x()),float(mesh.point(v).y()),float(mesh.point(v).z()),1.0f);
        point=rotateMatrix*point;
        //cout<<point.x()<<" "<<point.y()<<" "<<point.z()<<endl;
        points.push_back(Point(point.x(),point.y(),point.z()));
    }
    mesh.clear();
    vertices.clear();
    vector<float>X,Y,Z;
    for(size_t i=0;i<points.size();i++)
    {
        vertices.push_back(float(points[i].x()));
        vertices.push_back(float(points[i].y()));
        vertices.push_back(float(points[i].z()));
        X.push_back(float(points[i].x()));
        Y.push_back(float(points[i].y()));
        Z.push_back(float(points[i].z()));
        mesh.add_vertex(points[i]);
    }
    surroundBox[0]=*min_element(X.begin(),X.end());
    surroundBox[1]=*max_element(X.begin(),X.end());
    surroundBox[2]=*min_element(Y.begin(),Y.end());
    surroundBox[3]=*max_element(Y.begin(),Y.end());
    surroundBox[4]=*min_element(Z.begin(),Z.end());
    surroundBox[5]=*max_element(Z.begin(),Z.end());
    for(size_t i=0;i<indices.size();i +=3)
    {
        //cout<<indices[i]<<" "<<indices[i+1]<<" "<<indices[i+2]<<endl;
        Mesh::Vertex_index v0=Mesh::Vertex_index(Mesh::size_type(indices[i]));
        Mesh::Vertex_index v1=Mesh::Vertex_index(Mesh::size_type(indices[i+1]));
        Mesh::Vertex_index v2=Mesh::Vertex_index(Mesh::size_type(indices[i+2]));
        mesh.add_face(v0,v1,v2);
    }
}

void dataSet::halfedgeOnGpu()
{
    vertexset.clear();
    vertexset.reserve(mesh.number_of_vertices());
    cl_float3 v;
    for(Mesh::Vertex_index vi:mesh.vertices())
    {
        v.x=float(mesh.point(vi).x());
        v.y=float(mesh.point(vi).y());
        v.z=float(mesh.point(vi).z());
        vertexset.push_back(v);
    }
    halfedgeset.clear();
    halfedgeset.reserve(mesh.number_of_halfedges());
    cl_uint3 h;
    for(Mesh::Halfedge_index hi:mesh.halfedges())
    {
        Mesh::Vertex_index v0=mesh.vertex(mesh.edge(hi),0);
        Mesh::Vertex_index v1=mesh.vertex(mesh.edge(hi),1);
        Mesh::Face_index f=mesh.face(hi);
//        if(!f.is_valid())
//        {
//            cout<<f<<endl;
//        }
        h.x=v0;
        h.y=v1;
        h.z=f;
        halfedgeset.push_back(h);
    }
}

void dataSet::computeVertexnormals()
{
    vertexnormals.reserve(3*mesh.num_vertices());
    auto vnormals = mesh.add_property_map<Mesh::Vertex_index, Vector>("v:normals", CGAL::NULL_VECTOR).first;
    CGAL::Polygon_mesh_processing::compute_vertex_normals(mesh,vnormals,
          CGAL::Polygon_mesh_processing::parameters::vertex_point_map(mesh.points()).geom_traits(Kernel()));
    //std::cout << "Vertex normals :" << std::endl;
    for(Mesh::Vertex_index vd: mesh.vertices()){
        //std::cout << vnormals[vd] << std::endl;
        vertexnormals.push_back(float(vnormals[vd].x()));
        vertexnormals.push_back(float(vnormals[vd].y()));
        vertexnormals.push_back(float(vnormals[vd].z()));
    }
}

void dataSet::exportSTL(const QString stlfileName)
{
    QFile file(stlfileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Can't open file for writing";
    }
    char name[80]="1234";
    char attribute[2]="w";
    file.write(name,sizeof(name));
    uint facesnumber=mesh.number_of_faces();
    float x,y,z;
    file.write((char*)(&facesnumber),4);
    auto fnormals =mesh.add_property_map<Mesh::Face_index, Vector>("f:normals", CGAL::NULL_VECTOR).first;
    CGAL::Polygon_mesh_processing::compute_face_normals(mesh,fnormals,
           CGAL::Polygon_mesh_processing::parameters::vertex_point_map(mesh.points()).geom_traits(Kernel()));
    for(Mesh::Face_index f:mesh.faces())
    {
        //cout<<f<<"--"<<endl;
        //cout<<fnormals[f]<<endl;
        x=fnormals[f].x();y=fnormals[f].y();z=fnormals[f].z();
        file.write((char*)&x,4);
        file.write((char*)&y,4);
        file.write((char*)&z,4);
        BOOST_FOREACH(Mesh::Vertex_index vd,vertices_around_face(mesh.halfedge(f),mesh))
        {
           //cout <<dataset.mesh.point(vd)<<endl;
           x=mesh.point(vd).x();y=mesh.point(vd).y();z=mesh.point(vd).z();
           file.write((char*)&x,4);
           file.write((char*)&y,4);
           file.write((char*)&z,4);
         }
        file.write(attribute,2);
    }
    file.close();
}
