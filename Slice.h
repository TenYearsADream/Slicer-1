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
using namespace std;
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Kernel::Point_3 Point;
typedef Kernel::Segment_3 Segment;
typedef vector<Point> Lines;
typedef list<Lines> Polylines;

typedef vector<boost::any> Outline;
typedef list<Outline> Intredges;

class Slice
{
public:
    Slice();
    ~Slice();
    vector<Polylines> intrpoints;
    double thick;
    size_t layernumber;
    bool isAdapt;
    bool isParaComp;
public:
    void startSlice(Mesh mesh,double zmin,double zmax);

private:
    QTime time;

    Lines lines;
    Polylines polylines;
    Intredges intredges;

    float zheight;
    vector<float>z;
    unsigned long long linesnumber;

    int findtime,comptime,sorttime;
    OpenCL opencl;

private:
    float adaptSlice(Mesh mesh,Intredges intredges);
    void sliceByHeight(Mesh mesh,double zmin,double zmax);
    void sliceByEdge(Mesh mesh,double zmin,double zmax);
    void sliceByGpu(Mesh mesh,double zmin,double zmax);
};

#endif // SLICE_H
