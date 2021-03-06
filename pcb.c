/**
* By Mat Sharff
*/

#include "pcb.h"

/** Used for printing out state name based on state enum held in pcb */
const char * state_names[] = {
  "New", "Ready", "Running", "Interrupted", "Waiting", "Halted"
};

/** The string format of a PCB, also the longest string it can be. ~82 */
const char * string_format = "PID: 0x7FFFFFFFFFFFFFFF, State: Interrupted, Priority: 0xF, PC: 0x7FFFFFFFFFFFFFFF";

PCB_p PCB_construct(void) {
  return malloc(sizeof(PCB));
}

void PCB_destruct(PCB_p the_pcb) {
  free(the_pcb);
}

int PCB_init(PCB_p the_pcb) {
  if (the_pcb != NULL) {
    the_pcb->pid = DEFAULT_PID;
    the_pcb->state = DEFAULT_STATE;
    the_pcb->priority = DEFAULT_PRIORITY;
    the_pcb->pc = DEFAULT_PC;
    return NO_ERRORS;
  }
  return NULL_POINTER;
}

int PCB_set_pid(PCB_p the_pcb, unsigned long the_pid) {
  if (the_pcb != NULL) {
    the_pcb->pid = the_pid;
    return NO_ERRORS;
  }
  return NULL_POINTER;
}

unsigned long PCB_get_pid(PCB_p the_pcb) {
  if (the_pcb != NULL) {
    return the_pcb->pid;
  }
}

char * PCB_to_string(PCB_p the_pcb) {
  if (the_pcb != NULL) {
    static char return_string[82 + 1];
    sprintf(return_string, "PID: 0x%lX, State: %s, Priority: 0x%hX, PC: 0x%lX",
            the_pcb->pid, state_names[the_pcb->state], the_pcb->priority, the_pcb->pc);
    return return_string;
  }
  return "Null";
}

int PCB_test(void) {
  PCB_p test_pcb = malloc(sizeof(PCB));
  test_pcb->pid = 18;
  test_pcb->state = running;
  test_pcb->priority = 1;
  test_pcb->pc = 0;

  char * pcb_string = "PID: , State: , Priority: , PC: ";

  printf("Size of PCB is: %lu\n", sizeof(PCB));
  printf("Size of pcb_string is: %lu\n", sizeof(pcb_string));
  printf("Size of test_pcb is: %lu\n", sizeof(*test_pcb));
  printf("%s\n", PCB_to_string(test_pcb));

  // printf("size of pcb string: %lu\n", sizeof(PCB_to_string(test_pcb)));
  free(test_pcb);
}
