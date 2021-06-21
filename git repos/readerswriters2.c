#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define COUNT 5 

//for controlling access

sem_t read_protect; 
sem_t write_protect;
sem_t item_protect; 

int item=0; // to keep track
int write_waiting=0; 
int read_waiting=0; 

key_t key; 
int shmid;


void* writer(void* tid) {
  int* id=(int*) tid;

  sem_wait(&write_protect);
  write_waiting++;

  sem_wait(&item_protect);  
  

    key = ftok("shmfile",65);
  
    shmid = shmget(key,1024,0666|IPC_CREAT);

    int *num = (int*) shmat(shmid,(void*)0,0);
  	
    item++;     // updating item
    *num = item;
    
    printf("Data written in memory: %d\n",*num);
      
    shmdt(num);
 
  sem_post(&item_protect);

  write_waiting--; 
  
  sem_post(&write_protect); 
  
  free(id);
}

void* reader(void* tid) {

  int* id=(int*) tid;

  sem_wait(&read_protect);

  while(write_waiting);
  read_waiting++;

  sem_wait(&item_protect);
 
 
    key = ftok("shmfile",65);
  
    shmid = shmget(key,1024,0666|IPC_CREAT);
  
    int *num = (int*) shmat(shmid,(void*)0,0);
  	
    printf("Data read from memory: %d\n",*num);	
  
  
  sem_post(&item_protect);

  read_waiting--;
 
  sem_post(&read_protect);

  free(id);
}


void main() {
 
  sem_init(&write_protect,0,1);
  sem_init(&read_protect,0,1);
  sem_init(&item_protect,0,1);


  pthread_t thread_no_read[COUNT]; 
  pthread_t thread_no_write[COUNT]; 

  //creating read & write threads
  for(int i=0;i<COUNT;i++) {
    int *r =(int*) malloc(sizeof(int));
    *r=i;
    int *w =(int*) malloc(sizeof(int));
    *w=i;
    pthread_create(&thread_no_read[i],NULL,&reader,(void*)r);
    pthread_create(&thread_no_write[i],NULL,&writer,(void*)w);
  }

  for(int i=0;i<COUNT;i++) {
    pthread_join(thread_no_read[i],NULL);
    pthread_join(thread_no_write[i],NULL);
  }


  sem_destroy(&read_protect);
  sem_destroy(&write_protect);
  shmctl(shmid,IPC_RMID,NULL);  //destroy shared memory
  
  
  pthread_exit(NULL);
}
