#ifndef SLICE_H
#define SLICE_H
#include <vector>
#include <QString>
#include <QTime>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CL/cl.h>
#include <boost/any.hpp>
#include "opencl.h"
#include "dataset.h"
using namespace std;
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Kernel::Point_3 Point;
typedef Kernel::Segment_3 Segment;
typedef vector<Point> Lines;
typedef vector<Lines> Polylines;

class Slice
{
public:
    Slice();
    ~Slice();
    vector<Polylines> intrpoints;
    float thick;
    size_t layernumber;
    bool isAdapt;
    bool isParaComp;
public:
    void startSlice(Mesh mesh,vector<float> halfedge,float zmin,float zmax);

private:
    QTime time;

    Lines lines;
    Polylines polylines;

    float zheight;
    vector<float>z;

    int findtime,comptime,sorttime;
    OpenCL opencl;

private:
    void sliceByHeight(Mesh mesh,float zmin,float zmax);
    void sliceByEdge(Mesh mesh,float zmin,float zmax);
    void sliceByCpu(vector<float> halfedge,float zmin,float zmax);
    void sliceByGpu(vector<float> halfedge,float zmin,float zmax);
};

#endif // SLICE_H
