#ifndef DATASET_H
#define DATASET_H
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CL/cl_platform.h>
#include <QString>
using namespace std;
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Kernel::Point_3 Point;
typedef Kernel::Vector_3 Vector;
typedef Kernel::Segment_3 Segment;
typedef vector<Point> Lines;
typedef vector<Lines> Polylines;
typedef unsigned int uint;
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

class dataSet
{
public:
        dataSet();
        ~dataSet();
        void rotateModel(int x,int y,int z);
        void getIndices();
        void halfedgeOnGpu();
        void exportSTL(const QString stlfileName);
public:
        Mesh mesh;
        float surroundBox[6];
        vector<uint> indices;
        vector<float> vertices;
        vector<float> vertexnormals;

        vector<cl_float3> vertexset;
        vector<cl_uint3>halfedgeset;
        multimap<float,Edge>edgeset;
        vector<cl_uint3> faceset;
private:
        void computeVertexnormals();
};

#endif // DATASET_H
