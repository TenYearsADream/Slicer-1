#ifndef OPENCL_H
#define OPENCL_H
#include <CL/cl.hpp>
#include <vector>
using namespace std;
struct DeviceInfo{
    string deviceName;
    int maxComputeUnits;
    int maxWorkItemPerGroup;
    int maxConstantBufferSize;
    int maxGlobalMemSize;
    int maxLocalMemSize;
    int maxMemAllocSize;
};


class OpenCL
{
public:
    OpenCL();
    ~OpenCL();
    void executeKernel(float *interSection1,float *interSection2,float *result,int lineNum,float zheight);
    void executeKernel(cl::Buffer vertexbuf,cl::Buffer halfedgebuf,vector<int> &buf,float z0,float thick,size_t LINESNUMBER);
    void executeKernel(cl::Buffer vertexbuf,cl::Buffer halfedgebuf,vector<unsigned int>edgeset,vector<cl_float3>&result,size_t total,size_t LAYERNUMBER,float *zheight,vector<unsigned int>linesnumber);
    bool executeKernel(vector<cl_uint2> faceset,vector<unsigned int>linesnumber,vector<cl_int3>&hashTable,unsigned int layernumber,
                       vector<unsigned int>&location,vector<unsigned int>&loopcount,vector<unsigned int>&loopnumber);
    DeviceInfo deviceinfo;
public:
    cl::CommandQueue queue;
    cl::Kernel capbyheight,calalledges,groupedge,intersect,hashfind;
    cl::Context context;

private:
    cl::Event profileEvent;
};

#endif // OPENCL_H
