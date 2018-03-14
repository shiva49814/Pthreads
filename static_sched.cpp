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

// Global variables from the command line and the value of (b-a)/n
int sync, a, b, n, intensity, functionid, nbthreads; 



struct Params{
 int start, end;		//mode =0 for iteration level and =1 for thread level synchronisation
 double thread_sum = 0;		//used if it is a thread level synchronisation
 double *total_sum;		//used for both thread level(in the end to add to total) and iteration level(for each addition)
 float multiplier;
};

pthread_mutex_t mutexsum;	//Mutex lock for the sum


// Function that calculates the sum in each thread.
void *sum(void *arg){
struct Params *thread_data;
int begin, endindex;
float c,d;
double mulfactor;
thread_data = (struct Params *) arg;
begin = thread_data->start;
endindex = thread_data->end;
mulfactor = thread_data->multiplier;
//For iteration level synchronisation
 
if(sync == 0){
	switch(functionid){
	case 1:
		for(int i= begin; i< endindex;i++){
		c=a+(i+.5)*mulfactor;
		d=f1(c,intensity);
 		pthread_mutex_lock(&mutexsum);	
		*thread_data->total_sum = *thread_data->total_sum + (d*mulfactor);
  		pthread_mutex_unlock (&mutexsum);
		}
		
	break;
	case 2:
		
		for(int i= begin; i<endindex ;i++){
		c=a+(i+.5)*mulfactor;
		d=f2(c,intensity);
		pthread_mutex_lock(&mutexsum);		
		*thread_data->total_sum = *thread_data->total_sum + (d*mulfactor);
  		pthread_mutex_unlock (&mutexsum);
		}
		
	break;
	case 3:
		
		for(int i=begin; i<endindex; i++){
		c=a+(i+.5)*mulfactor;
		d=f3(c,intensity);
		pthread_mutex_lock(&mutexsum);		
		*thread_data->total_sum = *thread_data->total_sum + (d*mulfactor);
  		pthread_mutex_unlock (&mutexsum);
		}
		
	break;
	case 4:
		
		for(int i=begin; i<endindex; i++){
		c=a+(i+.5)*mulfactor;
		d=f4(c,intensity);
		pthread_mutex_lock(&mutexsum);		
		*thread_data->total_sum = *thread_data->total_sum + (d*mulfactor);
  		pthread_mutex_unlock (&mutexsum);
		}
		
	break;
	default : std::cerr << "Invalid Parameters passed" << std::endl;
	break;  
	}
			
  }
// for thread level synchronisation
else{
	switch(functionid){
	case 1:
		
		for(int i= begin; i< endindex;i++){
		c=a+(i+.5)*mulfactor;
		d=f1(c,intensity);		
		thread_data->thread_sum = thread_data->thread_sum + (d*mulfactor);
  		}
		
	break;
	case 2:
		
		for(int i= begin; i<endindex ;i++){
		c=a+(i+.5)*mulfactor;
		d=f2(c,intensity);
		thread_data->thread_sum = thread_data->thread_sum + (d*mulfactor);
  		}
		
	break;
	case 3:
		
		for(int i=begin; i<endindex; i++){
		c=a+(i+.5)*mulfactor;
		d=f3(c,intensity);
		thread_data->thread_sum = thread_data->thread_sum + (d*mulfactor);
  		}
		
	break;
	case 4:
		
		for(int i=begin; i<endindex; i++){
		c=a+(i+.5)*mulfactor;
		d=f4(c,intensity);		
		thread_data->thread_sum = thread_data->thread_sum + (d*mulfactor);
  		}
		
	break;
	default : std::cerr << "Invalid Parameters passed" << std::endl;
	break;  
	}
        pthread_mutex_lock(&mutexsum);		
	*thread_data->total_sum = *thread_data->total_sum + thread_data->thread_sum;
  	pthread_mutex_unlock (&mutexsum);
  } 
     
   pthread_exit(NULL);  
}
  


int main (int argc, char* argv[]) {
  int nperthread;
  double TotalSum=0;
  double mulfactor;  
  pthread_attr_t attr;
  
  if (argc < 8) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <sync>"<<std::endl;
    return -1;
  }
  
  else if (argc == 8){
  functionid = atoi(argv[1]);
  a = atoi(argv[2]);
  b = atoi(argv[3]);
  n = atoi(argv[4]);
  intensity = atoi(argv[5]);
  nbthreads = atoi(argv[6]);
  //std::string mode(argv[7]);
  if(std::strcmp(argv[7], "iteration") == 0 )   
	sync = 0; 		//sync = 0 ==> iteration level synchronisation
  else 
	sync = 1; 		//sync = 1 ==> thread level synchronisation
  
  }
  
  else 
	std::cerr<<"usage: " << argv[0]<< " Excessive number of arguments"<<std::endl;



  mulfactor = (double) (b-a)/n;
  nperthread = n/nbthreads;
  pthread_t threadsum[nbthreads];
  struct Params parameters[nbthreads];  
  pthread_mutex_init(&mutexsum,NULL);  
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  for(int i=0; i<nbthreads; i++){
   	parameters[i].start = i * nperthread;
	if(i == nbthreads-1)  parameters[i].end = n; 
  	else parameters[i].end = (i+1) * nperthread;
        parameters[i].total_sum = &TotalSum;
   	parameters[i].thread_sum = 0;  
	parameters[i].multiplier = mulfactor;
   	pthread_create(&threadsum[i], &attr, sum, (void *) &parameters[i]);
  
  }

  pthread_attr_destroy(&attr);

  for(int i=0; i<nbthreads; i++){
  pthread_join(threadsum[i], NULL);
  }
 
 pthread_mutex_destroy(&mutexsum);

  std::cout<< TotalSum << std::endl;

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cerr<< elapsed_seconds.count() << "s\n";

  return 0;
}
