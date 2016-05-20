#!/bin/bash
# debug script
# removes old run file
# removes old output files
# recompiles code with -g flag

rm run
rm output.txt
gcc -g pcb.c errors.c fifo_queue.c priority_queue.c operating_system.c -o run

