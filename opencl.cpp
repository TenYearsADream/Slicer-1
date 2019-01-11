#include "opencl.h"
#include <fstream>
#include <iostream> 
using namespace std;
OpenCL::OpenCL()
{
    const char* PROGRAM_FILE="D:/QTAPP/Slicer/kernels.cl";
    const char* CAPBYHEIGHT ="capbyheight";
    const char* CALALLEDGES ="calalledges";
    const char* GROUPEDGE ="groupedge";
    const char* GROUPEDGE2 ="groupedge2";
    const char* CALLAYEREDGES ="callayeredges";

    vector<cl::Platform> platforms;
    vector<cl::Device> devices;
    cl::Event profileEvent;
    // Place the GPU devices of the first platform into a context
    cl::Platform::get(&platforms);
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    for(size_t i=0; i<devices.size(); i++)
    {
        string device_name = devices[i].getInfo<CL_DEVICE_NAME>();
        cout << "Device: " << device_name.c_str() << endl;
    }
    context=cl::Context(devices);

    // Create kernel
    std::ifstream programFile(PROGRAM_FILE);
    string programString(istreambuf_iterator<char>(programFile),(istreambuf_iterator<char>()));
    cl::Program::Sources source(1, make_pair(programString.c_str(),programString.length()+1));
    cl::Program program(context, source);
    program.build(devices);

    capbyheight=cl::Kernel(program, CAPBYHEIGHT);
    calalledges=cl::Kernel(program, CALALLEDGES);
    groupedge=cl::Kernel(program, GROUPEDGE);
    groupedge2=cl::Kernel(program, GROUPEDGE2);
    callayeredges=cl::Kernel(program, CALLAYEREDGES);
    /* Create a command queue */
    queue=cl::CommandQueue(context, devices[0],CL_QUEUE_PROFILING_ENABLE);

}

OpenCL::~OpenCL()
{
}

void OpenCL::executeKernel(float *interSection1,float *interSection2,float *result,int lineNum,float zheight)
{

    /* Create a buffer to hold data */
    cl::Buffer buf1(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,lineNum *3*sizeof(float),interSection1);
    cl::Buffer buf2(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,lineNum *3*sizeof(float),interSection2);
    cl::Buffer clz(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float),&zheight);
    cl::Buffer clbuf(context, CL_MEM_WRITE_ONLY,lineNum *3*sizeof(float), NULL);

    /* Create kernel argument */
    capbyheight.setArg(0,buf1);
    capbyheight.setArg(1,buf2);
    capbyheight.setArg(2,clbuf);
    capbyheight.setArg(3,clz);

    cl::NDRange globalSize(size_t(lineNum),3);
    queue.enqueueNDRangeKernel(capbyheight,cl::NullRange,globalSize,cl::NullRange,NULL,&profileEvent);
    queue.finish();
    queue.enqueueReadBuffer(clbuf,CL_TRUE,0,lineNum *3* sizeof(float),result,NULL,NULL);

    // Configure event processing
//    cl_ulong start = 0, end = 0;
//    start = profileEvent.getProfilingInfo<CL_PROFILING_COMMAND_START>();
//    end = profileEvent.getProfilingInfo<CL_PROFILING_COMMAND_END>();
//    cl_ulong totaltime = end - start;
//    cout<<"gpu time: " <<totaltime*1e-6<<"ms"<<endl;
}
void OpenCL::executeKernel(cl::Buffer halfedgebuf,vector<int> &buf,float z0,float thick,size_t LINESNUMBER)
{

    /* Create a buffer to hold data */
    cl::Buffer clbuf(context,CL_MEM_WRITE_ONLY, LINESNUMBER*3*sizeof(int),NULL);

    /* Create kernel argument */
    groupedge.setArg(0,halfedgebuf);
    groupedge.setArg(1,sizeof (float),&z0);
    groupedge.setArg(2,sizeof(float),&thick);
    groupedge.setArg(3,clbuf);
    cl::NDRange globalSize(LINESNUMBER);
    queue.enqueueNDRangeKernel(groupedge,cl::NullRange,globalSize,cl::NullRange,NULL,&profileEvent);
    queue.finish();
    queue.enqueueReadBuffer(clbuf, CL_TRUE, 0,LINESNUMBER*3* sizeof(int),&buf[0], 0, NULL);
}

void OpenCL::executeKernel(cl::Buffer edgebuf,cl::Buffer resultbuf,size_t total,size_t LAYERNUMBER,float *zheight,vector<unsigned int>linesnumber)
{
    cl::Buffer zbuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,LAYERNUMBER*sizeof(float),zheight);
    cl::Buffer linesnumberbuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,linesnumber.size()*sizeof(unsigned int),linesnumber.data());
    calalledges.setArg(0,edgebuf);
    calalledges.setArg(1,resultbuf);
    calalledges.setArg(2,zbuf);
    calalledges.setArg(3,linesnumberbuf);
    cl::NDRange globalSize(total,LAYERNUMBER);
    queue.enqueueNDRangeKernel(calalledges,cl::NullRange,globalSize,cl::NullRange);
    queue.finish();
}

void OpenCL::executeKernel(cl::Buffer edgebuf,cl::Buffer resultbuf,size_t LINESNUMBER,float zheight)
{
    /* Create kernel argument */
    callayeredges.setArg(0,edgebuf);
    callayeredges.setArg(1,resultbuf);
    callayeredges.setArg(2,sizeof(float),&zheight);
    cl::NDRange globalSize(LINESNUMBER);
    queue.enqueueNDRangeKernel(callayeredges,cl::NullRange,globalSize,cl::NullRange);
    queue.finish();
}
