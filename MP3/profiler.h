#ifndef __MP3_SCHEDULER_INCLUDE__
#define __MP3_SCEDULAER_INCLUDE__

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>

struct mp3_task_struct{
    struct task_struct* linux_task;
    struct list_head list;
    int pid;
    uint32_t last_jiffies;
    uint32_t utime;
    uint32_t stime;
    uint32_t maj_flt;
    uint32_t min_flt;
};

extern struct mp3_task_struct tl;
extern spinlock_t lock;
extern long *vmem_buf;

void wq_handler(struct work_struct *w);

void TaskList_init(void);
void TaskList_destroy(void);
int Task_register(uint32_t pid);
int Task_deregister(uint32_t pid);

#endif
