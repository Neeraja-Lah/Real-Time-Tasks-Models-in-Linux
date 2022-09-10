CSE438-Embedded Systems Programming

Assignment 1 Real-Time Tasks Models in Linux
Name: Neeraja Lahudva Kuppuswamy
ASU ID: 1224187432

Description : 

In this assignment, we have developed a project to use POSIX threads to implement the Periodic and Aperiodic Tasks on Linux. 

STEPS TO COMPILE 
1. Unzip the ESP-LahudvaKuppuswamy-N-assgn01.zip.
2. The source file (.c file) must be updated with the Keyboard event device file. The input event for your keyboard can be checked by using the following command on your PC “sudo cat /proc/bus/input/devices”. 
3. Prepare the make file and set the make command: gcc -Wall assignment1.c -o assignment1 -lpthread
4. Run the make file using 'make' command and execute the code.

STEPS TO EXECUTE
1. Enter the command ‘sudo ./assignment1’ to run the output file
2. As periodic tasks are running, trigger the event (press the keyboard keys) to observe the aperiodic tasks execution.
3. Enter the command sudo trace-cmd record –e sched_switch taskset –c 0 ./assignment1 
4. The trace.dat file is generated that contains the trace data of assignment1
5. The trace.dat file is imported in kernelshark to view the trace
6. Under Plots, choose the CPU in which the threads are running
7. Tasks option contain the 7 signals which could be traced

