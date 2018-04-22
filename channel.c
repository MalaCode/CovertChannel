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

// #define MAX_SIZE (512*1024*1024)
#define MAX_SIZE (512)
#define vStride 128
#define hStride 64

pthread_mutex_t S;
pthread_mutex_t T;

void *Trojan(void *arrg){
  cpu_set_t my_set;
  pthread_t thread;
  thread = pthread_self();
  CPU_ZERO(&my_set);       //clears the cpuset
  CPU_SET(2, &my_set); //set CPU 2 on cpuset
  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &my_set);
  clock_t start, end;
  double total_time;
  int i = 0;
  int k = 0;
  int p = 0;
  int count = 0;
  int * arr = (int *)arrg;


  for(k = 0; k < 14; k++){
    pthread_mutex_lock(&T);
    printf("Trojan\n");
    printf("Running on core: %d\n", sched_getcpu());
    start = clock();
    for (i = 0; i < MAX_SIZE; i++){
        repeat16(p = arr[p];)
    }
    end = clock();

    total_time = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("cpu time for loop 1 (k : %4d) %.1f ms.\n",k,(total_time*1000));
    pthread_mutex_unlock(&S);
  }
}

void *Spy(void *arrg){
  cpu_set_t my_set;
  pthread_t thread;
  thread = pthread_self();
  CPU_ZERO(&my_set);       //clears the cpuset
  CPU_SET(2, &my_set);     //set CPU 2 on cpuset
  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &my_set);
  clock_t start, end;
  double total_time;
  int i = 0;
  int k = 0;
  int p = 0;
  int count = 0;
  int * arr = (int *)arrg;

  for(k = 0; k < 14; k++){
    pthread_mutex_lock(&S);
    printf("Spy\n");
    printf("Running on core: %d\n", sched_getcpu());
    start = clock();
    count = 0;
    for (i = 0; i < MAX_SIZE; i++){
      repeat16(p = arr[p];)
    }

    end = clock();

    total_time = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("cpu time for loop 1 (k : %4d) %.1f ms.\n",k,(total_time*1000));
    pthread_mutex_unlock(&T);
  }
}

void start(int *arr, int *arr2){
  pthread_t trojan;
  pthread_t spy;
  int size = 1024;

  // sched_setaffinity(spy, sizeof(cpu_set_t), &my_set);
    for (int i=0;i<512;i++)
    {
        arr[i] = i+vStride;  //placing values into the array, values are used to make sure that the values is always on the same cache line
        if (arr[i] >= size) //check to see if the value exceeds the size of the array, so that we can move to the next cache line
          arr[i] %= vStride;	 //moding by the stride allows to to maintain the current cache line
    }
    for (int i=0;i<512;i++)
    {
        arr2[i] = i+vStride; //Same as above
        if (arr2[i] >= size)  //Same as above
          arr2[i]= (arr2[i] % vStride); //Same as above
    }

    for(int k = 0; k < 2; k++){
      printf("Begin\n");
      printf("Allocated Size: %li\n", MAX_SIZE*sizeof(int));
      if(pthread_create(&trojan, NULL, Trojan, (void*)arr) || pthread_create(&spy, NULL, Spy, (void*)arr2)){
        fprintf(stderr, "Error creating thread\n");
        return;
      }

      pthread_join(trojan, NULL);
      pthread_join(spy, NULL);
    }

}

int main(void)
{
  // clock_t start, end;
  // sched_setaffinity(getpid(), sizeof(my_set), &my_set);
  double total_time;
  int i = 0;
  int k = 0;
  int count = 0;

  int *arr = (int*)malloc(MAX_SIZE);
  int *arr2 = (int*)malloc(MAX_SIZE);

  pthread_mutex_lock(&T);
  start(arr, arr2);
}
