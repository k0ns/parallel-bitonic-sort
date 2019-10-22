#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>


struct timeval startwtime, endwtime;
double seq_time;

int currentThreads = 0, maxThreads;

int N;          // data array size
int *a;         // data array to be sorted
pthread_mutex_t tC;

const int ASCENDING  = 1;
const int DESCENDING = 0;


void init(void);
void print(double*);
void sort(void);
int test(void);
void exchange(int i, int j);
void compare(int i, int j, int dir);
void bitonicMerge(int lo, int cnt, int dir);

//added functions
void *PrecBitonicSort(void *);
void Psort();
int asc(const void * a, const void * b);
int desc(const void * a, const void * b);

/**arguments structure**/
typedef struct {
	int lo;
	int cnt;
	int dir;
	}parm;

#define REPS 10
/** the main program **/
int main(int argc, char **argv) {
  
  a = (int *)malloc(sizeof(int));
  double max[9],min[9];

  double times[REPS][9];
  double recAvgTimes[9];
  
  pthread_mutex_init(&tC, NULL);
  
  for(int t=1;t<=8;t++) {
	  maxThreads = 1<<t; 
	  
	  //run test
	  for(int i=16;i<=24;i++) {
	 	N = 1<<i;
		a = (int *)realloc(a, N * sizeof(int));
	  	for(int j=0;j<REPS;j++) {
	  	
			  init();

			  gettimeofday (&startwtime, NULL);
			  Psort();
			  currentThreads = 0;
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
	  	recAvgTimes[i-16] = 0;
	  	for(int j = 0;j<REPS;j++) {
	  		recAvgTimes[i-16] += times[j][i-16];
	  	}
	  	
	  	recAvgTimes[i-16] -= max[i-16] + min[i-16];
	  	recAvgTimes[i-16] /= REPS - 2;
	  }
	  
	  printf("%%Recursive Average times for q = 16 - 24 with 2^%d threads\n",t);
	  printf("pThreads%d",t);
	  print(recAvgTimes);
	  printf("\n");
  }
  
  
  pthread_mutex_destroy(&tC);
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

//Parallel sort function
void Psort() {
    parm arg;
    arg.lo = 0;
    arg.cnt = N;
    arg.dir = ASCENDING;
    PrecBitonicSort((void *)&arg);
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


//parallel version of recBitonicSort
void *PrecBitonicSort(void *arg) {
	
  if (((parm* )arg)->cnt>1) {
    parm p = *(parm*)arg;
    int k=p.cnt/2;
    //check if allowed to create threads
    if(currentThreads <= maxThreads-2) {
    	
    	pthread_mutex_lock(&tC);
    	currentThreads += 2;
    	pthread_mutex_unlock(&tC);
    	
		//declare the 2 threads and their parameters
		pthread_t t1,t2;
		parm p1,p2;
		
		//set up parameters
		p1.lo = p.lo;
		p1.cnt = k;
		p1.dir = ASCENDING;

		p2.lo = p.lo + k;
		p2.cnt = k;
		p2.dir = DESCENDING;
		
		pthread_create(&t1, NULL, PrecBitonicSort,(void*)&p1);
		pthread_create(&t2, NULL, PrecBitonicSort,(void*)&p2);

		pthread_join(t1, NULL);
		pthread_join(t2, NULL);
    }
    //else use serial qsort
    else {
	    	qsort(a + p.lo, k, sizeof(int), asc);
    		qsort(a + p.lo + k, k, sizeof(int), desc);
    }
    
	parm l = *(parm*)arg;
	bitonicMerge(l.lo,l.cnt,l.dir);
  }
}
