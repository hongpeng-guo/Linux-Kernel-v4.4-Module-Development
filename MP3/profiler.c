#include "profiler.h"
#include "mp3_given.h"

#define SAMPLE_DELAY 50
#define VMEM_SIZE 128 * 4 * 1024

#define JIFFIE  0
#define MINOR   1
#define MAJOR   2
#define UTIL    3


static workqueue_struct* wq;
DECLARE_DELAYED_WORK(wq_fn, wq_handler);

// pushing current state info to the buffer.
uint32_t vmem_idx = 0; 
void vmem_buf_push_next(uint32_t last_jiffies, uint32_t min_flt, uint32_t \
                        maj_flt uint32_t util){
    
    vmem[4 * vmem_idex + JIFFIE] = jiffies;
    vmem[4 * vmem_idex + MINOR] = min_flt;
    vmem[4 * vmem_idex + MAJOR] = maj_flt;
    vmem[4 * vmem_idex + UTIL] = util * 1000 / (jiffies - last_jiffies);
    vmem_idx += 1;
    if (4 * vmem_idx >= VMEM_SIZE) vmem_idx = 0;
    return;
}


// mp3_task_struct list tl is initialized here.
void TaskList_init(void){
    printk(KERN_ALERT "mp3_task_sctruct init\n");
    INIT_LIST_HEAD(& tl->list);
    spin_lock_init(& lock);
    vmem_buf = (long*) vmalloc(VMEM_SIZE * sizeof(long));
    wq = create_singlethread_workqueue("wq");
    return;
}


// tl is destroied here. using slab cache to free allocated memory.
void TaskList_destroy(void){
    struct mp3_task_struct *itr, *tmp;
    spin_lock(& lock);
    list_for_each_entry_safe(itr, tmp, & tl->list, list){
        list_del(& itr->list);
        kfree(itr);
    }
    spin_unlock(& lock);

    cancel_delayed_workqueue(& wq);
    flush_workqueue(wq);
    destroy_workqueue(wq);

    vfree(vmem_buf);
}


// register a new task to /proc/mp3/status
int Task_register(uint32_t pid){
    struct mp3_task_struct *new_mp3_task, *itr;
    
    // first check if the task already exists.
    list_for_each_entry(itr, & tl->list, list){
        if (itr->pid == pid){ 
            printk(KERN_ALERT "process already exists\n");
            return -1;
        }
    }

    // initialize every memeber value of the new mp3_task_struct.
    new_mp3_task = (struct mp3_task_struct *) kmem_cache_alloc(mp3_task_cache, GFP_KERNEL);
    new_mp3_task -> pid = pid;
    new_mp3_task -> last_jiffies = jiffies;
    new_mp3_task -> min_flt = 0;
    new_mp3_task -> max_flt = 0;
    new_mp3_task -> utime = 0;
    new_mp3_task -> stime = 0;
    new_mp3_task -> linux_task = find_task_by_pid(pid);
    INIT_LIST_HEAD(& new_mp3_task->list);
    
    // critical section used to add this task into the task list.
    spin_lock(& lock);
    if (list_empty(& tl->list)){
        queue_delayed_work(wq, & wq_fn, msecs_to_jiffies(SAMPLE_DELAY));
    }
    list_add_tail(& new_mp3_task->list, & tl->list);
    spin_unlock(& lock);
    return 0;
}


// Deregister process when a task finishes.
int Task_deregister(uint32_t pid){
    struct mp3_task_struct *itr, *tmp;
    bool found = false;

    // critical section to find this task by pid.
    spin_lock(& lock)
    list_for_each_entry_safe(itr, tmp, &tl->list, list){
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

    if (empty_list(& tl->list)){
        cancle_delayed_workqueue(& wq);
        flush_workqueue(wq);
    }
    spin_unlock(& lock);
    return 0;
}


// execute periodically, retrive cpu use from kernel.
void wq_handler(struct mp3_task_struct* notuse){
    struct mp3_task_struct *itr;
    uint32_t min_flt, maj_flt, utime, stime;
    spin_lock(& lock);
    list_for_each_entry(itr, &tl->list, list){
        if (get_cpu_use(itr->pid, &min_flt, &maj_flt, &utime, &stime)){
            printk (KERN_ALERT "get cpu use faile for %d\n", itr->pid);
            return;
        }
        itr -> min_flt = min_flt;
        itr -> maj_flt = maj_flt;
        itr -> utime = utime;
        itr -> stime = stime;
        vmem_buf_push_next(itr->last_jiffies, min_flt, maj_flt, utime+stime);
        itr -> last_jiffies = jiffies; 
    }
    spin_unlock(& lock);
    return;
}
