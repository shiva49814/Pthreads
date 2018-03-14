#include <iostream>
#include <pthread.h>
#include <chrono>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

float f1(float x, int intensity);
float f2(float x, int intensity);
float f3(float x, int intensity);
float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif

int n, granularity, length, sync, funid;
int current = 0;
float totalsum = 0;
int inten;
float mul;
float a2,b2;

pthread_mutex_t mutexsum;	//Mutex lock for the sum.
pthread_mutex_t nxt_val; 	//Mutex Lock for next val.


struct Params{
float aa, bb;

};



int * get_Next(){
static int a[2];
if(current < n){
a[0] = current;
a[1] = current + granularity;
if (a[1] > n)	a[1] = n+1;
current = current+ granularity;
}
pthread_mutex_unlock(&nxt_val);	
return a;
}



int done(){
int d;
pthread_mutex_lock(&nxt_val);
if ( current < n) 	d = 0;
else if (current >= n){
d = 1;
pthread_mutex_unlock(&nxt_val);
}
return d;
}  


void *sum(void *arg){
struct Params *thread_data;
int begin, endindex;
float thread_sum = 0;
float c,d;

while(done() == 0){
int begin, end;
int *p;
p = get_Next();
begin = *p;
end = *(p+1); 

//Iteration level
	 if(sync == 0){

		for(int i=begin; i<end; i++){
		c=a2+(float)(i+.5)*mul;
		if(funid == 1)		d=f1(c,inten);
		else if(funid == 2)	d=f2(c,inten);
		else if(funid == 3)	d=f3(c,inten);
		else 			d=f4(c,inten);	
		pthread_mutex_lock(&mutexsum);		
		totalsum = totalsum + (d*mul);
  		pthread_mutex_unlock (&mutexsum);
		}
	}

//Chunk Level
	 else if(sync == 1){
		float chunksum = 0;
		for(int i=begin; i<end; i++){
		c=a2+(float)(i+.5)*mul;
		if(funid == 1)		d=f1(c,inten);
		else if(funid == 2)	d=f2(c,inten);
		else if(funid == 3)	d=f3(c,inten);
		else 			d=f4(c,inten);	
		chunksum = chunksum + (d*mul);
		}
	pthread_mutex_lock(&mutexsum);		
	totalsum = totalsum + chunksum;
  	pthread_mutex_unlock (&mutexsum);
	}

//Thread level Synchronisation.
	else if (sync == 2){
		for(int i=begin; i<end; i++){
		c=a2+(float)(i+.5)*mul;
		if(funid == 1)		d=f1(c,inten);
		else if(funid == 2)	d=f2(c,inten);
		else if(funid == 3)	d=f3(c,inten);
		else 			d=f4(c,inten);
		thread_sum = thread_sum + (d*mul);
	    	}
  	}  
}

	if(sync ==2){
        pthread_mutex_lock(&mutexsum);
	totalsum = totalsum + thread_sum;
  	pthread_mutex_unlock (&mutexsum);
	}
pthread_exit(NULL);
}


int main (int argc, char* argv[]) {
  float a, b, mulfactor;
  int nbthreads, functionid, length, intensity;

  pthread_attr_t attr;


  if (argc < 9) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <sync> <granularity>"<<std::endl;
    return -1;
  }
  else if (argc == 9){
  funid = atoi(argv[1]);
  a2 = atof(argv[2]);
  b2 = atof(argv[3]);
  n = atoi(argv[4]);
  inten = atoi(argv[5]);
  nbthreads = atoi(argv[6]);
  if(std::strcmp(argv[7], "iteration") == 0)   
	sync = 0; 		//sync = 0 ==> iteration level synchronisation--> Adding the value in the most inner loop.
  else if(std::strcmp(argv[7], "chunk") == 0)
	sync = 1; 		//sync = 1 ==> chunk level synchronisation--> Adding the value after each granular level execution of thread.
  else 
 	sync = 2;		//sync = 2 ==> thread level synchronisation--> Adding the value after the full thread is executed.
  granularity = atoi(argv[8]);
  }
  
  else 
	std::cerr<<"usage: " << argv[0]<< " Excessive number of arguments"<<std::endl;
  
  mul = (float) (b2-a2)/n;

  //Thread intialisation and related data initialisation.
  pthread_t threadsum[nbthreads];
  struct Params parameters[nbthreads];  
  pthread_mutex_init(&mutexsum, NULL);
  pthread_mutex_init(&nxt_val, NULL);  
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  for(int i=0; i<nbthreads; i++){
   	
   	pthread_create(&threadsum[i], &attr, sum, (void *) &parameters[i]);  
  }

  pthread_attr_destroy(&attr);

  for(int i=0; i<nbthreads; i++){
  pthread_join(threadsum[i], NULL);
  }

  pthread_mutex_destroy(&mutexsum);
  pthread_mutex_destroy(&nxt_val);

  std::cout<< totalsum << std::endl;

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cerr<< elapsed_seconds.count() << "s\n";
  return 0;
}
