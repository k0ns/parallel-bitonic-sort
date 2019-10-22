
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <unistd.h>


struct timeval startwtime, endwtime;
double seq_time;

int maxThreads;
int LOW;		// if array smaller than LOW, use qsort on subarrays


int N;          // data array size
int *a;         // data array to be sorted

const int ASCENDING  = 1;
const int DESCENDING = 0;


void init(void);
void print(void);
void test(void);
void exchange(int i, int j);
void compare(int i, int j, int dir);
void bitonicMerge(int lo, int cnt, int dir);

//added functions
void Psort();
void PrecBitonicSort(int lo, int cnt, int dir);
void PimpBitonicSort();
int asc(const void * a, const void * b);
int desc(const void * a, const void * b);

/** the main program **/ 
int main(int argc, char **argv) {

  if (argc != 3) {
    printf("Usage: %s q p\n  where n=2^q is problem size (power of two)\n  and t=2^p is the number of threads (power of two)\n", 
	   argv[0]);
    exit(1);
  }

  N = 1<<atoi(argv[1]);
  LOW = N;
  maxThreads = 1<<atoi(argv[2]);
  a = (int *) malloc(N * sizeof(int));
  
  
  
  init();

  gettimeofday (&startwtime, NULL);
  PimpBitonicSort();
  gettimeofday (&endwtime, NULL);

  seq_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
		      + endwtime.tv_sec - startwtime.tv_sec);

  printf("Parallel Imperative wall clock time = %f\n", seq_time);

  test();
  
  


  init();
  gettimeofday (&startwtime, NULL);
  Psort();
  gettimeofday (&endwtime, NULL);

  seq_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
		      + endwtime.tv_sec - startwtime.tv_sec);

  printf("Parallel Recursive wall clock time = %f\n", seq_time);
  
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


/** INLINE procedure exchange() : pair swap **/
inline void exchange(int i, int j) {
  int t;
  t = a[i];
  a[i] = a[j];
  a[j] = t;
}



/** procedure compare() 
   The parameter dir indicates the sorting direction, ASCENDING 
   or DESCENDING; if (a[i] > a[j]) agrees with the direction, 
   then a[i] and a[j] are interchanged.
**/
inline void compare(int i, int j, int dir) {
  if (dir==(a[i]>a[j])) 
    exchange(i,j);
}




/** Procedure bitonicMerge() 
   It recursively sorts a bitonic sequence in ascending order, 
   if dir = ASCENDING, and in descending order otherwise. 
   The sequence to be sorted starts at index position lo,
   the parameter cbt is the number of elements to be sorted. 
 **/
void bitonicMerge(int lo, int cnt, int dir) {
  if (cnt>1) {
    int k=cnt/2;
    int i;
    for (i=lo; i<lo+k; i++)
      compare(i, i+k, dir);
    bitonicMerge(lo, k, dir);
    bitonicMerge(lo+k, k, dir);
  }
}

//parallel version of recBitonicSort
void PrecBitonicSort(int lo, int cnt, int dir) {
  if (cnt>1) {
			int k=cnt/2;
	  	{
			#pragma omp task
			{
			if(cnt<LOW)
				qsort(a + lo, k, sizeof(int), asc);
			else 
				PrecBitonicSort(lo, k, ASCENDING);
			}
			#pragma omp task
			{
			if(cnt<LOW)
				qsort(a + lo + k, k, sizeof(int), desc);
			else
				PrecBitonicSort(lo+k, k, DESCENDING);
			}
			#pragma omp taskwait
		}
			bitonicMerge(lo, cnt, dir);
  }
}


//parallel version of sort
void Psort() {
  #pragma omp parallel num_threads(maxThreads)
  #pragma omp single nowait
  PrecBitonicSort(0, N, ASCENDING);
}


//parallel version of impBitonicSort
void PimpBitonicSort() {

  int i,j,k;
  
  omp_set_num_threads(maxThreads);
  
  for (k=2; k<=N; k=2*k) {
    for (j=k>>1; j>0; j=j>>1) {
  	#pragma omp parallel for schedule(guided)
      for (i=0; i<N; i++) {
	int ij=i^j;
	if ((ij)>i) {
	  if ((i&k)==0 && a[i] > a[ij]) 
	      exchange(i,ij);
	  if ((i&k)!=0 && a[i] < a[ij])
	      exchange(i,ij);
	}
      }
    }
  }
}
