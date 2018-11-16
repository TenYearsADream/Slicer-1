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
