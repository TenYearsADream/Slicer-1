#ifndef DATASET_H
#define DATASET_H
#include <vector>
#include <QObject>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
using namespace std;
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Kernel::Point_3 Point;

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
        vector<unsigned short> indices;
        vector<float> vertices;

        vector<Vertex> vertexset;
        multimap<float,Edge>edgeset;
        vector<Face> faceset;
        vector<float> halfedge;
private:

};

#endif // DATASET_H
