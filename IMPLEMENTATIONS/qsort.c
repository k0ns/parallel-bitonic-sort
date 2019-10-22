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
void print(void);
void sort(void);
void test(void);
void exchange(int i, int j);
void compare(int i, int j, int dir);
int asc(const void * a, const void * b);
int desc(const void * a, const void * b);


/** the main program **/
int main(int argc, char **argv) {


  if (argc != 2) {
    printf("Usage: %s q\n  where n=2^q is problem size (power of two)\n",
	   argv[0]);
    exit(1);
  }
  
  
  N = 1<<atoi(argv[1]);
  a = (int *)malloc(N*sizeof(int));
  
  //time and test qsort
  init();
  gettimeofday (&startwtime, NULL);
  qsort(a, N, sizeof(int), asc);
  gettimeofday (&endwtime, NULL);

  seq_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
		      + endwtime.tv_sec - startwtime.tv_sec);

  printf("qsort wall clock time = %f\n", seq_time);
  test();
	
	
	
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

  if(!pass) {
    for(int i=0;i<N;i++) {
        printf("%d",i);
        printf("                    ");
        printf("%d\n",a[i]);

    }
  }
}


/** procedure init() : initialize array "a" with data **/
void init() {
  int i;
  for (i = 0; i < N; i++) {
    a[i] = rand() % N; // (N - i);
  }
}

/** procedure  print() : print array elements **/
void print() {
  int i;
  for (i = 0; i < N; i++) {
    printf("%d\n", a[i]);
  }
  printf("\n");
}


