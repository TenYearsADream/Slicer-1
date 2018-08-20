#include "dataset.h"
typedef Mesh::Vertex_index vertex_descriptor;
typedef Kernel::Point_3 Point;
dataSet::dataSet(vector <tableNode *> points,vector<vector<size_t>> faceList)
{
    constructMesh(points,faceList);
    vector<vector<int>> index;
    int j=0;
    for (int i=0;i<points.size();i++)
    {
        tableNode *vertex =points[i];
        if(vertex!=NULL)
        {
            vector<int> tmp(2);
            tmp[0]=j;tmp[1]=i;
            index.push_back(tmp);
            vertices.push_back(vertex->point.x);
            vertices.push_back(vertex->point.y);
            vertices.push_back(vertex->point.z);
            j++;
        }
    }
    for (int i=0;i<faceList.size();i++)
    {
        for (int j=0;j<3;j++)
        {
            for(int k=0;k<index.size();k++)
            {
                if(index[k][1]==faceList[i][j])
                {
                    indices.push_back(index[k][0]);
                }
            }
        }
    }
}

dataSet::~dataSet()
{

}

void dataSet::constructMesh(vector <tableNode *> vertices,vector<vector<size_t>> faceList)
{
    mesh.clear();
    vector<vector<int>> index;
    int j=0;
    for (int i=0;i<vertices.size();i++)
    {
        tableNode *vertex = vertices[i];
        if(vertex!=NULL)
        {
            vector<int> tmp(2);
            tmp[0]=j;tmp[1]=i;
            index.push_back(tmp);
            vertex_descriptor v0=mesh.add_vertex(Point(vertex->point.x,vertex->point.y,vertex->point.z));
            j++;
        }
    }
    //cout<<"number of vertices:"<<mesh.vertices().size()<<endl;
    for(int i=0;i<faceList.size();i++)
    {
        vertex_descriptor vx(getIndex(index,faceList[i][0]));
        vertex_descriptor vy(getIndex(index,faceList[i][1]));
        vertex_descriptor vz(getIndex(index,faceList[i][2]));
        //cout<<vx<<" "<<vy<<" "<<vz<<endl;
        //mesh.add_edge(vx,vy);mesh.add_edge(vy,vz);mesh.add_edge(vz,vx);
        mesh.add_face(vx,vy,vz);
    }
    //cout<<"number of faces:"<<mesh.faces().size()<<endl;
}

int dataSet::getIndex(vector<vector<int> > index, int ID)
{
    for(int i=0;i<index.size();i++)
    {
        if(index[i][1]==ID)
        {
            return index[i][0];
        }
    }
    return 0;
}
