#ifndef TML_QUEUE_H
#define TML_QUEUE_H

// #include <HardwareSerial.h>
#include <stdio.h>
// #include <math.h>
// #include <stdbool.h>

#define BUFFER_SIZE 100

// Structure for Queue
typedef struct {
  char commands[BUFFER_SIZE];
  int front;
  int rear;
} Queue;

/*
* FUNCTION: initQueue(*q)
* 
* initializes the variables inside a Queue object
*
* q = Address of queue object
*/
void initQueue(Queue *q);

/*
* FUNCTION: queueIsEmpty(*q)
* 
* checks if the queue is empty
*
* q = Address of Queue object
* returns: True if queue is empty, false otherwise
*/
bool queueIsEmpty(Queue *q);

/*
* FUNCTION: queueIsFull(*q)
* 
* checks if the queue is full
*
* q = Address of Queue object
* returns: True if queue is full, false otherwise
*/
bool queueIsFull(Queue *q);

/*
* FUNCTION: enqueue(*q, item)
* 
* enqueues a char
*
* q = Address of Queue object
* item = character to enqueue
*/
void enqueue(Queue *q, char item);

/*
* FUNCTION: dequeue(*q)
* 
* dequeues a char. Returns null character if empty (non-blocking)
*
* q = Address of Queue object
* returns: character dequeued, null if there is nothing to dequeue
*/
char dequeue(Queue *q);

#endif