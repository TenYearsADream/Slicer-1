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

__kernel void groupedge(__global float3 *vertex,__global uint3 *halfedge, float z0,float thick,__global int *buf)
{
    int i = get_global_id(0);
    float zmax=fmax(vertex[halfedge[i].x].z,vertex[halfedge[i].y].z);
    float zmin=fmin(vertex[halfedge[i].x].z,vertex[halfedge[i].y].z);
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
		buf[i*3+2]=halfedge[i].z;
	}

}


__kernel void calalledges(__global float3 *vertex,__global uint3 *halfedge,__global uint *edges,__global float3 *result,__global float *z,__global uint *linesnumber)
{
	int i=get_global_id(0);
	int j=get_global_id(1);
	if(linesnumber[j-1]<=i && i<linesnumber[j])
	{
		float3 v1=vertex[halfedge[edges[i]].x];
		float3 v2=vertex[halfedge[edges[i]].y];
		float diffx=v2.x-v1.x;
		float diffy=v2.y-v1.y;
		float diffz=v2.z-v1.z;
		result[i].x=v1.x+diffx*(z[j]-v1.z)/diffz;
		result[i].y=v1.y+diffy*(z[j]-v1.z)/diffz;
		result[i].z=z[j];
	}
}

__kernel void intersect(__global float3 *vertex,__global uint3 *halfedge,float zheight,__global float4 *result)
{
	int i = get_global_id(0);
    float zmax=fmax(vertex[halfedge[i].x].z,vertex[halfedge[i].y].z);
    float zmin=fmin(vertex[halfedge[i].x].z,vertex[halfedge[i].y].z);
	if(zmin<=zheight && zheight<zmax)
	{
		float diffx=vertex[halfedge[i].x].x-vertex[halfedge[i].y].x;
		float diffy=vertex[halfedge[i].x].y-vertex[halfedge[i].y].y;
		float diffz=vertex[halfedge[i].x].z-vertex[halfedge[i].y].z;
		result[i].x=vertex[halfedge[i].x].x+diffx*(zheight-vertex[halfedge[i].x].z)/diffz;
		result[i].y=vertex[halfedge[i].x].y+diffy*(zheight-vertex[halfedge[i].x].z)/diffz;
		result[i].z=zheight;
		result[i].w=halfedge[i].z;
	}
	else
	{
		result[i].w=-1;
	}
}