#ifndef __CF_QUEUE_H__
#define __CF_QUEUE_H__

#include "stdint.h"
#include "stdio.h"

// Nicknamed cf for core framework -- my name for chad original systems that
// (eventually) will be reused in multiple projects

// Initialize at -1

typedef struct QueueNode {
  void *data;
  struct QueueNode *next;
} CF_QueueNode;

typedef struct {
  CF_QueueNode *front;
  CF_QueueNode *rear;
  uint32_t size;
} CF_Queue;

typedef void (*CF_FreeFunc)(
    void *data); // Supply your own free function for the data

CF_Queue *cf_queue_init();

void cf_queue_free(CF_Queue *q, CF_FreeFunc free_func);

void cf_queue_clear(
    CF_Queue *q); // In the case that you do not care for freeing

void cf_enqueue(CF_Queue *q, void *data);

void *cf_dequeue(CF_Queue *q);

#endif
