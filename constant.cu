
#include <stdio.h>
#include <stdlib.h>
#include "repeat.h"
#include "support.h"
#define CARRAY_SIZE 512  //2kb divided by 4

__constant__ unsigned int d_carray[CARRAY_SIZE];
__constant__ unsigned int d_carray1[CARRAY_SIZE];

unsigned int h_test[CARRAY_SIZE];
__device__ uint get_smid(void) {

       uint ret;
       asm("mov.u32 %0, %smid;" : "=r"(ret) );
       return ret;
}

__global__ void Spy (unsigned int *ts, unsigned int *out, int p1, int p2, int its2)
{//ts is the allocate memory on GPU, out is the output, p1 is 1, p2 is 3, its2 is 20
    int t1 = p1; int t2 = p1*p1; int t3 = p1*p1+p1; int t4 = p1*p1+p2; //t1 = 1, t2 = 1, t3 = 2, t4 = 4
    int start_time, end_time;

    unsigned int p;
    int p_start = 0;     
    p = p_start;
    start_time = clock(); //starts clock
                   
    for (int i=0;i<its2;i++) //occurs 20 times
    {
        repeat16(p = d_carray[p];) //repeats this 16x20 times, each time setting p = to an element in d_carray which contains the location-cache values, so p travels down the cache lines reading each value
				   //how does this read from the cache? is d_carray placed into the cache? I was under the impression that TS what the allocated cache memory on the GPU
    }
    
    end_time = clock(); //ends clock
    t1 = p; //sets t1 to end of the cache
    out[0] = t1+t2+t3+t4; //output is set to 4 + end of cache? why?
    if (threadIdx.x == 0)
    {
        ts[blockIdx.x * 2] = start_time; //Sets the 0th entry on the cache line to start time, also is this the handshake?
        ts[blockIdx.x * 2 + 1] = end_time; //Sets the 1st entry on the cache line to start time, also is this the handshake?
    }
//if(threadIdx.x==0)
//printf("1111 block %d smid %d\n", blockIdx.x, get_smid());
}


__global__ void Trojan (int k, unsigned int *ts, unsigned int *out, int p1, int p2, int its2)
{
  if (k %3 ==0 ){ //skips every third iteration, (why?)
  }
  else {
      int t1 = p1; int t2 = p1*p1; int t3 = p1*p1+p1; int t4 = p1*p1+p2; //t1 = 1, t2 = 1, t3 = 2, t5 = 4
      int start_time, end_time;

      unsigned int p;
      int p_start = 16;    //different starting place then spy,why?
      p = p_start;
      start_time = clock();
      for (int i=0;i<its2;i++)
      {
          repeat16(p = d_carray1[p];) //repeats 16x20 times, each time setting p equal to the next stride, however this starts at the 16th index of the array and travles from there, 
		  		      //how does this place values into the cache?
      }
      end_time = clock();		//end time
      t1 = p;				//t1 is end of the cache
      out[0] = t1+t2+t3+t4;		//output is set to 4 + end of cache? why?
  }

//if(threadIdx.x==0)
//printf("2222 block %d smid %d\n", blockIdx.x, get_smid());
}
void cmem_stride(unsigned int *h_carray,unsigned int *h_carray1, unsigned int *d_ts, unsigned int *d_out, unsigned int *ts, int stride, int min_size, int max_size, int step_size)
{   //h_carray is the spy allocation, h_carray1 is the trojan allocation, d_ts is the address of allocated memeory, d_out is where output will be, ts is output from kernel,
    //stride is num cacheset/size cache line, min_size is ?, max_size is ?, step_size is ?
    dim3 Db = dim3(256);	//why 256?, Creates a grid of size 256 x 1 x 1
    dim3 Dg = dim3(10,1,1);     //first param is the number of SMs, created a grid of 10 x 1 x 1
    dim3 Db1 = dim3(256);	//why 256?, creates a grid of size 256 x 1 x 1
    dim3 Dg1 = dim3(10,1,1);	//first param is the number of SMs, creates a grid of 10 x 1 x 1
    cudaStream_t stream3, stream1,stream2; //Creates stream in order to run concurrently
    cudaStreamCreate(&stream3);	//Create stream3
    cudaStreamCreate(&stream1); //create steam1
    cudaStreamCreate(&stream2); //create stream2
    cudaError_t errcode;
    Timer timer;
    int size = 512;		//Why is this constant? shouldn't it be the number of sets?
    printf ("Constant memory, %d-byte stride\n", stride*4);   //number of cache sets
    printf ("  [array size]: [clocks per read], [max], [min]\n");
    
    // Set up array contents
    for (int i=0;i<size;i++)
    {
        h_carray[i] = i+stride;  //placing values into the array, values are used to make sure that the values is always on the same cache line
        if (h_carray[i] >= size) //check to see if the value exceeds the size of the array, so that we can move to the next cache line
        h_carray[i] %= stride;	 //moding by the stride allows to to maintain the current cache line
    }
    for (int i=0;i<size;i++)
    {
        h_carray1[i] = i+stride; //Same as above
        if (h_carray1[i] >= size)  //Same as above
        h_carray1[i]= (h_carray1[i] % stride); //Same as above
    }
    cudaMemcpyToSymbol(d_carray, h_carray, CARRAY_SIZE*4); //copies CARRAY_SIZE*4 bytes of h_carray into d_carray
    cudaMemcpyToSymbol(d_carray1, h_carray1, CARRAY_SIZE*4); //copies CARRAY_SIZE*4 bytes of h_carray1 into d_carray1
    unsigned long long sum_time[14] ;
    
    for(int j=0; j < 14; j++){		//empties the array used to hold the sum_times
      sum_time [j] = 0;			
    }	
    unsigned int max_time[14], min_time[14];     //=(unsigned)-1;
    for(int j=0; j < 14; j++){
      max_time [j] = 0;
    }
    for(int j=0; j < 14; j++){
    min_time [j] =(unsigned)-1;
    }

    int kits = 50;    	//Why this value?
    int its = 20;	//Why this value?

    for (int k = 0; k < kits; k++) //runs 50 times
    {
        startTime(&timer); //starts time
        // Launch kernel

        Spy<<<Dg, Db, 0, stream1>>> (d_ts, d_out, 1,3, its); //launches 256 blocks with 10 threads each with a shared memory of 0, spy is passed the allocated memory, memory for output, 1 (?), 3(?), and its
        Trojan<<<Dg1, Db1, 0, stream2>>> (k, d_ts, d_out, 1,3, its); //launches 256 blocks with 10 threads each with a shared memory of 0, trojan is passed k, allocated memory, memory for output, 1(?), 2(), its
       
        cudaThreadSynchronize();				//makes sure that the trojan and spy are done befor emoving to next iteration

        stopTime(&timer); printf("eennnnddd %f s\n", elapsedTime(timer)); // ends the timer for both spy and trojan
        cudaMemcpy(ts, d_ts, 640, cudaMemcpyDeviceToHost);		//copies 640 bytes of d_ts to ts
        for(int j=0; j < 10; j++){
    //        sum_time[j] += ts[2*j+1]-ts[2*j];
  //          if (ts[2*j+1]-ts[2*j] > max_time[j]) max_time[j] = ts[2*j+1]-ts[2*j];
//            if (ts[2*j+1]-ts[2*j] < min_time[j]) min_time[j] = ts[2*j+1]-ts[2*j];
            printf (" k = %d: latency %.3f clk\n", k,(ts[2*j+1]-ts[2*j])/(its * 16.0)); 
        }

    }
    /*for(int i=0; i < 14; i++){
    printf ("  %d: %.3f, %.3f, %.3f clk\n", size*4, 
    sum_time[i]/(kits*its*4.0),
    min_time[i]/(its*4.0),
    max_time[i]/(its*4.0));
    }*/
    printf ("\n");
}


int main()
{
    unsigned int ts[4096];  // ts, output from kernel. Two elements used per thread.
    unsigned int *d_ts;	   //used to hold the address of the allocated memory on device
    unsigned int *d_out;  // Unused memory for storing output
    unsigned int *h_carray; //memory allocation for spy
    unsigned int *h_carray1; //memory allocation for trojan

 int nDevices;
         cudaGetDeviceCount(&nDevices);
	         for (int i = 0; i < nDevices; i++) {
			           cudaDeviceProp prop;
				             cudaGetDeviceProperties(&prop, i);
					                    //cudaSetDevice(1);
							              printf("Device Number: %d\n");
								                printf("  Device name: %s\n", prop.name);
									                  printf("  total Global memory (in bytes): %d\n",
												                    prop.totalGlobalMem);
										           printf("  total shared memory per block (in bytes): %d\n",
												                   prop.sharedMemPerBlock);
										           printf("  Register per block: %d\n",
												                  prop.regsPerBlock);
										           printf(" warp size: %d\n",
												                  prop.warpSize);
										           printf(" maxThreadsPerBlock: %d\n",
												                  prop.maxThreadsPerBlock);
										           printf(" maxThreadsDim1: %d\n",
												                  prop.maxThreadsDim[0]);
										           printf(" maxThreadsDim2: %d\n",
												                  prop.maxThreadsDim[1]);
										           printf(" maxThreadsDim3: %d\n",
												                  prop.maxThreadsDim[2]);
										           printf(" maxGridsize1: %d\n",
												                  prop.maxGridSize[0]);
										           printf(" maxGridsize2: %d\n",
												                  prop.maxGridSize[1]);
										           printf(" maxGridsize3: %d\n",
												                  prop.maxGridSize[2]);
										           printf(" clockrate: %d\n",
												                  prop.clockRate);
										           printf(" const memory: %d\n",
												                  prop.totalConstMem);
										           printf(" device overlap: %d\n",
												                  prop.deviceOverlap);
										           printf("multiProcessorCount : %d\n",
												                  prop.multiProcessorCount);
										           printf("integrated : %d\n",
												                  prop.integrated);
										
										           printf("concurrentKernels: %d\n",
												                  prop.concurrentKernels);
										           printf("asyncEngineCount: %d\n",
												                  prop.asyncEngineCount);
										           printf("maxThreadPerMultiProcessor: %d\n",
												                  prop.maxThreadsPerMultiProcessor);
										           printf(" L2 cache size (in bytes): %d\n",
												                  prop.l2CacheSize);
										           printf(" minor: %d\n",
												                  prop.minor);
										           printf(" major: %d\n", 
														                     prop.major);


													           }
														 
																																												       		  // Allocate device array.
    cudaError_t errcode;
    if (cudaSuccess != (errcode = cudaMalloc((void**)&d_ts, sizeof(ts)))) //attempting to allocate memory on GPU
    {
        printf ("cudaMalloc failed %s:%d\n", __FILE__, __LINE__);
        printf ("   %s\n", cudaGetErrorString(errcode));
        return -1;
    }
    if (cudaSuccess != cudaMalloc((void**)&d_out, 4))			  //attempting to allocate 4 bytes of memory, (why?)
    {
        printf ("cudaMalloc failed %s:%d\n", __FILE__, __LINE__);
        return -1;
    }
    h_carray = (unsigned int*)malloc(CARRAY_SIZE*4);  //Size of the cache line * size of cache set
    h_carray1 = (unsigned int*)malloc(CARRAY_SIZE*4); //Size of the cache line * size of cache set


    // Stride 16 L1
    cmem_stride(h_carray,h_carray1, d_ts, d_out, ts, 512/4, 512-64, 2048+192, 16/4); //first num is the number of cache sets divded by the size of each cache line

    //free memory
    cudaFree(d_ts);
    cudaFree(d_out);
    free(h_carray);
        free(h_carray1);
    return 0;
}
void startTime(Timer* timer) {
    gettimeofday(&(timer->startTime), NULL);
}

void stopTime(Timer* timer) {
    gettimeofday(&(timer->endTime), NULL);
}

float elapsedTime(Timer timer) {
    return ((float) ((timer.endTime.tv_sec - timer.startTime.tv_sec) \
                + (timer.endTime.tv_usec - timer.startTime.tv_usec)/1.0e6));
}

