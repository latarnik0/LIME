#!/bin/bash

grep -oE "(^cpu+[0-9]*+( +[^ ]+){7})" /proc/stat > /home/heron/monitorRes/cpud.txt
grep -oE "(^intr+( +[^ ]+){1})|(^ctxt+( +[^ ]+){1})|(^btime+( +[^ ]+){1})|(^processes+( +[^ ]+){1})|(^procs_running+( +[^ ]+){1})|(^procs_blocked+( +[^ ]+){1})" /proc/stat > /home/heron/monitorRes/procs.txt
ps aux > /home/heron/monitorRes/procs2.txt
df -h | grep "/dev/*" > /home/heron/monitorRes/diskinfo.txt


