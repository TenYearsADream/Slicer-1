#pragma OPENCL EXTENSION cl_amd_printf : enable

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
/* 	if(i>=0)
	{
		printf("i: %d\n",i);
		result[i].x=i;
		result[i].y=2*i;
		result[i].z=3*i;
		
	} */
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

void hashInsert(__global int4 *hashTable,int key,int value,uint length,uint hashoffset)
{
	int4 tmphash=(int4)(0);
	uint hashAddr =key % length;
	for(uint k=0;k<length;k++)   
	{
		tmphash=hashTable[hashoffset+hashAddr];
/* 		printf("key :%d tmphash.x: %d\n",key,tmphash.x);
		printf("bool : %d\n",tmphash.x!=key); */
		if(tmphash.x!=-2 && (tmphash.x!=key/length &&  tmphash.w!=key%length))  
		{				
			hashAddr =(hashAddr+1)% length; 
		}
		else
		{
			break;
		}	
	}
/* 	if(key==74894 || key==74986 )
	{
		printf("key:%d, value:%d, hashAddr: %d, offset: %d, hashAddr+offset: %d\n",key,value,hashAddr,hashoffset,hashAddr+hashoffset);
	} */
	if(tmphash.x==-2)
	{
		hashTable[hashoffset+hashAddr].x = key/length;
		hashTable[hashoffset+hashAddr].y = value;
		hashTable[hashoffset+hashAddr].w = key%length;
	}
	else
	{
		hashTable[hashoffset+hashAddr].z = value;
	}	
}

int hashSearch(__global int4 *hashTable,int key,uint length,uint hashoffset)
{
	uint hashAddr =key % length;
	while(hashTable[hashoffset+hashAddr].x!=key/length && hashTable[hashoffset+hashAddr].w!=key%length)
    {
        hashAddr =(hashAddr+1) % length;
		/* printf("key:%d,hashTable[hashoffset+hashAddr].x:%d, hashAddr:%d\n",key,hashTable[hashoffset+hashAddr].x,hashAddr); */
        if(hashTable[hashoffset+hashAddr].x==-2 || hashAddr == key % length)
        {
            return -1;
        }
    }
	return hashAddr;
}

__kernel void hashfind(const __global uint *edgebuf,const __global uint *linesnumber,__global int4 *hashTable,
					   __global uint *location,__global uint *loopcount,__global uint *loopnumber)
{
	int i = get_global_id(0);
	int edgewidth=0,edgeoffset=0;
	uint loopcounttmp=0,loopnumbertmp=0;
	uint length=0,hashoffset=0;
	int key=0,key1=0;
	uint loops=100000;
	/* if(i==0) */
	{
		if(i==0)
		{
			edgeoffset=0;
			hashoffset=0;
		}
		else 
		{
			edgeoffset=linesnumber[i-1];
			hashoffset = linesnumber[i-1];
		}
		edgewidth=linesnumber[i]-edgeoffset;
		length =edgewidth;
		/* printf("width of layer %d: %d\n",i,edgewidth); */
		for(int j=0;j<edgewidth;j+=2)
		{
			key=edgebuf[edgeoffset+j];
			key1=edgebuf[edgeoffset+j+1];
			if(key>=0 && key1>=0)
			{
				/* printf("---key1 : %d, value : %d---key2 : %d, value : %d\n",key,j,key1,j+1); */
				hashInsert(hashTable,key,j,length,hashoffset);
				hashInsert(hashTable,key1,j+1,length,hashoffset);
			}
		}
/* 		if(i<2)
		{
			printf("elements in hashTable of layer %d : %d\n",i,length);
			for(int k=0;k<length;k++)
			{
				if((hashTable[hashoffset+k].y==-2 || hashTable[hashoffset+k].z==-2) && hashTable[hashoffset+k].x!=-2)
				{
				printf("%d layer hashTable[%d]: %d : %d,%d\n",i,hashoffset+k,hashTable[hashoffset+k].x,hashTable[hashoffset+k].y,hashTable[hashoffset+k].z);
				}
			}
		} */
/* 		for(int j=0;j<edgewidth;j++)
		{
			int ret=hashSearch(hashTable,edgebuf[edgeoffset+j],length,hashoffset);
			if(ret == -1)
			{
				printf("search %d failed.\n",edgebuf[edgeoffset+j]);
			}
			else
			{
				printf("search %d : %d,%d\n",hashTable[hashoffset+ret].x,hashTable[hashoffset+ret].y,hashTable[hashoffset+ret].z);
			}
		} */
		int index=1,current=0,num=-1,start=1;
		for(int k=0;k<length;k++)
		{
			/* printf("%d : %d,%d\n",hashTable[hashoffset+k].x,hashTable[hashoffset+k].y,hashTable[hashoffset+k].z); */
			if(hashTable[hashoffset+k].y!=-2)
			{
				loopnumbertmp=0;
				index = hashTable[hashoffset+k].y;
				start = hashTable[hashoffset+k].y;
				current= current=edgebuf[edgeoffset+index];
				num++;
				/* printf("current: %d -- start: %d--num: %d\n",current,start,num); */
				location[edgeoffset+num]=index;
				loopnumbertmp++;
				for(int iter=0;iter<length;iter++)
				{
					int ret=hashSearch(hashTable,current,length,hashoffset);			
					if(ret == -1)
					{
						/* printf("search %d failed.\n",current); */
						break;
					}
					/* printf("search %d : %d,%d\n",current,hashTable[hashoffset+ret].y,hashTable[hashoffset+ret].z); */
					if(hashTable[hashoffset+ret].y!=index)
					{
						index=hashTable[hashoffset+ret].y;
					}
					else
					{
						index=hashTable[hashoffset+ret].z;
					}
					if(index<0)
					{
						/* printf("index= %d\n",index); */
						break;
					}
					if((index & 1) == 0)
					{
						index +=1;
					}
					else{
						index -=1;
					}
					/* printf("index= %d\n",index); */
					if(start!=index && index>=0)
					{
						hashTable[hashoffset+ret].y=-2;
						hashTable[hashoffset+ret].z=-2;
						num++;
						current=edgebuf[edgeoffset+index];
						/* printf("current: %d -- index: %d--num: %d\n",current,index,num); */
						location[edgeoffset+num]=index;
						loopnumbertmp++;						
					}
					else if(start==index)
					{
						hashTable[hashoffset+ret].y=-2;
						hashTable[hashoffset+ret].z=-2;
						num++;
						/* printf("current: %d -- index: %d--num: %d\n",current,index,num); */
						location[edgeoffset+num]=index;
						loopnumbertmp++;	
						break;
					} 
				}

				if(loopnumbertmp>2)
				{
					loopcounttmp++;	
					loopnumber[i*loops+loopcounttmp-1]=loopnumbertmp;
				}
			}
		}
		/* printf("layer %d :%d\n",i,loopcounttmp); */
		/* printf("\n"); */
		loopcount[i]=loopcounttmp;
	}
	
}