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
