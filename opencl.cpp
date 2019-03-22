#include "opencl.h"
#include <fstream>
#include <iostream> 
#include <QTime>
using namespace std;
OpenCL::OpenCL()
{
    const char* PROGRAM_FILE="D:/QTAPP/Slicer/kernels.cl";
    const char* CAPBYHEIGHT ="capbyheight";
    const char* CALALLEDGES ="calalledges";
    const char* GROUPEDGE ="groupedge";
    const char* INTERSECT ="intersect";
    const char* HASHFIND ="hashfind";


    vector<cl::Platform> platforms;
    vector<cl::Device> devices;
    cl::Event profileEvent;
    // Place the GPU devices of the first platform into a context
    cl::Platform::get(&platforms);
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    if(devices.empty())
    {
        platforms[0].getDevices(CL_DEVICE_TYPE_CPU, &devices);
    }
    for(size_t i=0; i<devices.size(); i++)
    {
        string device_name = devices[i].getInfo<CL_DEVICE_NAME>();
        cout << "Device: " << device_name.c_str() << endl;
        cl_uint maxComputeUnits=devices[i].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
        cout << "Parallel compute units: " << maxComputeUnits<< endl;
        size_t maxWorkItemPerGroup=devices[i].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
        cout << "maxWorkItemPerGroup: " << maxWorkItemPerGroup<< endl;
        cl_ulong maxGlobalMemSize=devices[i].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
        cout << "maxGlobalMemSize: " << maxGlobalMemSize/1024/1024<<"MB"<<endl;
        cl_ulong maxConstantBufferSize=devices[i].getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();
        cout << "maxConstantBufferSize: " << maxConstantBufferSize/1024<<"KB"<<endl;
        cl_ulong maxLocalMemSize=devices[i].getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
        cout << "maxLocalMemSize: " << maxLocalMemSize/1024<<"KB"<<endl;
        cl_ulong maxMemAllocSize=devices[i].getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
        cout << "maxMemAllocSize: " << maxMemAllocSize/1024/1024<<"MB"<<endl;

        deviceinfo.deviceName=device_name;
        deviceinfo.maxComputeUnits=int(maxComputeUnits);
        deviceinfo.maxWorkItemPerGroup=int(maxWorkItemPerGroup);
        deviceinfo.maxGlobalMemSize=int(maxGlobalMemSize/1024/1024);
        deviceinfo.maxConstantBufferSize=int(maxConstantBufferSize/1024);
        deviceinfo.maxLocalMemSize=int(maxLocalMemSize/1024);
        deviceinfo.maxMemAllocSize=int(maxMemAllocSize/1024/1024);
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
    intersect=cl::Kernel(program,INTERSECT);
    hashfind=cl::Kernel(program,HASHFIND);
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
void OpenCL::executeKernel(cl::Buffer vertexbuf,cl::Buffer halfedgebuf,vector<int> &buf,float z0,float thick,size_t LINESNUMBER)
{

    /* Create a buffer to hold data */
    cl::Buffer clbuf(context,CL_MEM_WRITE_ONLY, LINESNUMBER*3*sizeof(int),NULL);

    /* Create kernel argument */
    groupedge.setArg(0,vertexbuf);
    groupedge.setArg(1,halfedgebuf);
    groupedge.setArg(2,sizeof (float),&z0);
    groupedge.setArg(3,sizeof(float),&thick);
    groupedge.setArg(4,clbuf);
    cl::NDRange globalSize(LINESNUMBER);
    queue.enqueueNDRangeKernel(groupedge,cl::NullRange,globalSize,cl::NullRange,NULL,&profileEvent);
    queue.finish();
    queue.enqueueReadBuffer(clbuf, CL_TRUE, 0,LINESNUMBER*3* sizeof(int),&buf[0], 0, NULL);
}

void OpenCL::executeKernel(cl::Buffer vertexbuf,cl::Buffer halfedgebuf,vector<unsigned int>edgeset,vector<cl_float3>&result,size_t total,size_t LAYERNUMBER,float *zheight,vector<unsigned int>linesnumber)
{
    cl_int err;
    cl::Buffer edgebuf(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,edgeset.size()*sizeof(unsigned int),edgeset.data(),&err);
    if(err<0)
    {
        cout<<"can't create the edgebuf."<<endl;
    }
    cl::Buffer resultbuf(context,CL_MEM_WRITE_ONLY,total*sizeof(cl_float3),result.data(),&err);
    if(err<0)
    {
        cout<<"can't create the resultbuf."<<endl;
    }
    cl::Buffer zbuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,LAYERNUMBER*sizeof(float),zheight,&err);
    if(err<0)
    {
        cout<<"can't create the zbuf."<<endl;
    }
    cl::Buffer linesnumberbuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,linesnumber.size()*sizeof(unsigned int),linesnumber.data(),&err);
    if(err<0)
    {
        cout<<"can't create the linesnumberbuf."<<endl;
    }
    err =calalledges.setArg(0,vertexbuf);
    err |= calalledges.setArg(1,halfedgebuf);
    err |=calalledges.setArg(2,edgebuf);
    err |=calalledges.setArg(3,resultbuf);
    err |=calalledges.setArg(4,zbuf);
    err |=calalledges.setArg(5,linesnumberbuf);
    if(err<0)
    {
        cout<<"can't set the arg. "<<err<<endl;
    }
    cl::NDRange globalSize(total,LAYERNUMBER);
    err=queue.enqueueNDRangeKernel(calalledges,cl::NullRange,globalSize,cl::NullRange);
    if(err<0)
    {
        cout<<"can't executeKernel."<<err<<endl;
    }
    queue.finish();
    err=queue.enqueueReadBuffer(resultbuf, CL_TRUE, 0,result.size()*sizeof(cl_float3),&result[0], 0, NULL);
    if(err<0)
    {
        cout<<"can't read the result."<<err<<endl;
    }
}

void OpenCL::executeKernel(vector<unsigned int> edges,vector<unsigned int>linesnumber,vector<cl_int3>&hashTable,unsigned int layernumber,
                           vector<unsigned int>&location,vector<unsigned int>&loopcount,vector<unsigned int>&loopnumber)
{
    cl_int err=0;
    QTime time;
    time.start();
    cl::Buffer edgebuf(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,edges.size()*sizeof(unsigned int),edges.data(),&err);
    if(err<0)
    {
        cout<<"can't creat edgebuf."<<err<<endl;
    }
    cl::Buffer linesnumberbuf(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,linesnumber.size()*sizeof(unsigned int),linesnumber.data(),&err);
    if(err<0)
    {
        cout<<"can't creat linesnumberbuf."<<err<<endl;
    }
    cl::Buffer hashTablebuf(context,CL_MEM_READ_WRITE| CL_MEM_COPY_HOST_PTR,hashTable.size()*sizeof(cl_int3),hashTable.data(),&err);
    if(err<0)
    {
        cout<<"can't creat hashTablebuf."<<err<<endl;
    }
    cl::Buffer locationbuf(context,CL_MEM_WRITE_ONLY,location.size()*sizeof(unsigned int),&err);
    if(err<0)
    {
        cout<<"can't creat locationbuf."<<err<<endl;
    }
    cl::Buffer loopcountbuf(context,CL_MEM_WRITE_ONLY,loopcount.size()*sizeof(unsigned int),&err);
    if(err<0)
    {
        cout<<"can't creat loopcountbuf."<<err<<endl;
    }
    cl::Buffer loopnumberbuf(context,CL_MEM_WRITE_ONLY,loopnumber.size()*sizeof(unsigned int),&err);
    if(err<0)
    {
        cout<<"can't creat loopnumberbuf."<<err<<endl;
    }
    cout<<"create buffer time:"<<time.elapsed()<<"ms"<<endl;
    err =hashfind.setArg(0,edgebuf);
    err |=hashfind.setArg(1,linesnumberbuf);
    err |=hashfind.setArg(2,hashTablebuf);
    err |=hashfind.setArg(3,locationbuf);
    err |=hashfind.setArg(4,loopcountbuf);
    err |=hashfind.setArg(5,loopnumberbuf);
    if(err<0)
    {
        cout<<"can't setArg."<<err<<endl;
    }
    cl::NDRange globalSize(layernumber);
    err=queue.enqueueNDRangeKernel(hashfind,cl::NullRange,globalSize,cl::NullRange);
    if(err<0)
    {
        cout<<"can't executeKernel hashfind."<<err<<endl;
    }
    queue.finish();

    err= queue.enqueueReadBuffer(locationbuf, CL_TRUE, 0,location.size()*sizeof(unsigned int),&location[0]);

    err |=queue.enqueueReadBuffer(hashTablebuf, CL_TRUE, 0,hashTable.size()*sizeof(cl_int4),&hashTable[0]);

    err |=queue.enqueueReadBuffer(loopcountbuf, CL_TRUE, 0,loopcount.size()*sizeof(unsigned int),&loopcount[0]);

    err |=queue.enqueueReadBuffer(loopnumberbuf, CL_TRUE, 0,loopnumber.size()*sizeof(unsigned int),&loopnumber[0]);
    if(err<0)
    {
        cout<<"can't read the buf. "<<err<<endl;
    }
}
