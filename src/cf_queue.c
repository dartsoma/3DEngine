#include "cf_queue.h"
#include "stdlib.h"

CF_Queue *cf_queue_init() {

  CF_Queue *q = (CF_Queue *)malloc(sizeof(CF_Queue));

  q->rear = NULL;
  q->front = NULL;

  return q;
}

void cf_queue_free(CF_Queue *q, CF_FreeFunc free_func) {
  int i;
  for (i = 0; i < q->size; i++) {

    void *data = cf_dequeue(q);

    if (free_func && data) {
      free_func(data);
    }
  }
}

void cf_clear(CF_Queue *q) {
  int i;
  for (i = 0; i < q->size; i++) {
    cf_dequeue(q);
  }
}

void cf_enqueue(CF_Queue *q, void *data) {
  CF_QueueNode *node = (CF_QueueNode *)malloc(sizeof(CF_QueueNode));
  node->data = data;
  node->next = NULL;
  if (q->rear == NULL) {
    q->front = q->rear = node;
    return;
  }

  q->rear->next = node;
  q->rear = node;
}

void *cf_dequeue(CF_Queue *q) {
  if (q->front == NULL)
    return NULL;
  CF_QueueNode *temp = q->front;
  void *data = temp->data;
  q->front = q->front->next;
  if (q->front == NULL)
    q->rear = NULL;
  free(temp);
  return data;
}
