#include "opencl.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

using namespace std;

OpenCL::OpenCL()
{
    initOpencl();
}
OpenCL::~OpenCL()
{
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseKernel(capbyheight);
    clReleaseKernel(capbyedge);
}

void OpenCL::initOpencl()
{
    const char* PROGRAM_FILE ="F:/QT/Slicer/kernels.cl";
    const char* CAPBYHEIGHT ="capbyheight";
    const char* CAPBYEDGE ="capbyedge";

    /* OpenCL data structures */
    int  err;
    /* Create a device and context */
    device = create_device();
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if(err < 0) {
       perror("Couldn't create a context");
       exit(1);
    }

    /* Build the program and create a kernel */
    program = build_program(context, device, PROGRAM_FILE);
    capbyheight=clCreateKernel(program, CAPBYHEIGHT, &err);
    capbyedge=clCreateKernel(program, CAPBYEDGE, &err);
    if(err < 0) {
       perror("Couldn't create a kernel");
       exit(1);
    };
    /* Create a command queue */
    queue = clCreateCommandQueue(context, device,CL_QUEUE_PROFILING_ENABLE, &err);
    if(err < 0) {
       perror("Couldn't create a command queue");
       exit(1);
    };
}

cl_device_id OpenCL::create_device() {

   cl_device_id dev,*devices;
   cl_uint err,num,num_devices;
   char name_data[48];

   err = clGetPlatformIDs(0,NULL, &num);
   /* Identify a platform */
   vector<cl_platform_id> platforms(num);
   err = clGetPlatformIDs(num, &platforms[0], &num);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   }

   /* Access a device */
   err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU,0,NULL, &num_devices);
   cout<<"The number of devices:"<<num_devices<<endl;
   if(err == CL_DEVICE_NOT_FOUND) {
       cout<<"Couldn't find and GPU,use CPU instead!"<<endl;
       err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);
   }
   devices=(cl_device_id*)malloc(sizeof(cl_device_id) * num_devices);
   clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU,num_devices, devices, NULL);
   for(int i=0; i<num_devices; i++) {
       err = clGetDeviceInfo(devices[i], CL_DEVICE_NAME,sizeof(name_data), name_data, NULL);
       cout<<"Name:"<<name_data<<endl;
   }
   return devices[0];
}

/* Create program from a file and compile it */
cl_program OpenCL::build_program(cl_context ctx, cl_device_id dev, const char* filename) {

   cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
   int err;

   /* Read program file and place content into buffer */
   program_handle = fopen(filename, "rb");
   if(program_handle == NULL) {
      perror("Couldn't find the program file");
      exit(1);
   }
   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);

   /* Create program from file */
   program = clCreateProgramWithSource(ctx, 1,
      (const char**)&program_buffer, &program_size, &err);
   if(err < 0) {
      perror("Couldn't create the program");
      exit(1);
   }
   free(program_buffer);

   /* Build program */
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if(err < 0) {

      /* Find size of log and print to std output */
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
            0, NULL, &log_size);
      program_log = (char*) malloc(log_size + 1);
      program_log[log_size] = '\0';
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
            log_size + 1, program_log, NULL);
      printf("%s\n", program_log);
      free(program_log);
      exit(1);
   }

   return program;
}

void OpenCL::executeKernel(float *interSection1,float *interSection2,float *result,int lineNum,float zheight)
{

    int err;
    cl_event ev;
    /* Create a buffer to hold data */
    cl_mem buf1 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,lineNum *3*sizeof(float),interSection1, &err);
    cl_mem buf2 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,lineNum *3*sizeof(float),interSection2, &err);
    cl_mem clz = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(float),&zheight, &err);
    cl_mem clbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, lineNum *3*sizeof(float), NULL, &err);
    if(err < 0) {
       perror("Couldn't create buffer!");
       exit(1);
    };

    /* Create kernel argument */
    clSetKernelArg(capbyheight, 0, sizeof(cl_mem), &buf1);
    clSetKernelArg(capbyheight, 1, sizeof(cl_mem), &buf2);
    clSetKernelArg(capbyheight, 2, sizeof(cl_mem), &clbuf);
    clSetKernelArg(capbyheight, 3, sizeof(cl_mem), &clz);

    size_t globalSize[2] ={size_t(lineNum),3};
    size_t localSize[2]={0,3};
    if(lineNum<64)
        localSize[0] =lineNum;
    else
        localSize[0] = 64;
    err=clEnqueueNDRangeKernel(queue, capbyheight, 2, NULL, globalSize,NULL, 0, NULL, &ev);
    clFinish(queue);
    err = clEnqueueReadBuffer(queue, clbuf, CL_TRUE, 0,lineNum *3* sizeof(float), result, 0, NULL, NULL);
    if(err < 0) {
       perror("Couldn't enqueue the read buffer command");
       exit(1);
    }

    cl_ulong startTime = 0, endTime = 0;
    clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL);
    clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL);
    cl_ulong totaltime = endTime - startTime;
    //cout<<"simple kernel exec time: " <<totaltime*1e-6<<"ms"<<endl;

    /* Deallocate resources */
    clReleaseMemObject(buf1);
    clReleaseMemObject(buf2);
    clReleaseMemObject(clbuf);
    clReleaseMemObject(clz);
    clReleaseEvent(ev);
}

void OpenCL::executeKernel(float *interSection1,float *interSection2,float *result,size_t LAYERNUMBER,size_t LINESNUMBER ,float *zheight)
 {

     int err;
     cl_event ev;
     /* Create a buffer to hold data */
     cl_mem buf1 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,LAYERNUMBER*LINESNUMBER *3*sizeof(float),interSection1, &err);
     cl_mem buf2 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,LAYERNUMBER*LINESNUMBER *3*sizeof(float),interSection2, &err);
     cl_mem clz = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,LAYERNUMBER*sizeof(float),zheight, &err);
     cl_mem clbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, LAYERNUMBER*LINESNUMBER *3*sizeof(float), NULL, &err);
     if(err < 0) {
        perror("Couldn't create buffer!");
        exit(1);
     };

     /* Create kernel argument */
     clSetKernelArg(capbyedge, 0, sizeof(cl_mem), &buf1);
     clSetKernelArg(capbyedge, 1, sizeof(cl_mem), &buf2);
     clSetKernelArg(capbyedge, 2, sizeof(cl_mem), &clbuf);
     clSetKernelArg(capbyedge, 3, sizeof(cl_mem), &clz);

     size_t globalSize[3] ={LAYERNUMBER,LINESNUMBER,3};
     size_t localSize [3] ={4,16,3};
     err=clEnqueueNDRangeKernel(queue, capbyedge, 3, NULL, globalSize,NULL, 0, NULL, &ev);
     clFinish(queue);
     err = clEnqueueReadBuffer(queue, clbuf, CL_TRUE, 0,LAYERNUMBER*LINESNUMBER *3* sizeof(float), result, 0, NULL, NULL);
     if(err < 0) {
        perror("Couldn't enqueue the read buffer command");
        exit(1);
     }

     /* Deallocate resources */
     clReleaseMemObject(buf1);
     clReleaseMemObject(buf2);
     clReleaseMemObject(clbuf);
     clReleaseMemObject(clz);
     clReleaseEvent(ev);
 }
