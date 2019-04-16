#ifndef __MP3_SCHEDULER_INCLUDE__
#define __MP3_SCEDULAER_INCLUDE__

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

struct mp3_task_struct{
    struct task_struct* linux_task;
    struct list_head list;
    int pid;
    unsigned long last_jiffies;
    unsigned long utime;
    unsigned long stime;
    unsigned long maj_flt;
    unsigned long min_flt;
};

extern struct mp3_task_struct tl;
extern spinlock_t lock;
extern unsigned long *vmem_buf;

void wq_handler(struct work_struct *w);

void TaskList_init(void);
void TaskList_destroy(void);
int Task_register(int pid);
int Task_deregister(int pid);

#endif
