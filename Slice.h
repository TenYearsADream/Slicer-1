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
    int layernumber;
    bool isAdapt;
    bool isParaComp;
public:
    void startSlice(Mesh mesh,double zmin,double zmax);

private:
    Lines lines;
    QTime time;
    Polylines polylines;
    Intredges intredges;
    vector<Intredges> sliceedges;
    float zheight;
    vector<float>z;
    int linesnumber;
    Mesh::Property_map<Mesh::face_index,int>isSliced;

    int findtime,comptime;

    OpenCL opencl;

private:
    float adaptSlice(Mesh mesh,Intredges intredges);

};

#endif // SLICE_H
