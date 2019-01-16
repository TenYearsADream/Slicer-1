#ifndef SLICE_H
#define SLICE_H
#include <QString>
#include <QTime>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <boost/any.hpp>
#include "opencl.h"
#include "dataset.h"
using namespace std;
class Slice
{
public:
    Slice();
    ~Slice();
    float thick;
    size_t layernumber;
    bool isAdapt;
    bool isParaComp;
public:
    void startSlice(vector<EdgeNode> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints);

private:
    QTime time;

    Lines lines;
    Polylines polylines;

    float zheight;
    vector<float>z;

    int findtime,comptime,sorttime;
    OpenCL opencl;

private:
    void sliceByHeight(Mesh mesh,float zmin,float zmax,vector<Polylines> &intrpoints);
    void sliceByCpu(vector<EdgeNode> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints);
    void sliceByGpu(vector<EdgeNode> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints);
    bool genSlicesFile(const QString& fileName,const vector<Polylines> intrpoints,float surroundBox[6]);
};
#endif // SLICE_H
