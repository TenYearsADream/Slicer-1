#ifndef OPENCL_H
#define OPENCL_H
#include <CL/cl.hpp>
#include <vector>
using namespace std;
class OpenCL
{
public:
    OpenCL();
    ~OpenCL();
    void executeKernel(float *interSection1,float *interSection2,float *result,int lineNum,float zheight);
    void executeKernel(float *interSection1,float *interSection2,float *result,size_t LAYERNUMBER,size_t LINESNUMBER ,float *zheight);
    void executeKernel(vector<float>halfedge,vector<int> &buf,float z0,float thick,size_t LINESNUMBER);
public:
    cl::CommandQueue queue;
    cl::Kernel capbyheight,capbyedge,groupedge,groupedge2;
    cl::Context context;

private:
    cl::Event profileEvent;
};

#endif // OPENCL_H
