# Linux_Kernel_Module
 A Simple loadable kernel module which prints number of process, cpu core, details of runnig process etc based on user input

This kernel module name is "hello"


after loading the kernel module give any of the following command

1) echo "proc_count" > /proc/hello 
2) echo "proc_details" > /proc/hello 
3) echo "cpu_info" > /proc/hello 
4) echo "current_time" > /proc/hello 


After that write following 

cat /proc/hello 

1) will show the number of running process in the system
2) will show the pid and proc name running in the system
3) will show the number of cpu cores available in your processor
4) will show the current system time and date
