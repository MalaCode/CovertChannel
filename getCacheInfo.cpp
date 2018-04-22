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
#include <iostream>
using namespace std;

int main()
{
  clock_t start, end;
  int i = 200;
  int diff = 0;
  int past = 0;
  while(i < 256000){
    start = clock();
    int *arr = (int*)malloc(i);
    end = clock();
    diff = end-start;
    if(diff > past + 5){
      cout << "i: " << i << ", Time: ";
      cout << end-start << endl;
    }
    past = diff;
    i += 50;
  }
  return 0;
}
