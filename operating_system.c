// Created by Mat Sharff

#include "operating_system.h"

#define RUN_TIME 200
#define CREATE_ITERATIONS 5

//The following are initialized in main
unsigned long pid_counter; // Needs to be mutex locked if using multiple cpus on different threads
unsigned long sys_stack;
int dispatch_counter;
int create_count;
PCB_p idl;
FIFOq_p created_queue;
FIFOq_p ready_queue; // Could use Priority queue since all PCB priorities will be same value

//This pointer is changing constantly, initially set to point to idl in main
PCB_p current_process;

int dispatcher(void) {
  char * string; // free this after printing queue
  //save state of current process into its pcb (done in isr)

  if (ready_queue != NULL) {
    // Only dequeue next waiting process if ready queue has something to dequeue
    // otherwise, keep running the current process (usually idl)
    if (ready_queue->size > 0) {
      PCB_p temp = FIFOq_dequeue(ready_queue);
      if (dispatch_counter == 4) { // Only print stuff every 4th context switch (dispatch call)
        dispatch_counter = 0; // reset the counter;
        printf("Switching to: %s\n", PCB_to_string(temp));

        // PCB_set_state(temp, running); Only required when doing print stuff
        // because current_processgets set to temp and current_process state
        // get set to running outside of if (ready_queue->size > 0) statement
        PCB_set_state(temp, running);

        printf("Now running: %s\n", PCB_to_string(temp));
        printf("Returned to Ready Queue: %s\n", PCB_to_string(current_process));
        string = FIFOq_to_string(ready_queue);
        printf("Ready Queue: %s\n\n", string);
        free(string);
      }
      current_process = temp;
    }
    //current_process will be either the one we just dequeued or idl
    // either way, set it's state to running
    current_process->state = running; // change state to running
    //copy temps pc value to sys_stack to replace PC of interrupted process
    sys_stack = current_process->pc;

    dispatch_counter++; //increment the dispatch counter because it's been called
    return NO_ERRORS;
  }

  printf("ready_queue was null\n"); // this should never happen
  return NULL_POINTER;
}

int scheduler(enum interrupt_type int_type) {

  while (!FIFOq_is_empty(created_queue)) {
    PCB_p temp = FIFOq_dequeue(created_queue);
    PCB_set_state(temp, ready);
    // printf("Process PID: %lu moved to ready_queue from created_queue\n", PCB_get_pid(temp));
    printf("Process enqueued: %s\n", PCB_to_string(temp));
    FIFOq_enqueue(ready_queue, temp);
  }

  // Determine situation/what kind of interrupt happened;
  // I wonder if interrupt type could be held in a global queue to handle multiple interrupts coming in.
  int return_status;

  switch (int_type) {
    case io_1:
      // Replace next two lines when implemented
      return_status = INVALID_INPUT;
      printf("io_1 type passed, not implemented yet, should not happen\n");
      break;
    case io_2:
      // Replace next two lines when implemented
      return_status = INVALID_INPUT;
      printf("io_2 type passed, not implemented yet, should not happen\n");
      break;
    case timer: // what if current process is idl process?
      current_process->state = ready; //set to ready even if current_process is idl
      // printf("current_process pid = 0x%lX\n", current_process->pid);
      if (current_process->pid != idl->pid) { //only do this if the current process is not the idl one
        // just a timer interrupt, only need to set state back to ready
        // and place back into ready queue

        if (dispatch_counter != 4) {
          printf("Process enqueued: %s\n", PCB_to_string(current_process));
        }
        FIFOq_enqueue(ready_queue, current_process);
      } else {
        printf("current_process was idl process, do not add to ready_queue\n");
      }
      return_status = NO_ERRORS;
      break;
    default: // this should never happen
      return_status = INVALID_INPUT;
      printf("Invalid Interrupt type passed\n");
      break;
  }
  //call dispatcher
  dispatcher();
  // Additional Housekeeping (currently none)

  // return to pseudo_isr
  return return_status;
}


int pseudo_isr(enum interrupt_type int_type, unsigned long * cpu_pc_register) {
  if (current_process != NULL) {
    if (dispatch_counter == 4) {
      printf("\n\nCurrently Running PCB: %s\n", PCB_to_string(current_process));
    }
    // Change state of current process to interrupted
    current_process->state = interrupted;

    // Save cpu state to current processes pcb
    // pc was pushed to sys_stack right before isr happened.
    current_process->pc = sys_stack;

    // Up-call to scheduler
    scheduler(int_type);

    // IRET, put sys_stack into pc_register
    *cpu_pc_register = sys_stack;
    return NO_ERRORS;
  }
  printf("current_process was null\n"); //this should never happen
  return NULL_POINTER;
}


void cpu(void) {

  unsigned long pc_register = 0;
  int i, run_count;
  int random_pc_increment;
  int error_check;
  int rand_num_of_processes;

  // main cpu loop
  for (run_count = 0; run_count < RUN_TIME; run_count++) {
    rand_num_of_processes = rand() % 5;
    if (create_count < CREATE_ITERATIONS) { // Create processes
      printf("Creating %d processes\n", rand_num_of_processes);
      for (i = 0; i < rand_num_of_processes; i++) {
        PCB_p temp = PCB_construct();
        PCB_init(temp);
        PCB_set_pid(temp, pid_counter);
        pid_counter++;
        FIFOq_enqueue(created_queue, temp);
      }
    } // End create processes
    create_count++;
    // Simulate running of current process
    pc_register++; // increment by one this time, represents a single instruction
    // random_pc_increment = rand() % (4000 + 1 - 3000) + 3000;
    // pc_register += random_pc_increment;

    // Pseudo push of PC to system stack
    sys_stack = pc_register;
    error_check = pseudo_isr(timer, &pc_register);// call pseudo_isr with timer interrupt type
    switch(error_check) { // handle any errors that happened in pseudo_isr
      case NO_ERRORS: //working good, just break and continue;
        break;
      case NULL_POINTER:
        print_error(NULL_POINTER);
        break;
      case INVALID_INPUT:
        print_error(INVALID_INPUT);
        break;
      default:
        printf("Unknown error\n");
        break;
    }
  } // End main cpu loop
  // return error_check;
}

int main(void) {

  // Seed rand
  srand(time(NULL));

  // initialize variables
  pid_counter = 0;
  sys_stack = 0;
  create_count = 0;
  dispatch_counter = 0;

  // Create and initialize idle pcb
  idl = PCB_construct();
  PCB_init(idl);
  PCB_set_pid(idl, 0xFFFFFFFF);
  current_process = idl;

  // Create and initialize queues
  ready_queue = FIFOq_construct();
  FIFOq_init(ready_queue);
  created_queue = FIFOq_construct();
  FIFOq_init(created_queue);

  // Run a cpu
  cpu();

  // Cleanup / free stuff to not have memory leaks
  FIFOq_destruct(ready_queue);
  FIFOq_destruct(created_queue);
  PCB_destruct(idl);
  if (current_process != NULL) {
    PCB_destruct(current_process);
  }

  return 0;

}
