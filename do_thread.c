
/*###############
# Author: Atif Jaffery
# Course: IUPUI CSCI 503 Operating Systems - Fall Semester
# Lab: 5 (a) - Simple elevator simulation application
# Description: this applicaiton consists of three files main.c, do_thread.h and do_thread.c
# The application takes 6 parameters and create one person and multiple threads.
#Person thread creates the person, and elevator threads moves those person
#This class represents the thread class for both person and elevator threads
###############*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

#include "do_thread.h"

// global data
pthread_mutex_t lock; // lock for synchronization between elevator and person threads
P ll;	// linked list
GV gv;
time_t start; // note the time when program starts
int all_threads_done; // varibale to tell elevator threads to finish

// initialize global data, got from the user and other data
void init_globalData(GV global)
{
  // initialize globa data
  gv = global;

  // start time
  time(&start);

  // set it to 1
  all_threads_done = 1;

  // linked list

  ll = (P)malloc(sizeof(*ll));
  pthread_mutex_init(&lock, NULL);
  ll->next = NULL;
  ll->prev = NULL;
}

// get the current time based on the requirement
double getTime()
{
	time_t end;
	time(&end);

	// return the difference with start time
	double diff = difftime(end,start);
	return diff/10000;
}

// increamen the people started
void increment_num_people_started()
{
	// only person thread is making this change, so no need to synchrnoize this data
	gv->num_people_started++;
}

// this method add person to the linked list
void addPersonToLinkedList(P p)
{
	// only person thread is adding to linked list, so no need to synchrnoize this data

	if(ll->next == NULL)
	{
		// if first element in the linked list
		ll->next = p;
		p->prev = ll;
	}
	else
	{
		// if linkled list has more elements
		P next = ll->next;
		while(next->next != NULL)
		{
			next = next->next;	
		}

	   next->next = p;
   	p->prev = next;
	}

	// let the waiting elevator know that aperson has been added
   pthread_cond_broadcast(&(gv->cv));
}

// person thread, add perons to the linked list
void* do_person_thread(void *v) 
{
  // get the parameter and initialize the global data
  GV global = (GV)v;
  init_globalData(global);

        printf("num_elevator    :%d\n",gv->num_elevators);
	printf("num_floors      :%d\n",gv->num_floors);
	printf("arrival_time    :%d\n",gv->arrival_time);
	printf("elevator_speed  :%d\n",gv->elevator_speed);
	printf("simulation_time :%d\n",gv->simulation_time);
	printf("random_seed     :%d\n",gv->random_seed);

	
	// person id
  int id = 1;

	// person
  P p;

	// variable to run the program until simulation time
  int seconds = 0;

	// sleep time for creating a person
  int st = gv->arrival_time;
	
	// random seed
  srand(gv->random_seed);

	// person arrival time
  time_t arrivalTime;

  // while time is less than simulation time
  while(seconds < gv->simulation_time) 
  {  
		// sleep
		sleep(st);

		// add time
	   seconds += gv->arrival_time;

		// randomly generate to and from floors
		int from = (rand()%gv->num_floors) + 1;
		int to = (rand()%gv->num_floors) + 1;
		if(from == to) 
		{
			// if floors are same, continue
			continue;
		}
		 
		 // initialize a person
		 time(&arrivalTime);
		 p = (P)malloc(sizeof(*p));
		 p->id = id;
		 p->from_floor = from; 
	 	 p->to_floor = to;
		 p->arrival_time = arrivalTime;
		 p->next = NULL;

	 	 if(from > to)
	 		p->direction = down;
	    else
			p->direction = up;
	 
		 // add person to lniked list
		 addPersonToLinkedList(p);

		 // increment the number
	 	 increment_num_people_started();

		 printf("[%0.4lf] Person %d arrives on floor %d waiting to go to floor %d\n", getTime(), p->id, p->from_floor, p->to_floor);

		// increment the id
   	id++;
   }

	// inform elevator threads to finish
   all_threads_done = 0;
	return NULL;
}

// this mehtod will move the elevator up or down
void moveUporDown(E e)
{
	sleep(gv->elevator_speed);
	if(e->direction == up)
	{
  	   if(e->current_floor != gv->num_floors)
			e->current_floor++;
	}
	else
	{
		if(e->current_floor != 0)
			e->current_floor--;
	}
}

// this method gets the next person from linked list
P getPersonFromLinkedList(E e)
{
	pthread_mutex_lock(&lock);

	if (ll->next == NULL) //empty
	{
		pthread_cond_wait(&(gv->cv), &lock);
	}

	P result = NULL;
	if(ll->next != NULL)
	{
		P p = ll->next;
		if(p->next != NULL)
		{
			ll->next = p->next;
			ll->next->prev = ll;
		}
		else
		{
			ll->next = NULL;
		}
		result = p;
	}
	
	pthread_mutex_unlock(&lock);

	return result;
}

// This method picks the person to move it
void pickPerson(E e, P p)
{
	sleep(gv->elevator_speed);

	// only one person for this version
	e->people->next = p;

	printf("[%0.4lf] Elevator %d picks up person %d\n", getTime(), e->id, p->id);
}

// this method increment the person who are finished
void increment_num_people_finished(E e)
{
	pthread_mutex_lock(&(gv->lock));
	gv->num_people_finished++;
	pthread_mutex_unlock(&(gv->lock));
}

// This method drops the person
void dropPerson(E e, P p)
{
	// Only one person for this version, so just dropping the first one
	e->people->next = NULL;

	increment_num_people_finished(e);

 	printf("[%0.4lf] Elevator %d drops person %d\n", getTime(), e->id, p->id);

	free(p);
}

// elevator thread, might be many instances of this function are running
void* do_elevator_thread(void *v) 
{
	// elevator id
  int* id = (int*)v;
  
  // person
  P p;
  
  // initialize elevator info
  E e;
  e = (E)malloc(sizeof(*e));
  e->id = (int)id;
  e->current_floor = 1;
  e->people = (P)malloc(sizeof(*p));
  e->direction = up;
 
	// elevator speed
  int st = gv->elevator_speed;

  // loop until time is up
  while(all_threads_done != 0) 
  {
	 // get the perosn from linked list
  	 P p = getPersonFromLinkedList(e);	  

	 // continue if the person in undefined
	 if(p == NULL || p->id == 0 || p->from_floor == 0 || p->from_floor > gv->num_floors) continue;
	 
	 printf("[%0.4lf] Elevator %d starts moving from %d to %d\n", getTime(), e->id, e->current_floor, p->from_floor);

	 // set the direction of elevator
	 if(e->current_floor > p->from_floor)
	 {
		e->direction = down;
	 }
	 else
	 {
		e->direction = up;
	 }

	 // move the elevator
	 while(e->current_floor != p->from_floor)
    {
		moveUporDown(e);
	 }

	 printf("[%0.4lf] Elevator %d arrives at floor %d\n", getTime(), e->id, e->current_floor);

	 pickPerson(e, p);

	 // time to pick the person, used the elevator speed
	 sleep(st);

	 printf("[%0.4lf] Elevator %d starts moving from %d to %d\n", getTime(), e->id, e->current_floor, p->to_floor);

	 e->direction = p->direction;

	 // move the elevator to the person 's requested floor	
	 while(e->current_floor != p->to_floor)
    {
		moveUporDown(e);
	 }

	 printf("[%0.4lf] Elevator %d arrives at floor %d\n", getTime(), e->id, e->current_floor);

	 // drop the person
	 dropPerson(e, p);
  }

	return NULL;
}
