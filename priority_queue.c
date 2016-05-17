#include <time.h>
#include "priority_queue.h"

//Q1:Count=3: P8->P21->P2->P8-*

const char * pq_string_header = ":Count=";
const char * pq_colon_space = ": ";
const char * pq_spacer = " : contents: ";
const char * pq_queue_separator = "->";
const char * pq_queue_end = "-*";
const char * pq_pcb_format = "P";

const char * empty_priority_queue_string = "QXX:Count=0: -* : contents: Null";

PRIORITYq_p PRIORITYq_construct(void) {
  int i;
  PRIORITYq_p new_priority_queue = malloc(sizeof(PRIORITYq));
  for (i = 0; i < PRIORITY_RANGE; i++) {
    new_priority_queue->fifo_queues[i] = FIFOq_construct();
    FIFOq_init(new_priority_queue->fifo_queues[i]);
  }
  new_priority_queue->size = 0;
  return new_priority_queue;
}

// void PRIORITYq_init(PRIORITYq_p priority_queue) {
//
// }

void PRIORITYq_destruct(PRIORITYq_p priority_queue) {
  int i;
  for (i = 0; i < PRIORITY_RANGE; i++) {
    FIFOq_destruct(priority_queue->fifo_queues[i]);
  }
  free(priority_queue);
}

int PRIORITYq_enqueue(PRIORITYq_p priority_queue, PCB_p pcb) {
  if (priority_queue != NULL && pcb != NULL) {
    FIFOq_enqueue(priority_queue->fifo_queues[pcb->priority], pcb);
    return NO_ERRORS;
  }
  print_error(NULL_POINTER);
  return NULL_POINTER;
}

PCB_p PRIORITYq_dequeue(PRIORITYq_p priority_queue) {
  int priority;
  for (priority = 0; priority < PRIORITY_RANGE; priority++) {
    if (FIFOq_is_empty(priority_queue->fifo_queues[priority]) == 0) {
      return FIFOq_dequeue(priority_queue->fifo_queues[priority]);
    }
  }
  return NULL;
}

char * PRIORITYq_single_to_string(FIFOq_p queue, int priority) {
  char * return_string;
  char * last_pcb_string;

  // if the queue is empty, print a predefined empty queue string
  if (queue->size == 0) {
    return_string = calloc(1, strlen(empty_priority_queue_string) * sizeof(char) + 10);
    strcat(return_string, empty_priority_queue_string);
    return return_string;
  }

  if (queue != NULL) {
    // initialize variables
    int digit_count = 1;
    int queue_size;
    int string_length;
    char buffer[digit_count+2];

    // get the largest amount of digits in the queue, we'll need it for PX allocation
    // queue_size = queue->size;
    // while (queue_size != 0) {
    //   queue_size = queue_size / 10;
    //   digit_count++;
    // }
    digit_count = 4;
    queue_size = queue->size;

    last_pcb_string = PCB_to_string(queue->tail->pcb);

    // printf("Last PCB String: %s\n", last_pcb_string);

    // Get the total length that the string will be that we need to allocate
    // string_length = strlen(pq_string_header)
    //     + digit_count
    //     +

    string_length = strlen(pq_string_header) + 2
        + digit_count
        + strlen(pq_colon_space)
        + (strlen(pq_pcb_format) + digit_count) * (queue_size) // allocates for each PX
        + strlen(pq_queue_separator) * queue_size //allocate for each ->
        + strlen(pq_queue_end) * (queue_size - 1) //allocate for -*
        + strlen(pq_spacer)
        + strlen(last_pcb_string);

    // This must be freed after to_string is called
    // Set a variable to the return value of this function,
    //print the variable, then free the variable.
    return_string = malloc(string_length + 10 * sizeof(char));

    //add "QX:Count=%d: "
    sprintf(buffer, "Q%d", priority);
    sprintf(return_string, "%s%s%d%s", buffer, pq_string_header, queue_size, pq_colon_space);

    // This part adds all the P1->P2->P3 etc to the string
    node_p current = queue->head;

    while (current != NULL) {
      sprintf(buffer, "P%lu", current->pcb->pid);
      strcat(return_string, buffer);
      if (current->next != NULL) { //print the queue seperator only if it's not the last one
        strcat(return_string, pq_queue_separator);
      }
      current = current->next;
    }

    //print the queue_end string (default is -*)
    strcat(return_string, pq_queue_end);
    strcat(return_string, pq_spacer); //print spacer
    strcat(return_string, last_pcb_string); // print last pcb in the queue
    return return_string;
  }
  return "Queue is NULL";
}

char * PRIORITYq_to_string(PRIORITYq_p priority_queue) {
  char * queue_string;
  char * p_queue_string;
  int string_size = 0;
  int priority;
  for (priority = 0; priority < PRIORITY_RANGE; priority++) {
    if (FIFOq_is_empty(priority_queue->fifo_queues[priority]) == 0) {
      string_size+= (strlen(PRIORITYq_single_to_string(priority_queue->fifo_queues[priority], priority)) + 1);
    }
  }
  p_queue_string = calloc(1, string_size * sizeof(char) + 10);
  strcat(p_queue_string, "");
  for (priority = 0; priority < PRIORITY_RANGE; priority++) {
    if (FIFOq_is_empty(priority_queue->fifo_queues[priority]) == 0) {
      strcat(p_queue_string, PRIORITYq_single_to_string(priority_queue->fifo_queues[priority], priority));
      strcat(p_queue_string, "\n");
    }
  }
  // printf("%s\n", p_queue_string);
  return p_queue_string;
}

int priority_test(void) {
  srand(time(NULL));
  int r, i;
  PRIORITYq_p pqueue;

  pqueue = PRIORITYq_construct();

  for (i = 0; i < 50; i++) {
      r = rand() % 16;
      PCB_p temp = PCB_construct();
      PCB_init(temp);
      temp->priority = r;
      temp->pid = i;
      PRIORITYq_enqueue(pqueue, temp);
  }
  // for (i = 0; i < 16; i++) {
  //   printf("%s\n", FIFOq_to_string(pqueue->fifo_queues[i]));
  // }
  printf("%s\n", PRIORITYq_to_string(pqueue));
  return 0;
}
