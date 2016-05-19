#ifndef OPERATING_SYSTEM_H_
#define OPERATING_SYSTEM_H_
#include "priority_queue.h"
#include <time.h>

enum interrupt_type {
  io_1, io_2, timer
};

int dispatcher(void);

int scheduler(enum interrupt_type);

int pseudo_isr(enum interrupt_type, unsigned long *);

void cpu(void);

#endif
