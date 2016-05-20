// Created by Mat Sharff

#include "operating_system.h"

#define RUN_TIME 200
#define CREATE_ITERATIONS 5
#define QUANTUM_DURATION 300

//The following are initialized in main
unsigned long pid_counter; // Needs to be mutex locked if using multiple cpus on different threads
unsigned long sys_stack;
int dispatch_counter;
int create_count;
int io_1_downcounter;
int io_2_downcounter;

PCB_p idl;
FIFOq_p created_queue;
FIFOq_p ready_queue; // Could use Priority queue since all PCB priorities will be same value
FIFOq_p io_1_waiting_queue;
FIFOq_p io_2_waiting_queue;



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

int os_timer(int * the_timer_count) {
  if (* the_timer_count == QUANTUM_DURATION) {
    *the_timer_count = 0; // reset timer
    return 1; // timer interrupt happened
  }
  return 0;
}

int check_for_io1_done_interrupt(void) {
  if (io_1_downcounter == 0) {
    io_1_downcounter--;
    return 1;
  }
  io_1_downcounter--;
  return 0;
}

int check_io(PCB_p the_pcb) {
  int i;
  for (i = 0; i < 4; i++) {
    if (the_pcb->io_1_[i] == sys_stack) {
      return 1; // request for device number 1
    }
    if (the_pcb->io_2_[i]) {
      return 2; // request for device number 2
    }
  }
  return 0;
}

int io_service_request_trap_handler(int the_io_device_number) {
  if (the_io_device_number == 1) {
    if (FIFOq_is_empty(io_1_waiting_queue)) {
      io_1_downcounter = QUANTUM_DURATION * (rand() % (5 + 1 - 3) + 3); // multiply quantum 3-5 times
    }

  }
}


void cpu(void) {

  unsigned long pc_register = 0;
  int i, run_count;
  int random_pc_increment;
  int error_check;
  int rand_num_of_processes;
  int timer_count;
  int is_timer_interrupt;
  int is_io_interrupt;

  // main cpu loop
  for (run_count = 0; run_count < RUN_TIME; run_count++) {
    error_check = 0; // TODO: Test this statement
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
    // Simulate running of current process (Execute Instruction)
    pc_register++; // increment by one this time, represents a single instruction
    // random_pc_increment = rand() % (4000 + 1 - 3000) + 3000;
    // pc_register += random_pc_increment;

    // Check for timer interrupt
    is_timer_interrupt = os_timer(&timer_count);

    // Pseudo push of PC to system stack
    sys_stack = pc_register;
    if (is_timer_interrupt) {
      error_check = pseudo_isr(timer, &pc_register);
    }

    // if one of the I/O arrays has a number with the current pc value in it
    // trigger an IO_service_request_trap_handler which starts an I/O device downcounting
    // and adds the current process into a waiting queue
    is_io_interrupt = check_io(current_process);
    if (is_io_interrupt == io_1_interrupt) {
      // error_check = pseudo_isr(io_1_interrupt, &pc_register);
    } else if (is_io_interrupt == io_2_interrupt) {
      // error_check = pseudo_isr(io_1_interrupt, &pc_register);
    }


    // call pseudo_isr with timer interrupt type
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
  io_1_downcounter = 0;
  io_2_downcounter = 0;

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
  io_1_waiting_queue = FIFOq_construct();
  FIFOq_init(io_1_waiting_queue);
  io_2_waiting_queue = FIFOq_construct();
  FIFOq_init(io_2_waiting_queue);

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
