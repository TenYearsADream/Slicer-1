#include "dataset.h"
#include <QMatrix4x4>
typedef Kernel::Point_3 Point;

dataSet::dataSet()
{
    mesh.clear();
}

dataSet::~dataSet()
{

}

void dataSet::constructMesh(vector <Point> vertices,vector<vector<int>> faceList)
{
    mesh.clear();
    for (int i=0;i<vertices.size();i++)
    {
        Mesh::Vertex_index v0=mesh.add_vertex(Point(vertices[i].x(),vertices[i].y(),vertices[i].z()));
    }
    //cout<<"number of vertices:"<<mesh.vertices().size()<<endl;
    for(int i=0;i<faceList.size();i++)
    {
        Mesh::Vertex_index vx(faceList[i][0]);
        Mesh::Vertex_index vy(faceList[i][1]);
        Mesh::Vertex_index vz(faceList[i][2]);
        //cout<<vx<<" "<<vy<<" "<<vz<<endl;
        mesh.add_face(vx,vy,vz);
    }
    //qDebug()<<"number of faces:"<<mesh.faces().size()<<endl;
}

void dataSet::getIndices()
{
    vertices.clear();
    indices.clear();
    vector<float>X,Y,Z;
    for(size_t i=0;i<mesh.number_of_vertices();i++)
    {
        Mesh::Vertex_index v=Mesh::Vertex_index(Mesh::size_type(i));
        vertices.push_back((float)mesh.point(v).x());
        vertices.push_back((float)mesh.point(v).y());
        vertices.push_back((float)mesh.point(v).z());
        X.push_back((float)mesh.point(v).x());
        Y.push_back((float)mesh.point(v).y());
        Z.push_back((float)mesh.point(v).z());
        //cout<<v<<":"<<mesh.point(v)<<endl;
    }
    surroundBox[0]=*min_element(X.begin(),X.end());
    surroundBox[1]=*max_element(X.begin(),X.end());
    surroundBox[2]=*min_element(Y.begin(),Y.end());
    surroundBox[3]=*max_element(Y.begin(),Y.end());
    surroundBox[4]=*min_element(Z.begin(),Z.end());
    surroundBox[5]=*max_element(Z.begin(),Z.end());
    for(size_t i=0;i<mesh.number_of_faces();i++)
    {
        Mesh::Face_index f=Mesh::Face_index(Mesh::size_type(i));
        //cout<<f<<endl;
        CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
        for(boost::tie(vbegin, vend) = vertices_around_face(mesh.halfedge(f), mesh);vbegin != vend;++vbegin)\
        {
            indices.push_back((unsigned short)(*vbegin));
            //cout<<unsigned short(*vbegin)<<" ";
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
        QVector4D point=QVector4D(mesh.point(v).x(),mesh.point(v).y(),mesh.point(v).z(),1.0f);
        point=rotateMatrix*point;
        //cout<<point.x()<<" "<<point.y()<<" "<<point.z()<<endl;
        points.push_back(Point(point.x(),point.y(),point.z()));
    }
    mesh.clear();
    vertices.clear();
    vector<float>X,Y,Z;
    for(int i=0;i<points.size();i++)
    {
        vertices.push_back(points[i].x());
        vertices.push_back(points[i].y());
        vertices.push_back(points[i].z());
        X.push_back(points[i].x());
        Y.push_back(points[i].y());
        Z.push_back(points[i].z());
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


