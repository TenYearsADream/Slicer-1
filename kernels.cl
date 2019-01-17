typedef struct EdgeNode{
    float x1;
    float y1;
    float z1;
    float x2;
    float y2;
    float z2;
    unsigned int f;
}edge;

__kernel void capbyheight(__global float *A, __global float *B, __global float *C,__global float *z)
{
    int row = get_global_id(0);
    int col = get_global_id(1);
    float diffx=B[row*3]-A[row*3];
    float diffy=B[row*3+1]-A[row*3+1];
    float diffz=B[row*3+2]-A[row*3+2];
    C[row*3] = A[row*3]+diffx*(z[0]-A[row*3+2])/diffz;
    C[row*3+1] = A[row*3+1]+diffy*(z[0]-A[row*3+2])/diffz;
    C[row*3+2] = z[0];
}

__kernel void groupedge(__global edge *edges, float z0,float thick,__global int *buf)
{
    int i = get_global_id(0);
    float zmax=fmax(edges[i].z1,edges[i].z2);
    float zmin=fmin(edges[i].z1,edges[i].z2);
	if((zmax-zmin)<1e-8)
	{
		buf[i*3+0]=0;
		buf[i*3+1]=-1;
		buf[i*3+2]=0;
	}
	else
	{
		int num1=ceil((zmin-z0)/thick);
		int num2=(int)((zmax-z0)/thick);
		buf[i*3+0]=num1;
		buf[i*3+1]=num2;
		buf[i*3+2]=edges[i].f;
	}

}


__kernel void calalledges(__global edge *halfedge,__global unsigned int *edges,__global float *result,__global float *z,__global unsigned int *linesnumber)
{
	int i=get_global_id(0);
	int j=get_global_id(1);
	if(linesnumber[j-1]<=i && i<linesnumber[j])
	{
		float diffx=halfedge[edges[i]].x2-halfedge[edges[i]].x1;
		float diffy=halfedge[edges[i]].y2-halfedge[edges[i]].y1;
		float diffz=halfedge[edges[i]].z2-halfedge[edges[i]].z1;
		result[i*3+0]=halfedge[edges[i]].x1+diffx*(z[j]-halfedge[edges[i]].z1)/diffz;
		result[i*3+1]=halfedge[edges[i]].y1+diffy*(z[j]-halfedge[edges[i]].z1)/diffz;
		result[i*3+2]=z[j];
	}
}
