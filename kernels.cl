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

__kernel void groupedge2(__global float *A, __global float *z,__global float *buf)
{
    int i = get_global_id(0);
    float zmax=fmax(A[7*i+2],A[7*i+5]);
    float zmin=fmin(A[7*i+2],A[7*i+5]);
	if(zmin<=z[0] && z[0]<zmax)
	{
		float diffx=A[i*7]-A[7*i+3];
    	float diffy=A[i*7+1]-A[i*7+4];
    	float diffz=A[i*7+2]-A[i*7+5];
		
		buf[i*4+0]=A[i*7]+diffx*(z[0]-A[7*i+2])/diffz;
       	buf[i*4+1]=A[i*7+1]+diffy*(z[0]-A[7*i+2])/diffz;
		buf[i*4+2]=z[0];
		buf[i*4+3]=A[7*i+6];
		
	}
	else
	{
		buf[i*4+0]=0;
       	buf[i*4+1]=0;
		buf[i*4+2]=0;
		buf[i*4+3]=-1;
	}	
}
__kernel void calalledges(__global edge *edges ,__global float *buf,__global float *z,__global unsigned int *linesnumber)
{
	int i=get_global_id(0);
	int j=get_global_id(1);
	if(linesnumber[j-1]<=i && i<linesnumber[j])
	{
		float diffx=edges[i].x2-edges[i].x1;
		float diffy=edges[i].y2-edges[i].y1;
		float diffz=edges[i].z2-edges[i].z1;
		buf[i*3+0]=edges[i].x1+diffx*(z[j]-edges[i].z1)/diffz;
		buf[i*3+1]=edges[i].y1+diffy*(z[j]-edges[i].z1)/diffz;
		buf[i*3+2]=z[j];
	}
}

__kernel void callayeredges(__global edge *edges ,__global float *resultbuf,float z)
{
	int i=get_global_id(0);
	float diffx=edges[i].x2-edges[i].x1;
	float diffy=edges[i].y2-edges[i].y1;
	float diffz=edges[i].z2-edges[i].z1;
	resultbuf[i*3+0]=edges[i].x1+diffx*(z-edges[i].z1)/diffz;
	resultbuf[i*3+1]=edges[i].y1+diffy*(z-edges[i].z1)/diffz;
	resultbuf[i*3+2]=z;
}
