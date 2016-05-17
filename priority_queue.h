#ifndef PRIORITY_QUEUE_H_
#define PRIORITY_QUEUE_H_

#include "fifo_queue.h"

#define PRIORITY_RANGE 16

typedef struct priority_queue {
  FIFOq_p fifo_queues[PRIORITY_RANGE];
  int size;
} PRIORITYq;

typedef PRIORITYq * PRIORITYq_p;

/** Returns priorityq pointer to heap allocation. */
PRIORITYq_p PRIORITYq_construct(void);

void PRIORITYq_destruct(PRIORITYq_p);

int PRIORITYq_enqueue(PRIORITYq_p, PCB_p);

PCB_p PRIORITYq_dequeue(PRIORITYq_p);

char * PRIORITYq_to_string(PRIORITYq_p);

int priority_test(void);

#endif
