#ifndef SLICE_H
#define SLICE_H
#include <QString>
#include <QTime>
#include <boost/any.hpp>
#include "dataset.h"
#include "opencl.h"
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
    void startSlice(vector<cl_float3> &vertex,vector<cl_uint3> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints);

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
    void sliceByCpu(vector<cl_float3> &vertex,vector<cl_uint3> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints);
    void sliceByGpu(vector<cl_float3> &vertex,vector<cl_uint3> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints);
    bool genSlicesFile(const QString& fileName,const vector<Polylines> intrpoints,float surroundBox[6]);

    void sliceOnGpu(vector<cl_float3> &vertex,vector<cl_uint3> &halfedge,float surroundBox[6],vector<Polylines> &intrpoints);
};
#endif // SLICE_H
