#!/bin/bash
# run script
# removes old run file
# removes old output files
# recompiles code

rm run
rm output.txt
gcc pcb.c errors.c fifo_queue.c priority_queue.c operating_system.c -o run


