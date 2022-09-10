#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <linux/input.h>
#include "thread_model.h"

//GLOBAL VARIABLES AND PTHREADS DECLARATION
int flag = 1;

//Thread IDs for threads, calling thread & keyboard interrupt
pthread_t thread_id[NUM_THREADS];	 
pthread_t main_id;
pthread_t keyboard_event_id; 
pthread_attr_t attr;

//barrier synchronization
pthread_barrier_t barrier;

pthread_mutex_t mutex[NUM_MUTEXES] = {PTHREAD_MUTEX_INITIALIZER};

pthread_cond_t cond_var[10]; 
pthread_mutex_t cond_mutex[10] = PTHREAD_MUTEX_INITIALIZER; 

//FIFO Scheduling
struct sched_param mparam; 

void compute_func(struct Tasks periodic_thread)
{
	volatile int x;
	x = periodic_thread.loop_iter[0];
    while(x > 0)   
		x--;
    pthread_mutex_lock(&mutex[periodic_thread.mutex_num]);
    x = periodic_thread.loop_iter[1];
    while(x > 0) 
		x--;
    pthread_mutex_unlock(&mutex[periodic_thread.mutex_num]);
    x = periodic_thread.loop_iter[2];
    while(x > 0)    
		x--;
}

//Aperiodic Task
void *aperiodic_function(void *args)
{
    struct Tasks task_thread = *((struct Tasks *)(args)); 
    pthread_barrier_wait(&barrier);
    pthread_cond_wait(&cond_var[task_thread.event_key], &cond_mutex[task_thread.event_key]);
    while(flag)
    {
        compute_func(task_thread);
        pthread_cond_wait(&cond_var[task_thread.event_key], &cond_mutex[task_thread.event_key]);
    }
    pthread_exit(NULL);
}

void *keyboard_event_func(void *args)
{
    int fd;
    struct input_event keyboard_event; 
    fd = open(KEYBOARD_EVENT_DEV, O_RDONLY);
    pthread_barrier_wait(&barrier);
    while(flag == 1) 
    {
        if(read(fd, &keyboard_event, sizeof(struct input_event)))
        {
            if(keyboard_event.type ==1 && keyboard_event.code ==11 && keyboard_event.value ==0)  {
                pthread_cond_broadcast(&cond_var[0]); 
            }
            for(int i = 1; i<9; i++){
                if(keyboard_event.type == 1 && keyboard_event.code == (i+2) && keyboard_event.value == 0){
                    pthread_cond_broadcast(&cond_var[i+1]); 
                }
            }
        }
    }
    pthread_exit(NULL);
	close(fd);
}

//Periodic Task
void *periodic_function(void *args)
{
    struct Tasks task_thread = *((struct Tasks *)(args));
    struct timespec next, period; 
    period.tv_nsec = task_thread.period * 1000000;
    pthread_barrier_wait(&barrier);
    clock_gettime(CLOCK_MONOTONIC, &next);
    while(flag == 1)
    {
        compute_func(task_thread);
        next.tv_sec = next.tv_sec + ((next.tv_nsec + period.tv_nsec)/1000000000);
        next.tv_nsec = (next.tv_nsec + period.tv_nsec)%1000000000;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, 0);
    }
    pthread_exit(NULL);
}

	
int main()
{
    main_id = pthread_self();
    mparam.sched_priority = 99;
    pthread_setschedparam(main_id, SCHED_FIFO, &mparam);

    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_barrier_init(&barrier, NULL, (NUM_THREADS + 1));

    for(int i=0; i<10;i++)
    {
        pthread_cond_init(&cond_var[i], NULL); //Conditional Var for each event
    }

	for(int i=0; i<NUM_THREADS; i++)
	{
        mparam.sched_priority = threads[i].priority;
        pthread_attr_setschedparam(&attr, &mparam);
		//Task Type 0 for periodic and 1 for aperiodic task
		if(threads[i].task_type == 1)
        {
			pthread_create(&thread_id[i], &attr, aperiodic_function, &threads[i]);
        }
        else
        {
		 	pthread_create(&thread_id[i], &attr, periodic_function, &threads[i]);
        }
    }

    //Priority for Keyboard Interrupt
    mparam.sched_priority = 98;
    pthread_attr_setschedparam(&attr, &mparam);
    pthread_create(&keyboard_event_id, &attr, keyboard_event_func, NULL);

    sleep(TOTAL_TIME/1000);
    flag = 0;

    pthread_cancel(keyboard_event_id);

    for(int i=0; i<NUM_THREADS; i++)
    {
        if(threads[i].task_type == 1)
        pthread_cond_broadcast(&cond_var[threads[i].event_key]); 
    }
	
    for(int i=0;i<NUM_THREADS;i++)
    {
        pthread_join(thread_id[i], NULL);
    }
    pthread_join(keyboard_event_id, NULL);

    //DESTROY ALL MUTEX, COND, THREAD ATTRIBUTES, BARRIER OBJECT
    for(int i=0; i<NUM_MUTEXES; i++)
    {
        pthread_mutex_destroy(&mutex[i]);
    }
	
	pthread_attr_destroy(&attr);
	
    for(int i=0; i<10; i++)
    {
        pthread_cond_destroy(&cond_var[i]); 
		pthread_mutex_destroy(&cond_mutex[i]);
    }
	pthread_barrier_destroy(&barrier);
    return 0;
}