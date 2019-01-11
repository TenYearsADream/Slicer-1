#ifndef DATASET_H
#define DATASET_H
#include <vector>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
using namespace std;
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Kernel::Point_3 Point;
typedef Kernel::Vector_3 Vector;
typedef Kernel::Segment_3 Segment;
typedef vector<Point> Lines;
typedef vector<Lines> Polylines;
typedef unsigned int uint;

struct Vertex{
    Point point;
    uint edgeindex;
};

struct Edge{
    float zmax;
    float zmin;
    uint v0;
    uint v1;
    uint pre;
    uint next;
    uint opposite;
    uint id;
    uint f;
};

struct Face{
    uint e0;
    uint e1;
    uint e2;
};

struct EdgeNode{
    float x1;
    float y1;
    float z1;
    float x2;
    float y2;
    float z2;
    unsigned int f;
    EdgeNode(float _x1,float _y1,float _z1,float _x2,float _y2,float _z2,unsigned int _f)
    {
        x1=_x1;
        y1=_y1;
        z1=_z1;
        x2=_x2;
        y2=_y2;
        z2=_z2;
        f=_f;
    }
};

class dataSet
{
public:
        dataSet();
        ~dataSet();
        void rotateModel(int x,int y,int z);
        void getIndices();
        void halfedgeOnGpu();
public:
        Mesh mesh;
        float surroundBox[6];
        vector<uint> indices;
        vector<float> vertices;
        vector<float> vertexnormals;

        vector<Vertex> vertexset;
        multimap<float,Edge>edgeset;
        vector<Face> faceset;
        vector<EdgeNode> halfedge;
private:
        void computeVertexnormals();
};

#endif // DATASET_H
