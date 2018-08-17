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
    face_descriptor fd;
    bool isSliced;
    bool isParallel;
    intersectFace(){
        isSliced=false;
    };
    intersectFace(face_descriptor facedescriptor,bool issliced,bool isparallel)
    {
        fd=facedescriptor;
        isSliced=issliced;
        isParallel=isparallel;
    }
};

class Slice
{
public:
    Slice();
    ~Slice();
    Mesh mesh;
    vector<vector<Point>> intrpoints;
    void intrPoints(double zmin,double zmax);
    double thick;
    int layernumber;
private:
    vector<intersectFace> intrsurfs;
    void intrSurfs(double zheight);
    bool isIntr(CGAL::Halfedge_around_face_iterator<Mesh> e,double zheight);
    Point intersectPoint(CGAL::Halfedge_around_face_iterator<Mesh> e,double z);
};

#endif // SLICE_H
