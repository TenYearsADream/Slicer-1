#ifndef SLICE_H
#define SLICE_H
#include <vector>
#include <QString>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CL/cl.h>
#include <boost/any.hpp>
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
    vector<float>normalangle;
    Lines lines;
    Polylines polylines;
    Intredges intredges;
    vector<int> loopNum;
    double adaptthick;
    float zheight;
    Mesh::Property_map<Mesh::face_index,int>isSliced;

    float *interSection1,*interSection2;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel cap;
private:
    float normalAngle(Mesh mesh,Mesh::Face_index f0);
    void initOpencl();
    cl_device_id create_device();
    cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename);
    void executeKernel(float *interSection1,float*interSection2,float *buf,int number);
    void setBuffer(Mesh mesh,int BUFSIZE);
};

#endif // SLICE_H
