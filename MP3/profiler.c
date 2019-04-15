#include "profiler.h"
#include "mp3_given.h"

#define SAMPLE_DELAY 50
#define VMEM_SIZE 128 * 4 * 1024

#define PID  0
#define MINOR   1
#define MAJOR   2
#define UTIL    3


static struct workqueue_struct* wq;
DECLARE_DELAYED_WORK(wq_fn, wq_handler);

// pushing current state info to the buffer.
unsigned long vmem_idx = 0; 
void vmem_buf_push_next(int pid, unsigned long min_flt, unsigned long \
                        maj_flt, unsigned long util, unsigned long last_jiffies){
    
    vmem_buf[4 * vmem_idx + PID] = (long) pid;
    vmem_buf[4 * vmem_idx + MINOR] = (long) min_flt;
    vmem_buf[4 * vmem_idx + MAJOR] = (long) maj_flt;
    vmem_buf[4 * vmem_idx + UTIL] = (long) util * 100 / 20;
    vmem_idx += 1;
    if (4 * vmem_idx >= VMEM_SIZE) vmem_idx = 0;
    return;
}


// mp3_task_struct list tl is initialized here.
void TaskList_init(void){
    printk(KERN_ALERT "mp3_task_sctruct init\n");
    INIT_LIST_HEAD(& tl.list);
    spin_lock_init(& lock);
    vmem_buf = (unsigned long *) vmalloc(VMEM_SIZE * sizeof(unsigned long));
    memset(vmem_buf, 0, VMEM_SIZE * sizeof(unsigned long));
    wq = create_singlethread_workqueue("wq");
    return;
}


// tl is destroied here. using slab cache to free allocated memory.
void TaskList_destroy(void){
    struct mp3_task_struct *itr, *tmp;
    spin_lock(& lock);
    list_for_each_entry_safe(itr, tmp, & tl.list, list){
        list_del(& itr->list);
        kfree(itr);
    }
    spin_unlock(& lock);

    cancel_delayed_work(& wq_fn);
    flush_workqueue(wq);
    destroy_workqueue(wq);

    vfree(vmem_buf);
}


// register a new task to /proc/mp3/status
int Task_register(int pid){
    struct mp3_task_struct *new_mp3_task, *itr;
    
    // first check if the task already exists.
    list_for_each_entry(itr, & tl.list, list){
        if (itr->pid == pid){ 
            printk(KERN_ALERT "process already exists\n");
            return -1;
        }
    }

    // initialize every memeber value of the new mp3_task_struct.
    new_mp3_task = (struct mp3_task_struct *) kmalloc(sizeof(struct mp3_task_struct), GFP_KERNEL);
    new_mp3_task -> pid = pid;
    new_mp3_task -> last_jiffies = jiffies;
    new_mp3_task -> min_flt = 0;
    new_mp3_task -> maj_flt = 0;
    new_mp3_task -> utime = 0;
    new_mp3_task -> stime = 0;
    new_mp3_task -> linux_task = find_task_by_pid(pid);
    INIT_LIST_HEAD(& new_mp3_task->list);
    
    // critical section used to add this task into the task list.
    spin_lock(& lock);
    if (list_empty(& tl.list)){
        queue_delayed_work(wq, & wq_fn, msecs_to_jiffies(SAMPLE_DELAY));
    }
    list_add_tail(& new_mp3_task->list, & tl.list);
    spin_unlock(& lock);
    return 0;
}


// Deregister process when a task finishes.
int Task_deregister(int pid){
    struct mp3_task_struct *itr, *tmp;
    bool found = false;

    // critical section to find this task by pid.
    spin_lock(& lock);
    list_for_each_entry_safe(itr, tmp, &tl.list, list){
        if (itr->pid == pid){
            list_del(& itr->list);
            kfree(itr);
            found = true;
            break;
        }
    }

    if (! found){
        printk(KERN_ALERT "deregister task does not exist\n");
        spin_unlock(& lock);
        return -1;
    }

    if (list_empty(& tl.list)){
        cancel_delayed_work(& wq_fn);
        flush_workqueue(wq);
    }
    spin_unlock(& lock);
    return 0;
}


// execute periodically, retrive cpu use from kernel.
void wq_handler(struct work_struct* notuse){
    struct mp3_task_struct *itr;
    unsigned long min_flt, maj_flt, utime, stime;
    spin_lock(& lock);
    list_for_each_entry(itr, &tl.list, list){
        if (get_cpu_use(itr->pid, &min_flt, &maj_flt, &utime, &stime)){
            printk (KERN_ALERT "get cpu use faile for %d\n", itr->pid);
            return;
        }
        itr -> min_flt = min_flt;
        itr -> maj_flt = maj_flt;
        itr -> utime = utime;
        itr -> stime = stime;
        vmem_buf_push_next(itr->pid, min_flt, maj_flt, utime+stime, itr->last_jiffies);
        itr -> last_jiffies = jiffies; 
    }
    spin_unlock(& lock);
    queue_delayed_work(wq, & wq_fn, msecs_to_jiffies(SAMPLE_DELAY));
    return;
}
