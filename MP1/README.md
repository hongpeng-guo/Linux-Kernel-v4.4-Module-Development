step 1: "make" to compile the code, 
        you will get hg5_MP1.ko and userapp

step 2: "sudo insmod hg5_MP1.ko" to install
        the module.

step 3: "./userapp" will let the userapp run. It
        is a multi-processes program. several processes
        will register themselves to the status file.

step 4: "sudo rmmod mp1" will remove the module. 
