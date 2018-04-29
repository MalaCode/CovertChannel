#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <pthread.h>
#include <errno.h>
#include "repeat.h"
#include "rdtsc.h"

// #define MAX_SIZE (512*1024*1024)
#define MAX_SIZE (8192)
#define vStride 1024
#define hStride 64

pthread_mutex_t S;
pthread_mutex_t T;

void *Trojan(void *arrg){
  cpu_set_t my_set;
  pthread_t thread;
  thread = pthread_self();
  CPU_ZERO(&my_set);       //clears the cpuset
  CPU_SET(7, &my_set); //set CPU 2 on cpuset
  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &my_set);

  clock_t start, end;
  double total_time;
  int i = 0;
  int k = 0;
  int p = 16;
  int count = 0;
  int * arr = (int *)arrg;


  for(k = 0; k < 10; k++){
    pthread_mutex_lock(&T);
    printf("Trojan: ");
    if (k%2 == 0){
      // printf("Transmitting 0\n");
      printf("0\n");
    }
    else{
      // printf("Running on core: %d\n", sched_getcpu());
      start = rdtsc();
      for (i = 0; i < MAX_SIZE; i++){
          repeat16(p = arr[p];)
      }
      end = rdtsc();
      total_time = ((double)(end-start))/CLOCKS_PER_SEC;
      // printf("Trojan Start: %.1f ms.\n",(double)(start*1000));
      // printf("Trojan End: %.1f ms.\n",(double)(end*1000));
      printf("\n");
    }

    pthread_mutex_unlock(&S);
  }
}

void *Spy(void *arrg){
  cpu_set_t my_set;
  pthread_t thread;
  thread = pthread_self();
  CPU_ZERO(&my_set);       //clears the cpuset
  CPU_SET(7, &my_set);     //set CPU 2 on cpuset
  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &my_set);
  clock_t start, end;
  double total_time;
  int i = 0;
  int k = 0;
  int p = 0;
  int count = 0;
  int * arr = (int *)arrg;

  for(k = 0; k < 10; k++){
    pthread_mutex_lock(&S);
    printf("Spy: ");
    // printf("Running on core: %d\n", sched_getcpu());
    start = clock();
    count = 0;
    for (i = 0; i < MAX_SIZE; i++){
      repeat16(p = arr[p];)
    }

    end = clock();

    total_time = ((double)(end-start))/CLOCKS_PER_SEC;
    // printf("Start: %.1f ms.\n",(double)(start*1000));
    // printf("End: %.1f ms.\n",(double)(end*1000));
    printf("Time: %.1f ms.\n",(double)(total_time*1000));

    pthread_mutex_unlock(&T);
  }
}

void start(int *arr, int *arr2){
  pthread_t trojan;
  pthread_t spy;
  int size = 1024*4;
  int* clearCache = (unsigned int*)malloc(MAX_SIZE*4);
  int temp = 0;

  for(int x = 0; x < size; x++){
    repeat64(temp = clearCache[temp];)
  }
  for (int i=0;i<size;i++)
  {
      arr[i] = i+vStride;  //placing values into the array, values are used to make sure that the values is always on the same cache line
      if (arr[i] >= size) //check to see if the value exceeds the size of the array, so that we can move to the next cache line
        arr[i] %= vStride;	 //moding by the stride allows to to maintain the current cache line
  }
  for (int i=0;i<size;i++)
  {
      arr2[i] = i+vStride; //Same as above
      if (arr2[i] >= size)  //Same as above
        arr2[i]= (arr2[i] % vStride); //Same as above
  }



  // for(int k = 0; k < 2; k++){
    printf("Begin\n");
    printf("Allocated Size: %li\n", MAX_SIZE*sizeof(int));
    if(pthread_create(&trojan, NULL, Trojan, (void*)arr) || pthread_create(&spy, NULL, Spy, (void*)arr2)){
      fprintf(stderr, "Error creating thread\n");
      return;
    }

    pthread_join(trojan, NULL);
    pthread_join(spy, NULL);
  // }

}

int main(void)
{

  double total_time;
  int i = 0;
  int k = 0;
  int count = 0;

  int *arr = (unsigned int*)malloc(MAX_SIZE*4);
  int *arr2 = (unsigned int*)malloc(MAX_SIZE*4);

  pthread_mutex_lock(&T);
  start(arr, arr2);
}
