
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

struct timeval startwtime, endwtime;
double seq_time;

int maxThreads;
int LOW;		// if array smaller than LOW, use qsort on subarrays

int N;          // data array size
int *a;         // data array to be sorted

const int ASCENDING  = 1;
const int DESCENDING = 0;


void init(void);
void print(double*);
//void sort(void);
int test(void);
void exchange(int i, int j);
void compare(int i, int j, int dir);
void bitonicMerge(int lo, int cnt, int dir);
//void recBitonicSort(int lo, int cnt, int dir);
//void impBitonicSort(void);

//added functions
void Psort();
void PrecBitonicSort(int lo, int cnt, int dir);
void PimpBitonicSort();
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
  char cilkWorkers[5];
  
  for(int t=1;t<=8;t++) {
  	  maxThreads = 1<<t;
	  sprintf(cilkWorkers, "%d", maxThreads);
	  
	  __cilkrts_end_cilk();  
	  if (0!=  __cilkrts_set_param("nworkers",cilkWorkers)) {
	  	printf("Failed to set more than %d workers\n",1<<(t-1));
	  	printf("Exit...\n");
	  	exit(1);
	  }
	  
	  for(int i=16;i<=24;i++) {
	 	N = 1<<i;
	 	LOW = N;
		a = (int *)realloc(a, N * sizeof(int));
	  	for(int j=0;j<REPS;j++) {
	  	
			  init();

			  gettimeofday (&startwtime, NULL);
			  PimpBitonicSort();
			  gettimeofday (&endwtime, NULL);
			  
			  //if(!test())
			  	//printf("The program failed for p = %d, q = %d ",i,t);

			  seq_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
						  + endwtime.tv_sec - startwtime.tv_sec);
				
				
			  times[j][i-16] = seq_time;
			  
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
	  
	  
	  //run test for recursive
	  for(int i=16;i<=24;i++) {
	 	N = 1<<i;
		a = (int *)realloc(a, N * sizeof(int));
	  	for(int j=0;j<REPS;j++) {
	  	
			  init();

			  gettimeofday (&startwtime, NULL);
			  Psort();
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
	  	recAvgTimes[i-16] = 0;
	  	for(int j = 0;j<REPS;j++) {
	  		recAvgTimes[i-16] += times[j][i-16];
	  	}
	  	
	  	recAvgTimes[i-16] -= max[i-16] + min[i-16];
	  	recAvgTimes[i-16] /= REPS - 2;
	  }
	  
	  //print results
	  printf("%%Imperative Average times for q = 16 - 24 with 2^%d workers\n",t);
	  printf("cilkImp%d",t);
	  print(impAvgTimes);
	  printf("\n");
	  printf("%%Recursive Average times for q = 16 - 24 with 2^%d workers\n",t);
	  printf("cilkRec%d",t);
	  print(recAvgTimes);
	  
	  printf("\n");
	  printf("\n");
  }
  
  
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
int test() {
  int pass = 1;
  int i;
  for (i = 1; i < N; i++) {
    pass &= (a[i-1] <= a[i]);
  }

  return pass;
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

/** function recBitonicSort() 
    first produces a bitonic sequence by recursively sorting 
    its two halves in opposite sorting orders, and then
    calls bitonicMerge to make them in the same order 
 **/
 
//******THIS FUNCTION IS NOT USED*******

/* 
void recBitonicSort(int lo, int cnt, int dir) {
  if (cnt>1) {
    int k=cnt/2;
    recBitonicSort(lo, k, ASCENDING);
    recBitonicSort(lo+k, k, DESCENDING);
    bitonicMerge(lo, cnt, dir);
  }
}
*/


//parallel version of recBitonicSort
void PrecBitonicSort(int lo, int cnt, int dir) {
  if (cnt>1) {
    int k=cnt/2;
    
    if(cnt>=LOW) {
    	cilk_spawn
    	PrecBitonicSort(lo, k, ASCENDING);
    	PrecBitonicSort(lo+k, k, DESCENDING);
    }
    else {
    	cilk_spawn
		qsort(a + lo, k, sizeof(int), asc);
    	qsort(a + lo + k, k, sizeof(int), desc);
    }
    cilk_sync;
    
    bitonicMerge(lo, cnt, dir);
  }
}


/** function sort() 
   Caller of recBitonicSort for sorting the entire array of length N 
   in ASCENDING order
 **/
 
//******THIS FUNCTION IS NOT USED*******

/* 
void sort() {
  recBitonicSort(0, N, ASCENDING);
}
*/

//parallel version of sort
void Psort() {
  PrecBitonicSort(0, N, ASCENDING);
}



/*
  imperative version of bitonic sort
*/
//******THIS FUNCTION IS NOT USED*******

/*
void impBitonicSort() {

  int i,j,k;
  
  for (k=2; k<=N; k=2*k) {
    for (j=k>>1; j>0; j=j>>1) {
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
*/


//parallel version of impBitonicSort
void PimpBitonicSort() {

  int i,j,k;
  
  for (k=2; k<=N; k=2*k) {
    for (j=k>>1; j>0; j=j>>1) {
      cilk_for (i=0; i<N; i++) {
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
