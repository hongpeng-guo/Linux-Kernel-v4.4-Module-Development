This kernel scheduler consists of four major files.

"mp2_main.h", "mp2_main.c", "scheduler.h", "scheduler.c"

After compiling the file, simply install the kernel module
by calling "insmod hg5_MP2.ko"

Removing the module by calling "rmmod hg5_MP2.ko"

After the module is installed you can use userapp to test it.

"./userapp <period> <proc_time> <num_of_period>"

In this design, the <proc_time> should be 100.
