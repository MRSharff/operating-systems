// Created by Mat Sharff

#include "operating_system.h"

#define RUN_TIME 40000
#define CREATE_ITERATIONS 5
#define QUANTUM_DURATION 300

//The following are initialized in main
unsigned long pid_counter; // Needs to be mutex locked if using multiple cpus on different threads
unsigned long sys_stack;
int dispatch_counter;
int create_count;
int io_1_downcounter;
int io_2_downcounter;
int timer_count;
unsigned long pc_register;

int total_processes_created;

PCB_p idl;
FIFOq_p created_queue;
FIFOq_p ready_queue; // Could use Priority queue since all PCB priorities will be same value
FIFOq_p io_1_waiting_queue;
FIFOq_p io_2_waiting_queue;
FIFOq_p terminate_queue;



//This pointer is changing constantly, initially set to point to idl in main
PCB_p current_process;

PCB_p round_robin() {
  if (ready_queue != NULL) {
    if (!FIFOq_is_empty(ready_queue)) {
      return FIFOq_dequeue(ready_queue);
    }
    return idl; // no processes in ready queue, idl should be put onto ready queue
  }
  printf("in round_robin: ready_queue is null");
  return NULL;
}

int dispatcher(PCB_p pcb_to_dispatch) {
  char * string; // free this after printing queue
  //save state of current process into its pcb (done in isr)

  if (ready_queue != NULL) {
    // Only dequeue next waiting process if ready queue has something to dequeue
    // otherwise, keep running the current process (usually idl)
    if (ready_queue->size > 0) {
      if (dispatch_counter == 0) { // Only print stuff every 4th context switch (dispatch call)
        dispatch_counter = 0; // reset the counter;
        printf("Switching to: %s\n", PCB_to_string(pcb_to_dispatch));

        // PCB_set_state(temp, running); Only required when doing print stuff
        // because current_processgets set to temp and current_process state
        // get set to running outside of if (ready_queue->size > 0) statement
        PCB_set_state(pcb_to_dispatch, running);

        printf("Now running: %s\n", PCB_to_string(pcb_to_dispatch));
        printf("Returned to Ready Queue: %s\n", PCB_to_string(current_process));
        string = FIFOq_to_string(ready_queue);
        printf("Ready Queue: %s\n\n", string);
        free(string);
      }
      current_process = pcb_to_dispatch;
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

  int return_status;
  PCB_p pcb_to_be_dispatched; // determined by scheduler algorithm RR, SJF, PRR, etc

  while (!FIFOq_is_empty(created_queue)) {
    PCB_p temp = FIFOq_dequeue(created_queue);
    PCB_set_state(temp, ready);
    // printf("Process PID: %lu moved to ready_queue from created_queue\n", PCB_get_pid(temp));
    printf("Process enqueued: %s\n", PCB_to_string(temp));
    FIFOq_enqueue(ready_queue, temp);
  }

  // Determine situation/what kind of interrupt happened;
  // I wonder if interrupt type could be held in a global queue to handle multiple interrupts coming in.

  switch (int_type) {
    case timer: // what if current process is idl process?
      current_process->state = ready; //set to ready even if current_process is idl
      // printf("current_process pid = 0x%lX\n", current_process->pid);
      if (current_process->pid != idl->pid) { //only do this if the current process is not the idl one
        // just a timer interrupt, only need to set state back to ready
        // and place back into ready queue

        if (dispatch_counter != 4) {
          printf("total number of processes: %d\n", total_processes_created);
          printf("Process enqueued: %s\n", PCB_to_string(current_process));
        }
        FIFOq_enqueue(ready_queue, current_process);
      } else {
        printf("current_process was idl process, do not add to ready_queue\n");
      }
      // pcb_to_be_dispatched = round_robin();
      pcb_to_be_dispatched = round_robin();
      // printf("Timer interrupt: Resetting timer_count\n");
      timer_count = 0;
      return_status = NO_ERRORS;
      break;
    case io_1_interrupt: // current_process requested io from device 1
      current_process->state = waiting; // set state to waiting (waiting on io)
      FIFOq_enqueue(io_1_waiting_queue, current_process);
      // pcb_to_be_dispatched = round_robin();
      // Replace next two lines when implemented
      // return_status = INVALID_INPUT;
      // printf("io_1 type passed, not implemented yet, should not happen\n");
      pcb_to_be_dispatched = round_robin();
      printf("io 1 interrupt: Resetting timer_count\n");
      timer_count = 0;
      return_status = NO_ERRORS;
      break;
    case io_2_interrupt: // current_process requested io from device 2
      current_process->state = waiting; // set state to waiting (waiting on io)
      FIFOq_enqueue(io_2_waiting_queue, current_process);
      // pcb_to_be_dispatched = round_robin();
      // Replace next two lines when implemented
      // return_status = INVALID_INPUT;
      // printf("io_2 type passed, not implemented yet, should not happen\n");
      pcb_to_be_dispatched = round_robin();
      printf("io 2 interrupt: Resetting timer_count\n");
      timer_count = 0;
      return_status = NO_ERRORS;
      break;
    case io_1_completion_interrupt:
      //TODO: IMPLEMENT THIS, ITS ALREADY IN CPU loop
      //TODO: DO case for io_2_completion_interrupt
      io_service_request_trap_handler(1);
      //set pcb_to_be_dispatched to current process
      // move pcb_to_be_dispatched = round_robin(); to each case statement
      pcb_to_be_dispatched = current_process; // redispatch current process onto processer
      return_status = NO_ERRORS;
      break;
    case io_2_completion_interrupt:
      //TODO: IMPLEMENT THIS, ITS ALREADY IN CPU loop
      //TODO: DO case for io_2_completion_interrupt
      io_service_request_trap_handler(2); // could change to io_2_completion_interrupt and use that in the io_service_request_trap_handler if statements
      //set pcb_to_be_dispatched to current process
      // move pcb_to_be_dispatched = round_robin(); to each case statement
      pcb_to_be_dispatched = current_process; // redispatch current process onto processer
      return_status = NO_ERRORS;
      break;
    case process_termination_interrupt:
      //this happens when a process is terminated.
      // move current process to terminated queue, save clock() to pcb for terminate time
      current_process
      return_status = process_termination_trap_handler();


      pcb_to_be_dispatched = round_robin();
      printf("Process termination: Resetting timer_count\n");
      timer_count = 0;
      break;
    default: // this should never happen
      return_status = INVALID_INPUT;
      printf("Invalid Interrupt type passed\n");
      break;
  }

  //call dispatcher

  dispatcher(pcb_to_be_dispatched);
  // Additional Housekeeping (currently none)

  // return to pseudo_isr
  return return_status;
}


int pseudo_isr(enum interrupt_type int_type) { //, unsigned long * cpu_pc_register) {
  // Pseudo push of PC to system stack
  sys_stack = pc_register;
  // printf("Setting sys_stack to what was in pc_register: %lu\n", sys_stack);

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
    pc_register = sys_stack;
    return NO_ERRORS;
  }
  printf("current_process was null\n"); //this should never happen
  return NULL_POINTER;
}

int timer_check() {
  if (timer_count == QUANTUM_DURATION) {
    // printf("Time Slice over\n");
    // timer_count = 0; // reset timer NO THIS SHOULD HAPPEN WHEN A NEW PROCESS GETS SET TO RUNNING
    return 1; // timer interrupt happened
  }
  // printf("The timer is at: %d\n", timer_count);
  timer_count++;
  // printf("The timer is at: %d\n", timer_count);
  return 0;
}

int check_for_io1_done_interrupt(void) {
  // printf("io_1_downcounter is: %d\n", io_1_downcounter);
  if (io_1_downcounter > 0) { // if it's greater than zero it needs to downcount
    io_1_downcounter--;
  }
  if (io_1_downcounter == 0) { // if it's zero, set to -1 and return 1
    io_1_downcounter = -1;
    return 1;
  }
  return 0;
}

int check_for_io2_done_interrupt(void) {
  // printf("io_2_downcounter is: %d\n", io_2_downcounter);
  if (io_2_downcounter > 0) { // if it's greater than zero it needs to downcount
    io_2_downcounter--;
  }
  if (io_2_downcounter == 0) { // if it's zero, set to -1 and return 1
    io_2_downcounter = -1;
    return 1;
  }
  return 0;
}

int check_io(PCB_p the_pcb) {
  int i;
  for (i = 0; i < 4; i++) {
    if (the_pcb->io_1_[i] == sys_stack) {
      return 1; // request for device number 1
    }
    if (the_pcb->io_2_[i] == sys_stack) {
      return 2; // request for device number 2
    }
  }
  return 0;
}

int io_service_request_trap_handler(int the_io_device_number) {
  if (the_io_device_number == 1) {
    // if the waiting queue still has stuff in it, the IO device timer should be reset
    if (!FIFOq_is_empty(io_1_waiting_queue)) {
      io_1_downcounter = QUANTUM_DURATION * (rand() % (5 + 1 - 3) + 3); // multiply quantum 3-5 times
    }
  } else if (the_io_device_number == 2) {
    // if the waiting queue still has stuff in it, the IO device timer should be reset
    if (!FIFOq_is_empty(io_2_waiting_queue)) {
      io_1_downcounter = QUANTUM_DURATION * (rand() % (5 + 1 - 3) + 3); // multiply quantum 3-5 times
    }
  }
}

int terminate_check() { // simulates a process throwing a terminate interrupt
  printf("In terminate_check\n");
  if (current_process != NULL) {
    printf("current_process not null\n");
    if (pc_register >= current_process->max_pc) { //TODO: Changed from == to >=
      printf("pc_register >= current_process->maxpc\n");
      pc_register = 0; // reset pc to 0
      current_process->term_count++; // update term count
      printf("term count: %lu, terminate: %lu\n", current_process->term_count, current_process->terminate);
    }
    if (current_process->terminate > 0) { // if terminate == 0, the process does not terminate
      if (current_process->term_count >= current_process->terminate) {
        if (current_process->pid == total_processes_created) {
          printf("Last process is terminating\n");
          exit(1);
        }
        printf("Process terminating\n");
        return 1;
      }
    } else {
      printf("This process does not terminate\n");
    }
    // printf("Process not terminating\n");
    return 0;
  }
  printf("in terminate_check: current_process is null\n");
  return -1;
}

int process_termination_trap_handler() {
  // printf("In process_termination_trap_handler\n");
  if (current_process != NULL) {
    current_process->state = halted;
    current_process->termination = clock();
    FIFOq_enqueue(terminate_queue, current_process);
    return NO_ERRORS;
  }
  return NULL_POINTER;
}


void cpu(void) {

  pc_register = 0;
  int i, run_count;
  int random_pc_increment;
  int error_check;
  int rand_num_of_processes;

  int is_timer_interrupt;
  int is_io_request_interrupt;
  int is_io1_interrupt;
  int is_io2_interrupt;
  int is_terminate_state;

  // main cpu loop
  // for (run_count = 0; run_count < RUN_TIME; run_count++) {
  do {

    //reset the checking values
    error_check = 0; // TODO: Test this statement
    is_io1_interrupt = 0;
    is_io2_interrupt = 0;
    is_io_request_interrupt = 0;
    is_terminate_state = 0;


    if (create_count < CREATE_ITERATIONS) { // Create processes
      rand_num_of_processes = rand() % 5;
      printf("Creating %d processes\n", rand_num_of_processes);
      total_processes_created += rand_num_of_processes;
      for (i = 0; i < rand_num_of_processes; i++) {
        PCB_p temp = PCB_construct();
        PCB_init(temp);
        PCB_randomize_IO_arrays(temp);
        temp->max_pc = 1250; //TODO: Set randomly
        temp->terminate = 1; //TODO: Set randomly
        temp->term_count = 0;
        PCB_set_pid(temp, pid_counter);
        pid_counter++;
        FIFOq_enqueue(created_queue, temp);
      }
      create_count++;
    } // End create processes


    // DEBUG
    if (current_process == NULL) {
      printf("current_process is null\n");
      exit(1);
    }

    // Simulate running of current process (Execute Instruction)
    pc_register++; // increment by one this time, represents a single instruction
    // printf("Executed instruction\n");
    // random_pc_increment = rand() % (4000 + 1 - 3000) + 3000;
    // pc_register += random_pc_increment;


    // TODO: DO I DO ALL THE CHECKS FIRST AND THEN THE PSEUDO ISR CHECKS?

    //Check if process should be terminated
    is_terminate_state = terminate_check();

    if (is_terminate_state) {
      printf("is terminate state\n");
      // exit(1); //TODO: REMOVE THIS AFTER FINDING OUT IF IT WORKS
      error_check = pseudo_isr(process_termination_interrupt);
    }

    // Check for timer interrupt
    is_timer_interrupt = timer_check();


    if (is_timer_interrupt) {
      // printf("is timer interrupt\n");
      error_check = pseudo_isr(timer);
    }

    //check for I/O completion
    is_io1_interrupt = check_for_io1_done_interrupt();
    is_io2_interrupt = check_for_io2_done_interrupt();


    if(is_io1_interrupt) { // io1 completion happened
      printf("is_io1_interrupt\n");
      error_check = pseudo_isr(io_1_completion_interrupt);
      // Pull from io1 waiting queue and put into ready queue
      // FIFOq_enqueue(ready_queue, FIFOq_dequeue(io_1_waiting_queue));
      // if (!FIFOq_is_empty(io_1_waiting_queue)) {
      //   io_1_downcounter = QUANTUM_DURATION * (rand() % (5 + 1 - 3) + 3); // reset down counter
      // }
    }

    if(is_io2_interrupt) { // io2 completion happened
      error_check = pseudo_isr(io_2_completion_interrupt);
      // Pull from io2 waiting queue and put into ready queue
      // FIFOq_enqueue(ready_queue, FIFOq_dequeue(io_2_waiting_queue));
      // if (!FIFOq_is_empty(io_2_waiting_queue)) {
      //   io_2_downcounter = QUANTUM_DURATION * (rand() % (5 + 1 - 3) + 3); // reset down counter
      // }
    }

    // HOW DOES CHECKING IO FOR CURRENT PROCESS WORK IF IT GETS INTERRUPTED BEFORE THESE NEXT CHECKS


    // if one of the I/O arrays has a number with the current pc value in it
    // trigger an IO_service_request_trap_handler which starts an I/O device downcounting
    // and adds the current process into a waiting queue
    if (current_process->pid != idl->pid) { // idl process should never request io
      is_io_request_interrupt = check_io(current_process);
      if (is_io_request_interrupt == io_1_interrupt) {
        error_check = pseudo_isr(io_1_interrupt); // if current_process is asking for io 1 device
      } else if (is_io_request_interrupt == io_2_interrupt) {
        error_check = pseudo_isr(io_2_interrupt); // if current_process is asking for io 2 device
      }
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
    // printf("current_process pid: %lu\n", current_process->pid);
    // printf("idl pid: %lu\n", idl->pid);
    if (current_process->pid == total_processes_created) {
      printf("last process is running\n");
    }
  } while (ready_queue->size > 0 || io_1_waiting_queue-> size > 0 || io_2_waiting_queue->size > 0 || created_queue->size > 0 || current_process->pid != idl->pid);// End main cpu loop
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
  io_1_downcounter = -1;
  io_2_downcounter = -1;
  timer_count = 0;
  total_processes_created = 0;

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
  terminate_queue = FIFOq_construct();
  FIFOq_init(terminate_queue);

  // Run a cpu
  char * queue_string;
  cpu();
  printf("Ready Queue size: %d\n", ready_queue->size);
  printf("IO 1 Queue size: %d\n", io_1_waiting_queue->size);
  printf("IO 2 Queue size: %d\n", io_2_waiting_queue->size);
  printf("Terminated Queue size: %d\n", terminate_queue->size);
  queue_string = FIFOq_to_string(terminate_queue);
  printf("%s\n", queue_string);
  free(queue_string);
  printf("Total processes created: %d\n", total_processes_created);

  // Cleanup / free stuff to not have memory leaks
  FIFOq_destruct(ready_queue);
  FIFOq_destruct(created_queue);

  if (current_process != NULL) {
    if (current_process->pid != idl->pid) {
      printf("should never happen, idle process should always be the last thing running********\n");
      PCB_destruct(current_process);
    }
  }
  PCB_destruct(idl);

  return 0;

}
