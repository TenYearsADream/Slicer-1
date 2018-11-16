#ifndef OPENCL_H
#define OPENCL_H
#include <CL/cl.h>

class OpenCL
{
public:
    OpenCL();
    ~OpenCL();
    void executeKernel(float *interSection1,float *interSection2,float *result,int lineNum,float zheight);
    void executeKernel(float *interSection1,float *interSection2,float *result,size_t LAYERNUMBER,size_t LINESNUMBER ,float *zheight);

private:
    void initOpencl();
    cl_device_id create_device();
    cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename);

private:
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel capbyheight,capbyedge;
};

#endif // OPENCL_H
