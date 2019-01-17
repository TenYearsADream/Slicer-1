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
    void executeKernel(cl::Buffer halfedgebuf,vector<int> &buf,float z0,float thick,size_t LINESNUMBER);
    void executeKernel(cl::Buffer halfedgebuf,cl::Buffer edgebuf,cl::Buffer resultbuf,size_t total,size_t LAYERNUMBER,float *zheight,vector<unsigned int>linesnumber);
public:
    cl::CommandQueue queue;
    cl::Kernel capbyheight,calalledges,groupedge;
    cl::Context context;

private:
    cl::Event profileEvent;
};

#endif // OPENCL_H
