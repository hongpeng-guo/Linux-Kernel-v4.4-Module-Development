#include "scheduler.h"
#include "mp2_given.h"

// used for fixed point calcaulation
#define SHIFT_BITS 20
#define MAX_UTIL 726663

// initializing the total utility. 
static uint64_t total_util = 0;

// mp2_task_struct list tl is initialized here.
void Task_init(struct mp2_task_struct* tl){
    printk(KERN_ALERT "mp2_task_sctruct init\n");
    INIT_LIST_HEAD(& tl->list);
}

// tl is destroied here. using slab cache to free allocated memory.
void Task_destroy(struct mp2_task_struct* tl, struct kmem_cache* mp2_task_cache){
    struct mp2_task_struct *itr, *tmp;
    mutex_lock_interruptible(& tl_lock);
    list_for_each_entry_safe(itr, tmp, & tl->list, list){
        del_timer(& itr->timer);
        list_del(& itr->list);
        kmem_cache_free(mp2_task_cache, itr);
    }
    mutex_unlock(& tl_lock);
    kmem_cache_destroy(mp2_task_cache);
}

// register a new task to /proc/mp2/status
int Task_register(struct mp2_task_struct* tl, uint32_t pid, uint64_t period, uint64_t proc_time, struct kmem_cache* mp2_task_cache){
    struct mp2_task_struct *new_mp2_task, *itr;
    
    // first check if the task already exists.
    mutex_lock_interruptible(& tl_lock);
    list_for_each_entry(itr, & tl->list, list){
        if (itr->pid == pid){ 
            printk(KERN_ALERT "process already exists\n");
            return -1;
        }
    }
    mutex_unlock(& tl_lock);

    // admission control is handled here.
    if (total_util + (proc_time << SHIFT_BITS)/ period >= MAX_UTIL){
        printk(KERN_ALERT "max cpu utility reaches\n");
        return -1;
    }

    // passing admission control. 
    // fisrt update new totoal utility.
    total_util += (proc_time << SHIFT_BITS)/ period;

    // initialize every memeber value of the new mp2_task_struct.
    new_mp2_task = (struct mp2_task_struct *) kmem_cache_alloc(mp2_task_cache, GFP_KERNEL);
    new_mp2_task -> pid = pid;
    new_mp2_task -> period = period;
    new_mp2_task -> proc_time = proc_time;
    new_mp2_task -> next_ddl = 0;
    new_mp2_task -> state = SLEEPING;
    new_mp2_task -> linux_task = find_task_by_pid(pid);
    INIT_LIST_HEAD(& new_mp2_task->list);
    setup_timer(& new_mp2_task->timer, mp2_timer_callback, pid);

    // critical section used to add this task into the task list.
    mutex_lock_interruptible(& tl_lock);
    list_add_tail(& new_mp2_task->list, & tl->list);
    mutex_unlock(& tl_lock);
    return 0;
}

int Task_yeild(struct mp2_task_struct* tl, uint32_t pid, struct task_struct* dispaching_thread){
    struct mp2_task_struct *itr, *yeild_task = NULL;
    printk(KERN_ALERT "Enter yeild\n");

    // critical section to check if the yeild sender exists.
    mutex_lock_interruptible(& tl_lock);
    list_for_each_entry(itr, &tl->list, list){
        if (itr->pid == pid){yeild_task = itr;}
    }
    mutex_unlock(& tl_lock);
    if (yeild_task == NULL){
        printk(KERN_ALERT "yeild task does not exist\n");
        return -1;
    }

    // critical section used to reset timer and check expires.
    mutex_lock_interruptible(& tl_lock);
    if(yeild_task->next_ddl == 0){
        yeild_task->next_ddl = jiffies + msecs_to_jiffies(yeild_task -> period);
    }else{
        yeild_task->next_ddl += msecs_to_jiffies(yeild_task -> period);
        if (jiffies > yeild_task -> next_ddl){
            printk(KERN_ALERT "ddl expires\n");
            return -1;
        }
    }
    mod_timer(&yeild_task->timer, yeild_task->next_ddl);

    // chaning the yeild sender to SLEEPING mode and change 
    // the value of current running task.
    yeild_task ->state = SLEEPING; 
    set_task_state(yeild_task->linux_task,TASK_UNINTERRUPTIBLE);
    schedule();
    mutex_lock_interruptible(& ct_lock);
    ct = NULL;
    mutex_unlock(& ct_lock);
    mutex_unlock(& tl_lock);

    // call dispaching thread to run possible READY threads.
    wake_up_process(dispaching_thread);
    printk(KERN_ALERT "yeild finished\n");
    return 0;
}

// Deregister process when a task finishes.
int Task_deregister(struct mp2_task_struct* tl, uint32_t pid, struct kmem_cache* mp2_task_cache, struct task_struct* dispaching_thread){
    struct mp2_task_struct *itr, *tmp, *dereg_task = NULL;

    // critical section to find this task by pid.
    mutex_lock_interruptible(& tl_lock);
    list_for_each_entry_safe(itr, tmp, &tl->list, list){
        if (itr->pid == pid){
            dereg_task = itr;
            break;
        }
    }
    mutex_unlock(& tl_lock);
    if (dereg_task == NULL){
        printk(KERN_ALERT "deregister task does not exist\n");
        return -1;
    }

    // similar to Task_destroy(), remove the task from critical section.
    // if the task is currently running. Call dispaching_thread to reschedule.
    mutex_lock_interruptible(&tl_lock);
    if (dereg_task == ct){
        mutex_lock_interruptible(& ct_lock);
        ct = NULL;
        mutex_unlock(& ct_lock);
        wake_up_process(dispaching_thread);
    }
    total_util -= (dereg_task->proc_time << SHIFT_BITS)/ dereg_task->period;
    list_del(& dereg_task->list);
    del_timer(& dereg_task->timer);
    kmem_cache_free(mp2_task_cache, dereg_task);
    mutex_unlock(&tl_lock);
    return 0;
}

// find the task with highest priority wich is in READY state.
struct mp2_task_struct* Task_get_highest(struct mp2_task_struct* tl){
    struct mp2_task_struct *itr, *highest = NULL;
    mutex_lock_interruptible(& tl_lock);
    list_for_each_entry(itr, &tl->list, list){
        if (itr->state == 2){
            if (highest == NULL){highest = itr;}
            else if (itr->period < highest->period){
                highest = itr;
            }
        }
    }
    mutex_unlock(& tl_lock);
    return highest;
}
