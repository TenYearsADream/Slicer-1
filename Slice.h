#ifndef SLICE_H
#define SLICE_H
#include <vector>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CL/cl.h>
using namespace std;
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
    vector<sliceData> intrpoints;
    double thick;
    int layernumber;
    bool isAdapt;
    bool isParaComp;
public:
    void startSlice(Mesh mesh,double zmin,double zmax);

private:
    vector<intersectFace> intrsurfs;
    vector<float>normalangle;
    vector<Mesh::halfedge_index> intredges;
    vector<vector<Point>> points;
    vector<int> loopNum;
    double adaptthick;
    float zheight;

    float *interSection1,*interSection2;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel cap;
private:
    void intrSurfs(Mesh mesh);
    void findEdge(Mesh mesh,Mesh::face_index fbegin);
    bool isIntr(Mesh mesh,Mesh::halfedge_index edge);
    Point intersectPoint(Mesh mesh,Mesh::halfedge_index edge);
    float normalAngle(Mesh mesh,Mesh::Face_index f0);
    vector<vector<Point>> areaSort(vector<vector<Point>> points);
    void parallelPoint(Mesh mesh,Mesh::Face_index fbegin);
    void initOpencl();
    cl_device_id create_device();
    cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename);
    void executeKernel(float *interSection1,float*interSection2,float *buf,int number);
    void setBuffer(Mesh mesh,int BUFSIZE);
};

#endif // SLICE_H
