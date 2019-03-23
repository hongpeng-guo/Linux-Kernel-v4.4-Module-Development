#define LINUX
#define _POSIX_C_SOURCE 200112L
#include "mp2_main.h"
#include "scheduler.h"

#define DEBUG 1
#define FILENAME "status"
#define DIRECTORY "mp2"
#define BUFFER_MAX 2048

// Declare global variables.
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;
static struct mp2_task_struct tl;
static struct kmem_cache* mp2_task_cache; 
static struct task_struct* dispaching_thread;


// extern variables shared with scheduler.c
struct mp2_task_struct* ct = NULL;
struct mutex tl_lock = __MUTEX_INITIALIZER(tl_lock);
struct mutex ct_lock = __MUTEX_INITIALIZER(ct_lock);
spinlock_t lock;

/*
The timer callback funcion. it executes everytime when the period 
of a task expires.
*/
void mp2_timer_callback(unsigned long data){
    struct mp2_task_struct *itr;
    // Entering critical section.
    printk(KERN_ALERT "Enter timer callback\n");
    spin_lock(& lock);
    list_for_each_entry(itr, & tl.list, list){
        if (itr->pid == data){
            itr->state = READY;
        }
    }
    spin_unlock(& lock);
    wake_up_process(dispaching_thread); 
    printk(KERN_ALERT "Leave timer callback\n");
    return;
}


/*
The logic for kernel dispaching thread
*/
int dispaching_thread_function(void* data){
    struct mp2_task_struct *highest = NULL;
    struct sched_param sparam;

    while(!kthread_should_stop()){
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
        
        printk(KERN_ALERT "Enter dispaching thread\n");
        
        highest = Task_get_highest(& tl);
        mutex_lock_interruptible(& ct_lock);
        if (highest == NULL){
            if (ct != NULL){
                sparam.sched_priority = 0;
                ct->state = READY;
                sched_setscheduler(ct->linux_task, SCHED_NORMAL, &sparam);
            }
        }else{
            if (ct != NULL && ct->period > highest->period){
                sparam.sched_priority = 0;
                ct->state = READY;
                sched_setscheduler(ct->linux_task, SCHED_NORMAL, &sparam);
            }
            if(ct == NULL || ct->period > highest->period){    
                wake_up_process(highest->linux_task);
                highest->state = RUNNING;
                sparam.sched_priority = 99;
                sched_setscheduler(highest->linux_task, SCHED_FIFO, &sparam);
                ct = highest;
            }
        }
        mutex_unlock(& ct_lock);
    }
    printk(KERN_ALERT "Leave Dispaching thread\n");
    return 0;
}


/*
Read the content of /proc/mp2/status, which traverses the kernel
linked list to obtain all the application info.
*/
static ssize_t mp2_read(struct file *file, char __user *buffer, size_t count, loff_t *data)
{
    int copied;
    char * buf;
    struct mp2_task_struct *itr;
    
    // Bool logical help identify the whether mp1_read should stop. 
    static int finished = 0;
    if (finished){
        finished = 0;
        return 0;
    }
    finished = 1;

    #ifdef DEBUG
    printk(KERN_ALERT "Enter mp1_read\n");
    #endif
    buf = (char *) kmalloc(count, GFP_KERNEL);
    copied = 0;
    // Entering the critical section.
    mutex_lock_interruptible(& tl_lock); 
    list_for_each_entry(itr, &tl.list, list){
        copied += sprintf(buf+copied, "%u: %llu, %llu, %u\n", itr->pid, itr->period, itr->proc_time, itr->state);        
    }
    mutex_unlock(& tl_lock);
    copy_to_user(buffer, buf, copied);
    kfree(buf);
    return copied;
}

static ssize_t mp2_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    //variables decleration.
    char op;
    uint32_t pid;
    uint64_t period, proc_time;
    char * buf = (char *) kmalloc(count, GFP_KERNEL);
    
    //parsing input string and cast to three cases
    copy_from_user(buf, buffer, count);
    printk(KERN_ALERT "%s\n", buf);
    op = buf[0];
    switch(op){
        case 'R':
            if(sscanf(strsep(& buf, ","), "%c", & op) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            if(sscanf(strsep(& buf, ","), "%u", & pid) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            if(sscanf(strsep(& buf, ","), "%llu", & period) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            if(sscanf(strsep(& buf, ","), "%llu", & proc_time) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            Task_register(&tl, pid, period, proc_time, mp2_task_cache); 
            break;
        case 'Y':
            if(sscanf(strsep(& buf, ","), "%c", & op) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            if(sscanf(strsep(& buf, ","), "%u", & pid) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            printk(KERN_ALERT "%u\n", pid);
            Task_yeild(&tl, pid, dispaching_thread);
            break; 
        case 'D':
            if(sscanf(strsep(& buf, ","), "%c", & op) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            if(sscanf(strsep(& buf, ","), "%u", & pid) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            Task_deregister(&tl, pid, mp2_task_cache, dispaching_thread); 
            break;    
    }
	return count;
}

static const struct file_operations mp2_file = {
	.owner = THIS_MODULE,
	.read = mp2_read,
	.write = mp2_write,
};

// mp1_init - Called when module is loaded
int __init mp2_init(void)
{
    #ifdef DEBUG
    printk(KERN_ALERT "MP2 MODULE LOADING\n");
    #endif
    // /proc file system initialization.
    proc_dir = proc_mkdir(DIRECTORY, NULL);
    proc_entry = proc_create(FILENAME, 0666, proc_dir, & mp2_file); 
    printk(KERN_ALERT "MP2 MODULE LOADING\n");
   
	Task_init(&tl);
    spin_lock_init(& lock);
    mp2_task_cache = KMEM_CACHE(mp2_task_struct,SLAB_PANIC);
    dispaching_thread = kthread_run(dispaching_thread_function, NULL, "dispaching_thread");
    
    #ifdef DEBUG 
    printk(KERN_ALERT "MP2 MODULE LOADED\n");
    #endif
    return 0;   
}

// mp1_exit - Called when module is unloaded
void __exit mp2_exit(void)
{
    #ifdef DEBUG
    printk(KERN_ALERT "MP2 MODULE UNLOADING\n");
    #endif
    
    kthread_stop(dispaching_thread);
    mutex_destroy(& tl_lock);
    mutex_destroy(& ct_lock);
    Task_destroy(&tl, mp2_task_cache); 
    
    //remove proc file system.
    remove_proc_entry("status", proc_dir);
    remove_proc_entry("mp2", NULL);
   
    #ifdef DEBUG
    printk(KERN_ALERT "MP2 MODULE UNLOADED\n");
    #endif

    return;
}

// Register init and exit funtions
module_init(mp2_init);
module_exit(mp2_exit);
