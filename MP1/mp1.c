#define LINUX

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include "mp1_given.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("20");
MODULE_DESCRIPTION("CS-423 MP1");

#define DEBUG 1
#define FILENAME "status"
#define DIRECTORY "mp1"
#define BUFFER_MAX 2048

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;

struct Process{
    unsigned int pid;
    time_t time;
    struct list_head list;
};
struct Process p_list;

struct timer_list mp1_timer;
struct workqueue_struct *mp1_wq;
struct work_struct *mp1_w;
spinlock_t lock;

static void mp1_time_handler(struct timer_list* timer){
    struct Process *itr, *tmp;
    spin_lock(& lock);
    list_for_each_entry_safe(itr, tmp, &p_list.list, list){
        if (get_cpu_use(itr->pid, & itr->time) == -1){
            list_del(& itr->list);
            kfree(itr);
        }
    }
    spin_unlock(& lock);
    mod_timer(timer, jiffies + msecs_to_jiffies(5000));
    #ifdef DEBUG
    printk(KERN_ALERT "timer called at %u\n", jiffies_to_msecs(jiffies));
    #endif
    return;
}

static void mp1_work_handler(struct work_struct * work){
    queue_work(mp1_wq, (struct work_struct *) work);
    return;    
}

static ssize_t mp1_read(struct file *file, char __user *buffer, size_t count, loff_t *data)
{
    //implementation goes here
    int copied;
    char * buf;
    struct Process *itr;
    
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
    spin_lock(& lock); 
    //put something into the buf, update copied
    list_for_each_entry(itr, &p_list.list, list){
        copied += sprintf(buf+copied, "%u: %ld\n", itr->pid, itr->time);        
    }
    spin_unlock(& lock);
    copy_to_user(buffer, buf, copied);
    kfree(buf);
    return copied;
}

static ssize_t mp1_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    //implementation goes here
	struct Process *new_proc;
	char * buf = (char *) kmalloc(count, GFP_KERNEL);	
    static bool timer_start = false;

    copy_from_user(buf, buffer, count);
	new_proc = (struct Process *) kmalloc(sizeof(struct Process), GFP_KERNEL);
	sscanf(buf, "%u", &(new_proc->pid));
	kfree(buf);
	new_proc->time = 0;
    INIT_LIST_HEAD(& new_proc->list);
	
    #ifdef DEBUG
    printk(KERN_ALERT "Enter mp1_write\n");
    printk(KERN_ALERT "%u\n", new_proc->pid);
    #endif

	spin_lock(& lock);
	list_add_tail(&new_proc->list, &p_list.list);
	spin_unlock(& lock); 

    if (!timer_start){
        timer_setup(&mp1_timer, mp1_time_handler, 0);
        mod_timer(&mp1_timer, jiffies);
        timer_start = true;
    }
    
	return count;
}

static const struct file_operations mp1_file = {
	.owner = THIS_MODULE,
	.read = mp1_read,
	.write = mp1_write,
};

// mp1_init - Called when module is loaded
int __init mp1_init(void)
{
    #ifdef DEBUG
    printk(KERN_ALERT "MP1 MODULE LOADING\n");
    #endif
    // Insert your code here ...
    proc_dir = proc_mkdir(DIRECTORY, NULL);
    proc_entry = proc_create(FILENAME, 0666, proc_dir, & mp1_file); 
   
	INIT_LIST_HEAD(&p_list.list);
    spin_lock_init(& lock);

    mp1_wq = create_workqueue("mp1_queue");
    if (mp1_wq){
        mp1_w = (struct work_struct *)kmalloc(sizeof(struct work_struct), GFP_KERNEL);
        if(mp1_w){
            INIT_WORK((struct work_struct *) mp1_w, mp1_work_handler);
        }else{
            printk(KERN_ERR "Work not allocated correctly\n");
        }
    }

    #ifdef DEBUG 
    printk(KERN_ALERT "MP1 MODULE LOADED\n");
    #endif
    return 0;   
}

// mp1_exit - Called when module is unloaded
void __exit mp1_exit(void)
{
    struct Process *itr, *tmp; 
    
    #ifdef DEBUG
    printk(KERN_ALERT "MP1 MODULE UNLOADING\n");
    #endif

    list_for_each_entry_safe(itr, tmp, &p_list.list, list){
        list_del(& itr->list);
        kfree(itr);
    }

    flush_workqueue(mp1_wq);
    destroy_workqueue(mp1_wq);

    del_timer(&mp1_timer);
    kfree(mp1_w);
    
    remove_proc_entry("status", proc_dir);
    remove_proc_entry("mp1", NULL);
   
    #ifdef DEBUG
    printk(KERN_ALERT "MP1 MODULE UNLOADED\n");
    #endif

    return;
}

// Register init and exit funtions
module_init(mp1_init);
module_exit(mp1_exit);
