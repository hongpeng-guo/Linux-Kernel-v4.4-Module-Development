#ifndef __MP2_SCHEDULER_INCLUDE__
#define __MP2_SCEDULAER_INCLUDE__

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/module.h>

#define SLEEPING    1
#define READY       2
#define RUNNING     3

struct mp2_task_struct{
    struct task_struct* linux_task;
    struct timer_list timer;
    struct list_head list;
    uint32_t state;
    uint32_t pid;
    uint64_t period;
    uint64_t proc_time;
    uint64_t next_ddl;
};

extern struct mp2_task_struct* ct;
extern struct mutex tl_lock;
extern struct mutex ct_lock;

int dispaching_thread_function(void*);
void mp2_timer_callback(unsigned long);

void Task_init(struct mp2_task_struct*);
void Task_destroy(struct mp2_task_struct*, struct kmem_cache*);
int Task_register(struct mp2_task_struct*, uint32_t, uint64_t, uint64_t, struct kmem_cache*);
int Task_yeild(struct mp2_task_struct*, uint32_t, struct task_struct*);
int Task_deregister(struct mp2_task_struct*, uint32_t, struct kmem_cache*, struct task_struct*);
struct mp2_task_struct* Task_get_highest(struct mp2_task_struct*);

#endif
