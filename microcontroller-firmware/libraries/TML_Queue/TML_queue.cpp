#include "TML_queue.h"

void initQueue(Queue *q)
{
  q->front = -1;
  q->rear = -1;
}

bool queueIsEmpty(Queue *q)
{
    return (q->front == -1 && q->rear == -1);
}

bool queueIsFull(Queue *q)
{
    return ((q->rear + 1) % BUFFER_SIZE == q->front);
}

void enqueue(Queue *q, char item) 
{
    if (queueIsFull(q)) {
        //Serial.println("Queue is full. Cannot enqueue.");
        return; // TODO: what to do when full? How to indicate?
    }
    if (queueIsEmpty(q)) {
        q->front = 0;
        q->rear = 0;
    } else {
        q->rear = (q->rear + 1) % BUFFER_SIZE;
    }
    q->commands[q->rear] = item;
}

// TODO: Make sure main loop runs really fast!!!
char dequeue(Queue *q) 
{
    char dequeuedItem;
    if (queueIsEmpty(q)) {
        //Serial.println("Queue is empty. Cannot dequeue.");
        return '\0'; // Null character
    }
    dequeuedItem = q->commands[q->front];
    if (q->front == q->rear) {
        q->front = -1;
        q->rear = -1;
    } else {
        q->front = (q->front + 1) % BUFFER_SIZE;
    }
    return dequeuedItem;
}