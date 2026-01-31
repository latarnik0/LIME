#!/bin/bash

grep -oE "(^cpu+[0-9]*+( +[^ ]+){7})" /proc/stat > /home/user/monitorRes/cpud.txt
grep -oE "(^intr+( +[^ ]+){1})|(^ctxt+( +[^ ]+){1})|(^btime+( +[^ ]+){1})|(^processes+( +[^ ]+){1})|(^procs_running+( +[^ ]+){1})|(^procs_blocked+( +[^ ]+){1})" /proc/stat > /home/user/monitorRes/procs.txt
ps aux > /home/user/monitorRes/procs2.txt
df -h | grep "/dev/*" > /home/user/monitorRes/diskinfo.txt


