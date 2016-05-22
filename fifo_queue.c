/**
* By Mat Sharff
*/

#include "fifo_queue.h"

const char * fifo_queue_string_header = "Q:Count=";
const char * colon_space = ": ";
const char * spacer = " : contents: ";
const char * queue_separator = "->";
const char * queue_end = "-*";
const char * pcb_format = "P";

const char * empty_fifo_queue_string = "Q:Count=0: -* : contents: Empty";

FIFOq_p FIFOq_construct() {
  FIFOq_p queue = malloc(sizeof(FIFOq));

  if (queue != NULL) {
    queue->head = NULL;
    queue->tail = NULL;
  }

  return queue;
}

int FIFOq_init(FIFOq_p queue) {
  if (queue != NULL) {
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;
    return NO_ERRORS;
  }
  print_error(NULL_POINTER);
  printf("queue was null in FIFOq_init");
  return NULL_POINTER;
}

int FIFOq_enqueue(FIFOq_p queue, PCB_p pcb) {
  if (queue != NULL && pcb != NULL) {
    node_p new_node = malloc(sizeof(node));
    new_node->pcb = pcb;
    new_node->next = NULL;

    if (FIFOq_is_empty(queue)) {
      queue->head = new_node;
      queue->tail = new_node;
    } else {
      queue->tail->next = new_node;
      queue->tail = new_node;
    }

    queue->size++;
    return NO_ERRORS;
  }
  print_error(NULL_POINTER);
  printf("queue or pcb was null in FIFOq_enqueue");
  return NULL_POINTER;
}

PCB_p FIFOq_dequeue(FIFOq_p queue) {
  queue->size--; // decrement size
  if (queue != NULL) {
    node_p old_head = queue->head; //set old head to current head
    if (queue->size != 0) { // queue is not empty, no new head
      queue->head = old_head->next; // set head to the next of the old head
    }

    PCB_p result = old_head->pcb;
    free(old_head); // Free the old head node
    return result;
  }
  print_error(NULL_POINTER);
  printf("queue was null in FIFOq_dequeue");
  return NULL;
}

PCB_p FIFOq_peek(FIFOq_p queue) {
  if (queue != NULL) {
    return queue->head->pcb;
  }
  print_error(NULL_POINTER);
  printf("queue was null in FIFOq_peek");
  return NULL;
}

int FIFOq_destruct(FIFOq_p queue) {
  if (queue != NULL) {
    while (queue->size > 0) { //free the remainding pcbs
      PCB_destruct(FIFOq_dequeue(queue)); // dequeue frees the node
    }
    free(queue); //free the queue
    return NO_ERRORS;
  }
  print_error(NULL_POINTER);
  printf("queue was null in FIFOq_destruct");
  return NULL_POINTER;
}

int FIFOq_is_empty(FIFOq_p queue) {
  if (queue != NULL) {
    return (queue->size == 0);
  }
  print_error(NULL_POINTER);
  printf("queue was null in FIFOq_is_empty");
  return -1;
}

int FIFOq_size(FIFOq_p queue) {
  if (queue != NULL) {
    return queue->size;
  }
  print_error(NULL_POINTER);
  printf("queue was null in FIFOq_size");
  return -1;
}

char * FIFOq_to_string(FIFOq_p queue) {
  // This function requires that the implementation holds a variable to the pointer
  // that this function returns. Then prints it. Then frees using the variable.

  // The logic for string allocation size is not exact. There are overestimates in
  // some places which may cause high memory usage. For what we are using this for,
  // this logic works fine. However, it has the ability to be optimized.

  //initialize variables
  char * return_string;
  char * last_pcb_string;
  int digit_count = 4;
  int queue_size;
  int string_length;
  char buffer[digit_count+2];

  // if the queue is empty, print a predefined empty queue string
  if (queue->size == 0) {
    return_string = calloc(1, strlen(empty_fifo_queue_string) * sizeof(char) + 10);
    strcat(return_string, empty_fifo_queue_string);
    return return_string;
  }

  if (queue != NULL) {
    // get the largest amount of digits in the queue, we'll need it for PX allocation
    // Set Digit count to 4 to allow for PXXXX process number allocation.
    // This could be a cause of a segfault if the PIDs ever reach very high numbers
    // if so, just set digit_count to higher number. Although, this will take up more memory.
    queue_size = queue->size;

    last_pcb_string = PCB_to_string(queue->tail->pcb);

    // printf("Last PCB String: %s\n", last_pcb_string);

    // Get the total length that the string will be that we need to allocate
    string_length = strlen(fifo_queue_string_header)
        + digit_count
        + strlen(colon_space)
        + (strlen(pcb_format) + digit_count) * (queue_size) // allocates for each PX
        + strlen(queue_separator) * queue_size //allocate for each ->
        + strlen(queue_end) * (queue_size - 1) //allocate for -*
        + strlen(spacer)
        + strlen(last_pcb_string);

    // This must be freed after to_string is called
    // Set a variable to the return value of this function,
    //print the variable, then free the variable.
    return_string = malloc(string_length + 10 * sizeof(char));

    //add "Q:Count=%d: "
    sprintf(return_string, "%s%d%s", fifo_queue_string_header, queue_size, colon_space);

    // This part adds all the P1->P2->P3 etc to the string
    node_p current = queue->head;
    while (current != NULL) {
      sprintf(buffer, "P%lu", current->pcb->pid);
      strcat(return_string, buffer);
      //print the queue seperator only if it's not the last item
      if (current->next != NULL) {
        strcat(return_string, queue_separator);
      }
      current = current->next;
    }

    strcat(return_string, queue_end);//print queue_end string (default is -*)
    strcat(return_string, spacer); //print spacer
    strcat(return_string, last_pcb_string); // print last pcb in the queue
    return return_string;
  }
  return "Queue is NULL in to_string";
}

int FIFOq_test(int test_size) {
  int i;
  FIFOq_p queue = FIFOq_construct();
  FIFOq_init(queue);
  for (i = 0; i < test_size; i++) {
    PCB_p temp = PCB_construct();
    PCB_init(temp);
    PCB_set_pid(temp, i);
    printf("%s\n", PCB_to_string(temp));
    FIFOq_enqueue(queue, temp);
    char * queue_string = FIFOq_to_string(queue);
    printf("%s\n", queue_string);
    free(queue_string);
  }
  while (queue->size > 0) {
    PCB_p temp = FIFOq_dequeue(queue);
    char * queue_string = FIFOq_to_string(queue);
    printf("%s\n", queue_string);
    free(queue_string);
    PCB_destruct(temp);
  }
  FIFOq_destruct(queue);
}
