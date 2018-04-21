#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <pthread.h>

#define MAX_SIZE (512*1024*1024)

void *Trojan(){
  printf("Trojan\n");
}

void *Spy(){
  printf("Spy\n");
}

void start(int *arr, int *arr2){
  pthread_t trojan;
  pthread_t spy;

  printf("Begin\n");
  if(pthread_create(&trojan, NULL, Trojan, arr) || pthread_create(&spy, NULL, Spy, arr2)){
    fprintf(stderr, "Error creating thread\n");
    return;
  }

  pthread_join(trojan, NULL);
  pthread_join(spy, NULL);



}

int main(void)
{
  // clock_t start, end;
  double total_time;
  int i = 0;
  int k = 0;
  int count = 0;

  int *arr = (int*)malloc(MAX_SIZE * sizeof(int));
  int *arr2 = (int*)malloc(MAX_SIZE * sizeof(int));

  start(arr, arr2);

  // // for(k = 0; k < 3; k++){
  //   start = clock();
  //   count = 0;
  //   for (i = 0; i < MAX_SIZE; i++){
  //     arr[i] += 1;
  //     printf("Address %d\n", *arr[i]);
  //   }
  //
  //   end = clock();
  //
  //   total_time = ((double)(end-start))/CLOCKS_PER_SEC;
  //   printf("cpu time for loop 1 (k : %4d) %.1f ms.\n",k,(total_time*1000));
  // }
}
