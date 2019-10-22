#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>


struct timeval startwtime, endwtime;
double seq_time;


int N;          // data array size
int *a;         // data array to be sorted

const int ASCENDING  = 1;
const int DESCENDING = 0;


void init(void);
void print(double*);
void sort(void);
void test(void);
void exchange(int i, int j);
void compare(int i, int j, int dir);
int asc(const void * a, const void * b);
int desc(const void * a, const void * b);


/** the main program **/
#define REPS 10
int main(int argc, char **argv) {


  a = (int *)malloc(sizeof(int));
  double max[9],min[9];

  double times[REPS][9];
  double impAvgTimes[9];
  double recAvgTimes[9];
  
  //run test for imperative
  for(int i=16;i<=24;i++) {
 	N = 1<<i;
	a = (int *)realloc(a, N * sizeof(int));
  	for(int j=0;j<REPS;j++) {
  	
		  init();

		  gettimeofday (&startwtime, NULL);
  		  qsort(a, N, sizeof(int), asc);
		  gettimeofday (&endwtime, NULL);

		  seq_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
					  + endwtime.tv_sec - startwtime.tv_sec);
			
			
		  times[j][i-16] = seq_time;
		  
		  //printf("Imperative wall clock time = %f\n", seq_time);
			
	  }
  }
  
  
  
  //discard max-min and find average
  for(int i=16;i<=24;i++) {
	  max[i-16] = times[0][i-16];
	  min[i-16] = times[0][i-16];
	  
	  for(int j=1;j<REPS;j++) {
	  	if(times[j][i-16]<min[i-16])
	  		min[i-16] = times[j][i-16];
	  	if(times[j][i-16]>max[i-16])
	  		max[i-16] = times[j][i-16];  	
	  }
  }
  		
  		
  for(int i=16;i<=24;i++) {
  	impAvgTimes[i-16] = 0;
  	for(int j = 0;j<REPS;j++) {
  		impAvgTimes[i-16] += times[j][i-16];
  	}
  	
  	impAvgTimes[i-16] -= max[i-16] + min[i-16];
  	impAvgTimes[i-16] /= REPS - 2;
  }
  
  
  printf("%%qsort Average times for q = 16 - 24\n");
  printf("qsort");
  print(impAvgTimes);

	
  free(a);

}

/** -------------- SUB-PROCEDURES  ----------------- **/

//compare functions for qsort
int asc(const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

int desc(const void * a, const void * b) {
   return -( *(int*)a - *(int*)b );
}

/** procedure test() : verify sort results **/
void test() {
  int pass = 1;
  int i;
  for (i = 1; i < N; i++) {
    pass &= (a[i-1] <= a[i]);
  }

  printf(" TEST %s\n",(pass) ? "PASSed" : "FAILed");

}


/** procedure init() : initialize array "a" with data **/
void init() {
  int i;
  for (i = 0; i < N; i++) {
    a[i] = rand() % N; // (N - i);
  }
}

/** procedure  print() : print array elements **/
void print(double *array) {
  printf(" = [");
  int i;
  for (i = 0; i < 9; i++) {
    printf("%f ", array[i]);
  }
  printf("];");
  printf("\n");
}


