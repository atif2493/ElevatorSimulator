/*###############
# Author: Atif Jaffery
# Course: IUPUI CSCI 503 Operating Systems - Fall Semester
# Lab: 5 (a) - Simple elevator simulation application
# Description: this applicaiton consists of three files main.c, do_thread.h and do_thread.c
# The application takes 6 parameters and create one person and multiple threads.
#Person thread creates the person, and elevator threads moves those person
###############*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "do_thread.h"

// main method
int main(int argc, char *argv[]) 
{
	// local data
  	int i, num_elevators, num_floors, people_arrival_time_sec, elevator_speed_sec, time_simulate_sec, random_number_seed;

  	GV              gv;
  	pthread_t       *person_thrd;
  	pthread_t       *elevator_thrds;
  	pthread_attr_t  *person_attrs;
  	pthread_attr_t  *elevator_attrs;
  	void            *retval;

	// check parameters

  	if(argc != 7) 
  	{
   	 fprintf(stderr, "Usage: num_elevators num_floors people_arrival_time_sec elevator_speed_sec time_simulate_sec random_number_seed\n");
	    exit(1);
  	}

	num_elevators = atoi(argv[1]);
	if(num_elevators <= 0)
	{
	    printf("Enter valid num_elevators value, this is not a valid value %s\n", argv[1]);
	    exit(1);
	}

	num_floors = atoi(argv[2]);
	if(num_floors <= 0)
	{
	    printf("Enter valid num_floors value, this is not valid value %s\n", argv[2]);
	    exit(1);
	}

	people_arrival_time_sec = atoi(argv[3]);
	if(people_arrival_time_sec <= 0)
	{
	    printf("Enter valid people_arrival_time_sec value, this is not valid value %s\n", argv[3]);
	    exit(1);
	}

	elevator_speed_sec = atoi(argv[4]);
	if(elevator_speed_sec <= 0)
	{
	    printf("Enter valid elevator_speed_sec value, this is not valid value %s\n", argv[4]);
	    exit(1);
	}

	time_simulate_sec = atoi(argv[5]);
	if(time_simulate_sec <= 0)
	{
	    printf("Enter valid time_simulate_sec value, this is not valid value %s\n", argv[5]);
	    exit(1);
	}

	random_number_seed = atoi(argv[6]);
	if(random_number_seed <= 0)
	{
	    printf("Enter valid random_number_seed value, this is not valid value %s\n", argv[6]);
	    exit(1);
	}

  // initialize all the values
  gv = (GV) malloc(sizeof(*gv));

  gv->num_elevators = num_elevators;
  gv->num_floors = num_floors;
  gv->arrival_time = people_arrival_time_sec;
  gv->elevator_speed = elevator_speed_sec;
  gv->simulation_time = time_simulate_sec;
  gv->random_seed = random_number_seed;
  gv->num_people_started = 0;
  gv->num_people_finished = 0;

  // create person thread
  person_thrd = (pthread_t*) malloc(sizeof(pthread_t));
  person_attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t));
  if(pthread_attr_init(person_attrs)) perror("attr_init()");
  if(pthread_attr_setscope(person_attrs, PTHREAD_SCOPE_SYSTEM)) perror("attr_setscope()");
  if(pthread_create(person_thrd, person_attrs, do_person_thread, (void*)gv)) 
  {
     perror("pthread_create()");
     exit(1);
  }

  // create elevator threads
  elevator_thrds = (pthread_t*) malloc(sizeof(pthread_t)*num_elevators);
  elevator_attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*num_elevators);
  for(i = 0; i < num_elevators; i++) 
  {
    if(pthread_attr_init(elevator_attrs+i)) perror("attr_init()");
    if(pthread_attr_setscope(elevator_attrs+i, PTHREAD_SCOPE_SYSTEM)) perror("attr_setscope()");
    if(pthread_create(elevator_thrds+i, elevator_attrs+i, do_elevator_thread, (void*)(uintptr_t)i+1)) 
	{
      perror("pthread_create()");
      exit(1);
    }
  }

   pthread_join(person_thrd[0], &retval);

	printf("Simulation result: %d people have started, %d people have finished during %d seconds\n", gv->num_people_started, gv->num_people_finished, gv->simulation_time);

  // free memory
  free(gv);
  free(person_attrs);
  free(elevator_attrs);
  free(person_thrd);
  free(elevator_thrds);

  return 0; 
}
