#include "Slice.h"
#include "Polygon_mesh_slicer_mine.h"
#include <boost/bind.hpp>
#include <CGAL/intersections.h>

#include <algorithm>
#include <QtCore/qmath.h>
#include <QTime>
#include <QDebug>
#include <windows.h>
#include <QProgressDialog>
#include <QMessageBox>
using namespace std;
Slice::Slice()
{
    thick=10;
    layernumber=0;
    zheight=0.0f;
    isParaComp=true;
    lines.reserve(1000);
    initOpencl();
}

Slice::~Slice()
{
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseKernel(cap);
    delete(interSection1);
    delete(interSection2);
}


void Slice::startSlice(Mesh mesh,double zmin,double zmax)
{
    CGAL::Polygon_mesh_slicer<Mesh, Kernel> slicer(mesh);
    intrpoints.clear();
    layernumber=1;
//    QProgressDialog *progressDlg=new QProgressDialog();
//    progressDlg->setWindowModality(Qt::WindowModal);
//    progressDlg->setMinimumDuration(0);
//    progressDlg->setAttribute(Qt::WA_DeleteOnClose, true);
//    progressDlg->setWindowTitle("切片");
//    progressDlg->setLabelText("正在切片......");
//    progressDlg->setRange(zmin,zmax);
    zheight=zmin;
    while(zheight<=zmax)
    {
//        progressDlg->setValue(zheight);
//        if(progressDlg->wasCanceled())
//        {
//            layernumber=1;
//            intrpoints.clear();
//            QMessageBox::warning(NULL,QStringLiteral("提示"),QStringLiteral("取消切片"));
//            return;
//        }
        //cout<<"layer of "<<layernumber<<":"<<endl;
        polylines.clear();
        intredges.clear();
        slicer(Kernel::Plane_3(0, 0, 1, -zheight),back_inserter(intredges));
        if(!isParaComp)
        {
            for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
            {
                lines.clear();
                (*iter).pop_back();
                //cout<<(*iter).size()<<endl;
                for(vector<boost::any>::iterator it=(*iter).begin();it!=(*iter).end();it++)
                {
                    try
                    {
                        Mesh::edge_index ed=boost::any_cast<Mesh::edge_index>(*it);
                        //cout<<ed<<endl;
                        Point p1=mesh.point(mesh.vertex(ed,0));
                        Point p2=mesh.point(mesh.vertex(ed,1));
                        Segment s(p1,p2);
                        CGAL::cpp11::result_of<Kernel::Intersect_3(Kernel::Plane_3, Segment)>::type
                                  inter = intersection(Kernel::Plane_3(0, 0, 1, -zheight), s);
                        CGAL_assertion(inter != boost::none);
                        const Point* pt_ptr = boost::get<Point>(&(*inter));
                        lines.push_back(*pt_ptr);
                    }
                    catch(boost::bad_any_cast & ex)
                    {
                        //cout<<"cast error:"<<ex.what()<<endl;
                        Point point=boost::any_cast<Point>(*it);
                        lines.push_back(point);
                        //cout<<vd<<endl;
                    }
                }
                polylines.push_back(lines);
            }
        }
        else
        {
            int lineNum=0;
            for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
            {
                (*iter).pop_back();
                //cout<<(*iter).size()<<endl;
                for(vector<boost::any>::iterator it=(*iter).begin();it!=(*iter).end();it++)
                {
                    try
                    {
                        Mesh::edge_index ed=boost::any_cast<Mesh::edge_index>(*it);
                        lineNum +=(*iter).size();
                        break;
                    }
                    catch(boost::bad_any_cast & ex)
                    {
                        //cout<<"cast error:"<<ex.what()<<endl;
                    }
                }
            }
            if(lineNum==0)
            {
                for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
                {
                    lines.clear();
                    //cout<<(*iter).size()<<endl;
                    for(vector<boost::any>::iterator it=(*iter).begin();it!=(*iter).end();it++)
                    {
                        try
                        {
                            Point point=boost::any_cast<Point>(*it);
                            lines.push_back(point);
                        }
                        catch(boost::bad_any_cast & ex)
                        {
                            //cout<<"cast error:"<<ex.what()<<endl;
                        }
                    }
                    polylines.push_back(lines);
                }
            }
            else
            {
                float *result;
                result  = (float *)malloc(lineNum *3* sizeof(float));
                //for (int i = 0; i < lineNum *3; result[i] = 0, i++);
                setBuffer(mesh,lineNum);
                executeKernel(interSection1,interSection2,result,lineNum);
                int num=0;
                for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
                {
                    lines.clear();
                    for(uint j=0;j<(*iter).size();j++)
                    {
                        //cout<<buf[3*(num+j)]<<" "<<buf[3*(num+j)+1]<<" "<<buf[3*(num+j)+2]<<endl;
                        float x=result[3*(num+j)];
                        float y=result[3*(num+j)+1];
                        float z=result[3*(num+j)+2];
                        lines.push_back(Point(x,y,z));
                    }
                    num +=(*iter).size();
                    polylines.push_back(lines);
                }
                free(interSection1);
                free(interSection2);
                free(result);
            }

        }
        intrpoints.push_back(polylines);
        layernumber++;
//            for(int i=0;i<normalangle.size();i++)
//            {
//                cout<<normalangle[i]<<" ";
//            }
//            cout<<endl;
            //float minnormalangle=*min_element(normalangle.begin(),normalangle.end());
            //cout<<minnormalangle<<endl;
//            if(minnormalangle>0.99)
//                adaptthick=0.3;
//            else
//                adaptthick=0.1/(minnormalangle+1e-3);
//        }
//        if(adaptthick<0.1)adaptthick=0.1;
//        if(adaptthick>0.3)adaptthick=0.3;
        //cout<<adaptthick<<endl;
//        if(isAdapt)
//            zheight += adaptthick;
//        else
            zheight += thick;
    }

    //progressDlg->close();
}

float Slice::normalAngle(Mesh mesh,Mesh::Face_index f0)
{
    CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
    vector<Point> point;
    for(boost::tie(vbegin, vend) = vertices_around_face(mesh.halfedge(f0), mesh);vbegin != vend;++vbegin)
    {
        point.push_back(mesh.point(*vbegin));
        //cout << *vbegin<<":"<<mesh.point(*vbegin)<<endl;
    }
    //cout<<point.size()<<endl;
    float x1,x2,x3,y1,y2,y3,z1,z2,z3,nx,ny,nz;
    //求面f0的法向量
    x1=point[0].x();y1=point[0].y();z1=point[0].z();
    x2=point[1].x();y2=point[1].y();z2=point[1].z();
    x3=point[2].x();y3=point[2].y();z3=point[2].z();
    nx=(y2-y1)*(z3-z1)-(z2-z1)*(y3-y1);
    ny=(z2-z1)*(x3-x1)-(z3-z1)*(x2-x1);
    nz=(x2-x1)*(y3-y1)-(x3-x1)*(y2-y1);
    //cout<<nx<<" "<<ny<<" "<<nz<<endl;
    float dist=sqrt(nx*nx+ny*ny+nz*nz);
    float cos=nz/dist;
    //cout<<cos<<endl;
    return cos;
}

void Slice::initOpencl()
{
    const char* PROGRAM_FILE ="F:/QT/Slicer/kernels.cl";
    const char* KERNEL_FUNC ="cap";

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
    cap=clCreateKernel(program, KERNEL_FUNC, &err);
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

cl_device_id Slice::create_device() {

   cl_device_id dev,*devices;
   cl_uint err,num,num_devices;
   char name_data[48];

   err = clGetPlatformIDs(0, 0, &num);
   /* Identify a platform */
   vector<cl_platform_id> platforms(num);
   err = clGetPlatformIDs(num, &platforms[0], &num);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   }

   /* Access a device */
   err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU,0,NULL, &num_devices);
   //cout<<"The number of devices:"<<num_devices<<endl;
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
       //cout<<"Name:"<<name_data<<endl;
   }
   return devices[0];
}

/* Create program from a file and compile it */
cl_program Slice::build_program(cl_context ctx, cl_device_id dev, const char* filename) {

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

void Slice::executeKernel(float *interSection1,float*interSection2,float *result,int lineNum)
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
    clSetKernelArg(cap, 0, sizeof(cl_mem), &buf1);
    clSetKernelArg(cap, 1, sizeof(cl_mem), &buf2);
    clSetKernelArg(cap, 2, sizeof(cl_mem), &clbuf);
    clSetKernelArg(cap, 3, sizeof(cl_mem), &clz);

    size_t globalSize[2] ={size_t(lineNum),3};
    err=clEnqueueNDRangeKernel(queue, cap, 2, NULL, globalSize,NULL, 0, NULL, &ev);
    clFinish(queue);
    err = clEnqueueReadBuffer(queue, clbuf, CL_TRUE, 0,lineNum *3* sizeof(float), result, 0, NULL, NULL);
    if(err < 0) {
       perror("Couldn't enqueue the read buffer command");
       exit(1);
    }

//    cl_ulong startTime = 0, endTime = 0;
//    clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL);
//    clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL);
//    cl_ulong totaltime = endTime - startTime;
//    cout<<"simple kernel exec time: " <<totaltime*1e-6<<"ms"<<endl;

    /* Deallocate resources */
    clReleaseMemObject(buf1);
    clReleaseMemObject(buf2);
    clReleaseMemObject(clbuf);
    clReleaseMemObject(clz);
    clReleaseEvent(ev);
}

void Slice::setBuffer(Mesh mesh,int lineNum)
{
    interSection1 = (float *)malloc(lineNum *3* sizeof(float));
    interSection2 = (float *)malloc(lineNum *3* sizeof(float));
    int num=0;
    for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
    {
        (*iter).pop_back();
        for(uint j=0;j<(*iter).size();j++)
        {
            try
            {
                Mesh::edge_index ed=boost::any_cast<Mesh::edge_index>((*iter)[j]);
                //cout<<ed<<endl;
                Point p1=mesh.point(mesh.vertex(ed,0));
                Point p2=mesh.point(mesh.vertex(ed,1));
                //cout<<p1.x()<<" "<<p1.y()<<" "<<p1.z()<<endl;
                interSection1[3*(num+j)+0]=p1.x();
                interSection1[3*(num+j)+1]=p1.y();
                interSection1[3*(num+j)+2]=p1.z();
                interSection2[3*(num+j)+0]=p2.x();
                interSection2[3*(num+j)+1]=p2.y();
                interSection2[3*(num+j)+2]=p2.z();
            }
            catch(boost::bad_any_cast & ex)
            {
                cout<<"cast error:"<<ex.what()<<endl;
            }
        }
        num +=(*iter).size();
    }
}

