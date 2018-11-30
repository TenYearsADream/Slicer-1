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

__kernel void capbyedge(__global float *A, __global float *B, __global float *C,__global float *z)
{
    size_t xsize=get_global_size(0);
    size_t ysize=get_global_size(1);
    size_t zsize=get_global_size(2);

    int row = get_global_id(0);
    int col = get_global_id(1);
    int dep = get_global_id(2);
    
    float diffx = B[row*ysize*zsize+col*3]-A[row*ysize*zsize+col*3];
    float diffy = B[row*ysize*zsize+col*3+1]-A[row*ysize*zsize+col*3+1];
    float diffz = B[row*ysize*zsize+col*3+2]-A[row*ysize*zsize+col*3+2];

    C[row*ysize*zsize+col*3] = A[row*ysize*zsize+col*3]+diffx*(z[row]-A[row*ysize*zsize+col*3+2])/diffz;
    C[row*ysize*zsize+col*3+1] = A[row*ysize*zsize+col*3+1]+diffy*(z[row]-A[row*ysize*zsize+col*3+2])/diffz;
    C[row*ysize*zsize+col*3+2] = z[row];
}

__kernel void groupedge(__global float *A, float z0,float thick,__global int *buf)
{
    int i = get_global_id(0);
    float zmax=fmax(A[7*i+2],A[7*i+5]);
    float zmin=fmin(A[7*i+2],A[7*i+5]);
	if((zmax-zmin)<1e-8)
	{
		buf[i*3+0]=2;
		buf[i*3+1]=0;
		buf[i*3+2]=0;
	}
	else
	{
		int num1=((zmin-z0)/thick)+1;
		int num2=((zmax-z0)/thick);
		buf[i*3+0]=num1;
		buf[i*3+1]=num2;
		buf[i*3+2]=A[7*i+6];
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
