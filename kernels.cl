__kernel void cap(__global float *A, __global float *B, __global float *C,__global float *z)
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