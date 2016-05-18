/**
* By Mat Sharff
*/

#ifndef FIFO_QUEUE_H_
#define FIFO_QUEUE_H_

#include <string.h>
#include "pcb.h"

typedef struct node_struct node;
typedef node * node_p;

struct node_struct {
  PCB_p pcb;
  node_p next;
};

typedef struct FIFO_q {
  int size;
  node_p head;
  node_p tail;
} FIFOq;
typedef FIFOq * FIFOq_p;

/** Returns fifoq pointer to heap allocation. */
FIFOq_p FIFOq_construct(void);

/** Sets the default values for fifoq member data. */
int FIFOq_init(FIFOq_p);

/** Enqueues a PCB in the queue. */
int FIFOq_enqueue(FIFOq_p, PCB_p);

/** Returns the PCB at the head of the queue and dequeues it. */
PCB_p FIFOq_dequeue(FIFOq_p);

/** Returns the PCB at the head of the queue but does not dequeue it. */
PCB_p FIFOq_peek(FIFOq_p);

/** Deallocates fifoq from the heap. */
int FIFOq_destruct(FIFOq_p);

/** Returns 1 if the fifoq is empty, 0 otherwise. */
int FIFOq_is_empty(FIFOq_p);

/** Returns the size of the fifoq. */
int FIFOq_size(FIFOq_p);

/** Returns a string representation of the fifoq. */
char * FIFOq_to_string(FIFOq_p);

/** Test method */
int FIFOq_test(int);

#endif
