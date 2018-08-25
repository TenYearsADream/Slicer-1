#ifndef SLICE_H
#define SLICE_H
#include <vector>
#include "hashtable.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Kernel::Point_3 Point;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
struct intersectFace
{
    Mesh::Face_index Faceindex;
    bool isSliced;
    bool isParallel;
    intersectFace(Mesh::Face_index faceindex,bool issliced,bool isparallel)
    {
        Faceindex=faceindex;
        isSliced=issliced;
        isParallel=isparallel;
    }
};
struct sliceData
{
    vector<vector<Point>> Points;
    sliceData(vector<vector<Point>> point)
    {
        Points=point;
    }
};

class Slice
{
public:
    Slice();
    ~Slice();
    Mesh mesh;
    vector<sliceData> intrpoints;
    double thick;
    int layernumber;
    bool isAdapt;
public:
    void intrPoints(double zmin,double zmax);
private:
    vector<intersectFace> intrsurfs;
    vector<float>normalangle;
    double adaptthick;
private:
    void intrSurfs(double zheight);
    bool isIntr(CGAL::Halfedge_around_face_iterator<Mesh> e,double zheight);
    Point intersectPoint(CGAL::Halfedge_around_face_iterator<Mesh> e,double z);
    float normalAngle(Mesh::Face_index f0);
    vector<vector<Point>> areaSort(vector<vector<Point>> points);
};

#endif // SLICE_H
