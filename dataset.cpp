#include "dataset.h"
#include <QMatrix4x4>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
dataSet::dataSet()
{
    mesh.clear();
}

dataSet::~dataSet()
{

}

void dataSet::getIndices()
{
    vertexnormals.clear();
    if(CGAL::is_closed(mesh))
    {
        computeVertexnormals();
    }
    vertices.clear();
    indices.clear();
    vertices.reserve(3*mesh.number_of_vertices());
    indices.reserve(3*mesh.number_of_faces());
    vector<float>X,Y,Z;
    X.reserve(mesh.number_of_vertices());
    Y.reserve(mesh.number_of_vertices());
    Z.reserve(mesh.number_of_vertices());
    for(Mesh::Vertex_index v:mesh.vertices())
    {
        //cout<<point.x()<<" "<<point.y()<<" "<<point.z()<<endl;
        vertices.push_back(float(mesh.point(v).x()));
        vertices.push_back(float(mesh.point(v).y()));
        vertices.push_back(float(mesh.point(v).z()));
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
    for(Mesh::Face_index f:mesh.faces())
    {
        CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
        for(boost::tie(vbegin, vend) = vertices_around_face(mesh.halfedge(f), mesh);vbegin != vend;++vbegin)
        {
            indices.push_back(uint(*vbegin));
            //cout<<ushort(*vbegin)<<" ";
        }
        //cout<<endl;
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
//    for(Mesh::Vertex_index vi:mesh.vertices())
//    {
//        Vertex v;
//        v.point=mesh.point(vi);
//        vertexset.push_back(v);
//    }

//    for(Mesh::Face_index fi:mesh.faces())
//    {
//        Mesh::Halfedge_index e0,e1,e2;
//        e0=mesh.halfedge(fi);
//        e1=mesh.next(e0);
//        e2=mesh.next(e1);
//        //cout<<fi<<" "<<e0<<" "<<e1<<" "<<e2<<endl;
//        Face f;
//        f.e0=e0;
//        f.e1=e1;
//        f.e2=e2;
//        faceset.push_back(f);
//    }
//    for(Mesh::Halfedge_index hi:mesh.halfedges())
//    {
//        Mesh::Halfedge_index hpre=mesh.prev(hi);
//        Mesh::Halfedge_index hnext=mesh.next(hi);
//        Mesh::Halfedge_index hopposite=mesh.opposite(hi);
//        Mesh::Vertex_index vertex0=mesh.vertex(mesh.edge(hi),0);
//        Mesh::Vertex_index vertex1=mesh.vertex(mesh.edge(hi),1);
//        Mesh::Face_index f=mesh.face(hi);
//        Point p1=mesh.point(vertex0);
//        Point p2=mesh.point(vertex1);
//        float zmin=float(qMin(p1.z(),p2.z()));
//        float zmax=float(qMax(p1.z(),p2.z()));
//        //cout<<zmax<<" "<<zmin<<" "<<hi<<" "<<vertex0<<" "<<vertex1<<" "<<hpre<<" "<<hnext<<" "<<hopposite<<" "<<f<<endl;
//        Edge e={zmax,zmin,vertex0,vertex1,hpre,hnext,hopposite,hi,f};
//        edgeset.insert(make_pair(zmax,e));
//    }
    halfedge.clear();
    for(Mesh::Halfedge_index hi:mesh.halfedges())
    {
        Mesh::Vertex_index vertex0=mesh.vertex(mesh.edge(hi),0);
        Mesh::Vertex_index vertex1=mesh.vertex(mesh.edge(hi),1);
        Mesh::Face_index f=mesh.face(hi);
        Point p1=mesh.point(vertex0);
        Point p2=mesh.point(vertex1);
        halfedge.push_back(EdgeNode(float(p1.x()),float(p1.y()),float(p1.z()),float(p2.x()),float(p2.y()),float(p2.z()),f));
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
