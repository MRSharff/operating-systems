/**
* By Mat Sharff
*/

#ifndef PCB_H_
#define PCB_H_

#define DEFAULT_PID 0
#define DEFAULT_STATE 0
#define DEFAULT_PRIORITY 0
#define DEFAULT_PC 0

// /** Exit Codes */
// #define OK 0
// #define NULL_OBJECT 1
// #define FAIL 2


#include <stdio.h>
#include <stdlib.h>
#include "errors.h"

typedef enum state_type {
  new, ready, running, interrupted, waiting, halted
} pcb_state;

typedef struct pcb {
  unsigned long pid; //process ID#, a unique number
  pcb_state state; // process state
  unsigned short priority; // priorities 0=highest, 15=lowest
  unsigned long pc; //holds the current pc value when preempted
} PCB;
typedef PCB * PCB_p;

/** Returns pcb pointer to heap allocation */
PCB_p PCB_construct(void);

/** Deallocates pcb from the heap */
void PCB_destruct(PCB_p);

/** Sets the default values for member data */
int PCB_init(PCB_p);

/** Sets the PID of the pcb */
int PCB_set_pid(PCB_p, unsigned long);

/** Returns the PID of the pcb */
unsigned long PCB_get_pid(PCB_p);

/** Returns the state of the pcb */
pcb_state PCB_get_state(PCB_p);

/** Sets the state of the pcb */
int PCB_set_state(PCB_p, pcb_state);

/** Returns a string representation of the PCB */
char * PCB_to_string(PCB_p);

/** PCB Test */
int PCB_test(void);

#endif
