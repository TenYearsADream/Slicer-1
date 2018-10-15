#include "Slice.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <boost/bind.hpp>
#include <algorithm>
#include <QtCore/qmath.h>
#include <QTime>
#include <QDebug>
#include <windows.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Surface_mesh<Kernel::Point_3> Mesh;
typedef Mesh::Vertex_index vertex_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
typedef Kernel::Point_3 Point;
using namespace std;
Slice::Slice()
{
    thick=10;
    layernumber=0;
    zheight=0.0f;
    isParaComp=true;
    initOpencl();
}

Slice::~Slice()
{
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseKernel(cap);
}

void Slice::intrSurfs(Mesh mesh)
{
    vector<intersectFace>().swap(intrsurfs);
    vector<float>().swap(normalangle);
    for(Mesh::Face_iterator f=mesh.faces_begin();f!=mesh.faces_end();f++)
    {
        CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
        vector<double>height;
        for(boost::tie(vbegin, vend) = vertices_around_face(mesh.halfedge(*f), mesh);vbegin != vend;++vbegin)\
        {
            height.push_back(mesh.point(*vbegin).z());
        }
        double zmin=*min_element(height.begin(),height.end());
        double zmax=*max_element(height.begin(),height.end());
        //cout<<zmin<<" "<<zmax<<" "<<zheight<<endl;
        if((zheight-zmin)>1e-4 && (zmax-zheight)>1e-4)
        {
            float cos=normalAngle(mesh,*f);
            normalangle.push_back(sqrt(1-cos*cos));
            intrsurfs.push_back(intersectFace(*f,false,false));
        }
        else if((qAbs(zheight-zmin)<=1e-4) && (qAbs(zheight-zmax)<=1e-4))
        {
            float cos=normalAngle(mesh,*f);
            normalangle.push_back(sqrt(1-cos*cos));
            intrsurfs.push_back(intersectFace(*f,false,true));
        }
    }
    //cout<<"number of intrsurfs:"<<intrsurfs.size()<<endl;
}

void Slice::startSlice(Mesh mesh,double zmin,double zmax)
{

    zheight=zmin;
    while(zheight<=zmax)
    {       
        vector<vector<Point>>().swap(points);
        vector<Mesh::halfedge_index>().swap(intredges);
        vector<int>().swap(loopNum);
        //cout<<"layer of "<<layernumber+1<<":"<<endl;
        intrSurfs(mesh);
        //寻找相交线集合
        for(uint i=0;i<intrsurfs.size();i++)
        {
            Mesh::Face_index fbegin=intrsurfs[i].Faceindex;
            if(!intrsurfs[i].isSliced)
            {
                //cout<<"face:"<<fbegin<<endl;
                if(intrsurfs[i].isParallel)
                {
                    //cout<<"parallelled!"<<endl;
                    parallelPoint(mesh,fbegin);
                }
                else
                {
                    //cout<<"isnot parallel!"<<endl;
                    findEdge(mesh,fbegin);
                }
            }
        }

        if(intredges.size()>0)
        {
            int lineNum = intredges.size();
            //cout<<"The number of intersection line:"<<BUFSIZE<<endl;

            if(!isParaComp)
            {
                int num=0;
                for(uint i=0;i<loopNum.size();i++)
                {
                    vector<Point>point;
                    //cout<<"The number of lines this loop:"<<loopNum[i]<<endl;
                    for(int j=0;j<loopNum[i];j++)
                    {
                        //cout<<intredges[num+j]<<" "<<num+j<<" ";
                        point.push_back(intersectPoint(mesh,intredges[num+j]));
                    }
                    num +=loopNum[i];
                    //cout<<endl;
                    points.push_back(point);
                }
            }
            else
            {
                float *result;
                result  = (float *)malloc(lineNum *3* sizeof(float));
                for (int i = 0; i < lineNum *3; result[i] = 0, i++);
                setBuffer(mesh,lineNum);
                executeKernel(interSection1,interSection2,result,lineNum);
                int num=0;
                for(uint i=0;i<loopNum.size();i++)
                {
                    vector<Point>point;
                    for(int j=0;j<loopNum[i];j++)
                    {
                        //cout<<buf[3*(num+j)]<<" "<<buf[3*(num+j)+1]<<" "<<buf[3*(num+j)+2]<<endl;
                        float x=result[3*(num+j)];
                        float y=result[3*(num+j)+1];
                        float z=result[3*(num+j)+2];
                        point.push_back(Point(x,y,z));
                    }
                    num +=loopNum[i];
                    points.push_back(point);
                }
                free(interSection1);
                free(interSection2);
                free(result);
            }
        }

        if(!points.empty())
        {
            //cout<<points[0].size()<<endl;
            intrpoints.push_back(sliceData(areaSort(points)));
            layernumber++;
//            for(int i=0;i<normalangle.size();i++)
//            {
//                cout<<normalangle[i]<<" ";
//            }
//            cout<<endl;
            float minnormalangle=*min_element(normalangle.begin(),normalangle.end());
            //cout<<minnormalangle<<endl;
            if(minnormalangle>0.99)
                adaptthick=0.3;
            else
                adaptthick=0.1/(minnormalangle+1e-3);
        }
        if(adaptthick<0.1)adaptthick=0.1;
        if(adaptthick>0.3)adaptthick=0.3;
        //cout<<adaptthick<<endl;
        if(isAdapt)
            zheight += adaptthick;
        else
            zheight += thick;
    }
}

bool Slice::isIntr(Mesh mesh,Mesh::halfedge_index edge)
{
    Mesh::Edge_index edgeindex=mesh.edge(edge);
    Point p1=mesh.point(mesh.vertex(edgeindex,0));
    Point p2=mesh.point(mesh.vertex(edgeindex,1));
    //cout<<"p1:"<<p1.x()<<" "<<p1.y()<<" "<<p1.z()<<endl;
    //cout<<"p2:"<<p2.x()<<" "<<p2.y()<<" "<<p2.z()<<endl;
    if(p1.z()<zheight && p2.z()>=zheight)
        return true;
    else if(p2.z()<zheight && p1.z()>=zheight)
        return true;
    else if(p2.z()==zheight && p1.z()==zheight)
        return true;
    else
        return false;
}

Point Slice::intersectPoint(Mesh mesh,Mesh::halfedge_index edge)
{
    Mesh::Edge_index edgeindex=mesh.edge(edge);
    Point p1=mesh.point(mesh.vertex(edgeindex,0));
    Point p2=mesh.point(mesh.vertex(edgeindex,1));
    double x=p1.x()+(p2.x()-p1.x())*(zheight-p1.z())/(p2.z()-p1.z());
    double y=p1.y()+(p2.y()-p1.y())*(zheight-p1.z())/(p2.z()-p1.z());
    //cout<<"intersectPoint:"<<x<<" "<<y<<" "<<zheight<<endl;
    return Point(x,y,zheight);
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

vector<vector<Point>> Slice::areaSort(vector<vector<Point>> points)
{
    vector<float>area;
    for(uint i=0;i<points.size();i++)
    {
        float S=0;
        for(uint j=1;j<points[i].size()-1;j++)
        {
            float x1,x2,x3,y1,y2,y3;
            x1=points[i][0].x();y1=points[i][0].y();
            x2=points[i][j].x();y2=points[i][j].y();
            x3=points[i][j+1].x();y3=points[i][j+1].y();
            S +=qAbs((x1*y2+x2*y3+x3*y1-x1*y3-x2*y1-x3*y2)/2.0);
        }
        //cout<<"面积："<<S<<endl;
        area.push_back(S);
    }
    auto max = max_element(area.begin(), area.end());
    vector<Point>tmp;
    tmp=points[distance(area.begin(), max)];
    points[distance(area.begin(), max)]=points[0];
    points[0]=tmp;
//    for(int i=0;i<points.size();i++)
//    {
//        cout<<"第"<<i+1<<"个圈"<<endl;
//        for(int j=0;j<points[i].size();j++)
//        {
//            cout<<points[i][j].x()<<" "<<points[i][j].y()<<" "<<points[i][j].z()<<endl;
//        }

//    }
    return points;
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

//查找相交的边
void Slice::findEdge(Mesh mesh,Mesh::Face_index fbegin)
{    
    //查找相交环
    int loopnum=0;
    Mesh::Face_index fnext;
    //cout<<"start face:"<<fbegin<<endl;
    Mesh::Halfedge_index hebegin=mesh.halfedge(fbegin);
    if(!isIntr(mesh,hebegin))
    {
        hebegin=mesh.next(hebegin);
    }
    //cout<<"start edge:"<<hebegin<<endl;
    //cout<<"the next edge:";
    do
    {
        //cout<<hebegin<<" "<<mesh.opposite(hebegin)<<" "<<mesh.next(mesh.opposite(hebegin))<<" ";
        hebegin=mesh.next(mesh.opposite(hebegin));
        if(!isIntr(mesh,hebegin))
        {
            hebegin=mesh.next(hebegin);
        }
        intredges.push_back(hebegin);
        loopnum++;
        //cout<<hebegin<<" ";
        fnext=mesh.face(hebegin);
        vector<intersectFace>::iterator it =find_if(intrsurfs.begin (),intrsurfs.end (),boost::bind (&intersectFace::Faceindex, _1 ) == fnext);
        (*it).isSliced=true;
    }while(fnext!=fbegin);
    //cout<<endl;
    loopNum.push_back(loopnum);
}

void Slice::parallelPoint(Mesh mesh,Mesh::Face_index fbegin)
{
    vector<Mesh::Vertex_index>contour;
    vector<Mesh::Vertex_index>contourpoints;
    vector<Point>point;

    Mesh::Vertex_index v,vnext;
    CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
    for(boost::tie(vbegin, vend) = vertices_around_face(mesh.halfedge(fbegin), mesh);vbegin != vend;++vbegin)
    {
        vector<Mesh::Vertex_index>::iterator it = find(contour.begin( ),contour.end( ),*vbegin); //查找
        if(it==contour.end( ))
        {
            v=*vbegin;
            //cout<<"start point "<<*vbegin<<":"<<mesh.point(*vbegin)<<endl;
        }
    }
    Mesh::Halfedge_index h0,h1;
    Mesh::Face_index f0,f1;
    vector<Mesh::Vertex_index>::iterator result;
    do{
        contourpoints.push_back(v);
        contour.push_back(v);
        CGAL::Vertex_around_target_circulator<Mesh> vbegin(mesh.halfedge(v),mesh), done(vbegin);
        do {
            //cout<<*vbegin<<endl;
            if(qAbs(mesh.point(*vbegin).z()-zheight)<1e-4)
            {
                vector<Mesh::Vertex_index>::iterator it = find(contourpoints.begin( ),contourpoints.end( ),*vbegin); //查找
                if(it==contourpoints.end( ))
                {
                    h0=mesh.halfedge(v,*vbegin);
                    h1=mesh.opposite(h0);
                    //cout<<h0<<h1<<endl;
                    f0=mesh.face(h0);
                    f1=mesh.face(h1);
                    //cout<<f0<<f1<<endl;
                    vector<intersectFace>::iterator it0 =find_if(intrsurfs.begin (),intrsurfs.end (),boost::bind (&intersectFace::Faceindex, _1 ) == f0);
                    vector<intersectFace>::iterator it1 =find_if(intrsurfs.begin (),intrsurfs.end (),boost::bind (&intersectFace::Faceindex, _1 ) == f1);
                    if(it0 == intrsurfs.end() || it1 == intrsurfs.end())
                    {
                        if(it0 != intrsurfs.end())
                        {
                            (*it0).isSliced=true;
                        }
                        else
                        {
                            (*it1).isSliced=true;
                        }
                        vnext=*vbegin;
                        //cout <<zheight<<":"<<*vbegin<<" "<<mesh.point(*vbegin)<<endl;
                    }
                }
            }
            vbegin++;
        } while(vbegin != done);
        v=vnext;
        //cout<<"next:"<<vnext<<endl;
        result = find(contourpoints.begin( ),contourpoints.end( ),vnext); //查找
    }while(result==contourpoints.end());
    for(uint i=0;i<contourpoints.size();i++)
    {
        point.push_back(mesh.point(contourpoints[i]));
        //cout<<contourpoints[i]<<" ";
    }
    //cout<<endl;
    points.push_back(point);
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
    //cout<<"the number of loops this layer:"<<loopNum.size()<<endl;
    for(uint i=0;i<loopNum.size();i++)
    {
        //cout<<"The number of lines this loop:"<<loopNum[i]<<endl;
        for(int j=0;j<loopNum[i];j++)
        {
            //cout<<intredges[num+j]<<" "<<num+j<<" ";
            Mesh::Edge_index edgeindex=mesh.edge(intredges[num+j]);
            Point p1=mesh.point(mesh.vertex(edgeindex,0));
            Point p2=mesh.point(mesh.vertex(edgeindex,1));
            //cout<<p1.x()<<" "<<p1.y()<<" "<<p1.z()<<endl;
            interSection1[3*(num+j)+0]=p1.x();
            interSection1[3*(num+j)+1]=p1.y();
            interSection1[3*(num+j)+2]=p1.z();
            interSection2[3*(num+j)+0]=p2.x();
            interSection2[3*(num+j)+1]=p2.y();
            interSection2[3*(num+j)+2]=p2.z();
        }
        num +=loopNum[i];
        //cout<<endl;
    }
}
