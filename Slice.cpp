#include "Slice.h"
#include "Polygon_mesh_slicer_mine.h"
#include <boost/bind.hpp>
#include <CGAL/intersections.h>

#include <algorithm>
#include <QtCore/qmath.h>
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
    findtime=0;
    comptime=0;
    sliceedges.reserve(2000);
    z.reserve(2000);
    linesnumber=0;

}

Slice::~Slice()
{

}


void Slice::startSlice(Mesh mesh,double zmin,double zmax)
{
    CGAL::Polygon_mesh_slicer<Mesh, Kernel> slicer(mesh);
    intrpoints.clear();
    sliceedges.clear();
    z.clear();
    layernumber=0;
//    QProgressDialog *progressDlg=new QProgressDialog();
//    progressDlg->setWindowModality(Qt::WindowModal);
//    progressDlg->setMinimumDuration(0);
//    progressDlg->setAttribute(Qt::WA_DeleteOnClose, true);
//    progressDlg->setWindowTitle("切片");
//    progressDlg->setLabelText("正在切片......");
//    progressDlg->setRange(zmin,zmax);
    zheight=zmin;
    findtime=0;
    comptime=0;
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
        time.start();
        slicer(Kernel::Plane_3(0, 0, 1, -zheight),back_inserter(intredges));
        findtime +=time.elapsed();
        if(!isParaComp)
        {
            time.restart();
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
            intrpoints.push_back(polylines);
            comptime +=time.elapsed();
        }
        else
        {
            time.restart();
            int num=0;
            for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
            {
                (*iter).pop_back();
                //cout<<(*iter).size()<<endl;
                num +=(*iter).size();
            }
            if(linesnumber<num)
            {
                linesnumber=num;
            }
            sliceedges.push_back(intredges);
            z.push_back(zheight);
            comptime +=time.elapsed();
        }
        layernumber++;
        if(isAdapt)
        {
            float adaptthick;
            float minangle=adaptSlice(mesh,intredges);
            if(minangle>0.99)
                adaptthick=0.3;
            else
                adaptthick=0.1/(minangle+1e-3);
            if(adaptthick<0.1)adaptthick=0.1;
            if(adaptthick>0.3)adaptthick=0.3;
            //cout<<adaptthick<<endl;
            zheight += adaptthick;
        }
        else
            zheight += thick;
    }
    if(isParaComp)
    {
        time.restart();
        //cout<<linesnumber<<endl;
        float *interSection1,*interSection2,*result;
        interSection1 = (float *)malloc(layernumber*linesnumber *3* sizeof(float));
        interSection2 = (float *)malloc(layernumber*linesnumber *3* sizeof(float));
        result = (float *)malloc(layernumber*linesnumber *3* sizeof(float));
        for(int i=0;i<layernumber;i++)
        {
            int lineNum=0;
            for(list<Outline>::iterator iter= sliceedges[i].begin();iter != sliceedges[i].end();iter++)
            {
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
            if(lineNum>0)
            {
                int num=0;
                for(list<Outline>::iterator iter= sliceedges[i].begin();iter != sliceedges[i].end();iter++)
                {
                    for(uint j=0;j<(*iter).size();j++)
                    {
                        try
                        {
                            Mesh::edge_index ed=boost::any_cast<Mesh::edge_index>((*iter)[j]);
                            //cout<<ed<<endl;
                            Point p1=mesh.point(mesh.vertex(ed,0));
                            Point p2=mesh.point(mesh.vertex(ed,1));
                            //cout<<p1.x()<<" "<<p1.y()<<" "<<p1.z()<<endl;
                            interSection1[i*3*linesnumber+3*(num+j)+0]=p1.x();
                            interSection1[i*3*linesnumber+3*(num+j)+1]=p1.y();
                            interSection1[i*3*linesnumber+3*(num+j)+2]=p1.z();
                            interSection2[i*3*linesnumber+3*(num+j)+0]=p2.x();
                            interSection2[i*3*linesnumber+3*(num+j)+1]=p2.y();
                            interSection2[i*3*linesnumber+3*(num+j)+2]=p2.z();
                        }
                        catch(boost::bad_any_cast & ex)
                        {
                            //cout<<"cast error:"<<ex.what()<<endl;
                        }
                    }
                    num +=(*iter).size();
                }
            }
        }
        opencl.executeKernel(interSection1,interSection2,result,layernumber,linesnumber,z.data());
        for(int i=0;i<layernumber;i++)
        {
            polylines.clear();
            int lineNum=0;
            for(list<Outline>::iterator iter= sliceedges[i].begin();iter != sliceedges[i].end();iter++)
            {
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
                for(list<Outline>::iterator iter=sliceedges[i].begin();iter !=sliceedges[i].end();iter++)
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
                intrpoints.push_back(polylines);
            }
            else
            {
                int num=0;
                for(list<Outline>::iterator iter= sliceedges[i].begin();iter !=sliceedges[i].end();iter++)
                {
                    lines.clear();
                    for(uint j=0;j<(*iter).size();j++)
                    {
                        float x=result[i*3*linesnumber+3*(num+j)+0];
                        float y=result[i*3*linesnumber+3*(num+j)+1];
                        lines.push_back(Point(x,y,z[i]));
                    }
                    num +=(*iter).size();
                    polylines.push_back(lines);
                }
                intrpoints.push_back(polylines);
            }

        }
        free(interSection1);
        free(interSection2);
        comptime +=time.elapsed();
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"gpu compute time:"<<comptime<<"ms"<<endl;
        //cout<<"time of parallel computing:"<<time.elapsed()<<"ms"<<endl;
    }
    else
    {
        cout<<"find edge time:"<<findtime<<"ms"<<endl;
        cout<<"cpu compute time:"<<comptime<<"ms"<<endl;
        //cout<<"time of cpu computing:"<<time.elapsed()<<"ms"<<endl;
    }
    //progressDlg->close();
}

float Slice::adaptSlice(Mesh mesh,Intredges intredges)
{
    vector<float> angle;
    angle.reserve(2000);
    for(list<Outline>::iterator iter= intredges.begin();iter != intredges.end();iter++)
    {
        //cout<<(*iter).size()<<endl;
        for(vector<boost::any>::iterator it=(*iter).begin();it!=(*iter).end();it++)
        {
            try
            {
                Mesh::edge_index ed=boost::any_cast<Mesh::edge_index>(*it);
                Mesh::face_index f0=mesh.face(mesh.halfedge(ed,0));
                //cout<<f<<" ";
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
                angle.push_back(cos);
            }
            catch(boost::bad_any_cast & ex)
            {
                angle.push_back(1.0);
                //cout<<"cast error:"<<ex.what()<<endl;
            }
        }
        //cout<<endl;
    }
    float minangle=*min_element(angle.begin(),angle.end());
    return minangle;
}
