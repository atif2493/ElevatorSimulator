/*###############
# Author: Atif Jaffery
# Course: IUPUI CSCI 503 Operating Systems - Fall Semester
# Lab: 5 (a) - Simple elevator simulation application
# Description: this applicaiton consists of three files main.c, do_thread.h and do_thread.c
# The application takes 6 parameters and create one person and multiple threads.
#Person thread creates the person, and elevator threads moves those person
###############*/

#include <stdio.h>
#include <pthread.h>

typedef struct lv_t
{
	int id;
}* LV;

typedef struct gv_t 
{ 
	/* All the global information related to the elevator simulation */
	int num_elevators;
	int num_floors;
	int arrival_time;
	int elevator_speed;
	int simulation_time;
	int random_seed;
	int num_people_started; /* statistics */
	int num_people_finished; /* statistics */
	pthread_mutex_t lock;
	pthread_cond_t cv; 	/* Used to block the elevator if there is no request */
}* GV;

enum Direction
{
	up,
	down
};

typedef struct person 
{
	int id;
	int from_floor; 
	int to_floor; /* i.e., from where to where */
	double arrival_time; /* The time at which the person arrives */

	/* You can add more fields */
	enum Direction direction;
	struct person* next;
	struct person* prev;
}* P;

typedef struct elevator 
{
	int id; /* 0, 1, 2, 3, … */
	int current_floor; /* Current location of the elevator */
	pthread_mutex_t lock;
	pthread_cond_t cv; /* Used to block the elevator if there is no request */
	P people;/* All the people inside the elevator */

	/* You can add more fields */
	enum Direction direction;
}* E;

void* do_person_thread(void* v);

void* do_elevator_thread(void* v);

