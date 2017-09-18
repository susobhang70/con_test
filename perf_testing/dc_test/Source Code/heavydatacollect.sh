#!/bin/bash

stress -c 50 -t 11000 &
while(true)
do
    3>>HogCPU.log perf stat --log-fd 3 -e cpu-clock,task-clock,context-switches -p 17615 sleep 10
done