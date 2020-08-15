#define LINUX
#define _POSIX_C_SOURCE 200112L
#include "mp3_main.h"
#include "profiler.h"

#define DEBUG 1
#define FILENAME "status"
#define DIRECTORY "mp3"
#define BUFFER_MAX 2048
#define CDEV_NAME "node"
#define CDEV_MAJOR 0

// Declare global variables.
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_entry;

// the spinlock is only used in time handller.
struct mp3_task_struct tl;
spinlock_t lock;
unsigned long *vmem_buf;


/*
Read the content of /proc/mp3/status, which traverses the kernel
linked list to obtain all the application info. The logic is similar
to MP1 read function.
*/
static ssize_t mp3_read(struct file *file, char __user *buffer, size_t count, loff_t *data)
{
    int copied;
    char * buf;
    struct mp3_task_struct *itr;
    
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
    spin_lock(& lock); 
    list_for_each_entry(itr, &tl.list, list){
        copied += sprintf(buf+copied, "%d\n", itr->pid);        
    }
    spin_unlock(& lock);
    copy_to_user(buffer, buf, copied);
    kfree(buf);
    return copied;
}

static ssize_t mp3_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    //variables decleration.
    char op;
    int pid;
    char * buf = (char *) kmalloc(count, GFP_KERNEL);
    
    //parsing input string and cast to three cases
    copy_from_user(buf, buffer, count);
    printk(KERN_ALERT "%s\n", buf);
    op = buf[0];
    switch(op){
        case 'R':
            if(sscanf(strsep(& buf, " "), "%c", & op) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            if(sscanf(strsep(& buf, " "), "%u", & pid) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            // entering Task_register(), which is defined in scheduler.c
            // and declared in scheduler.h
            printk(KERN_ALERT "BEFORE REG\n");
            Task_register(pid); 
            printk(KERN_ALERT "AFTER REG\n");
            break;
        case 'U':
            if(sscanf(strsep(& buf, " "), "%c", & op) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            if(sscanf(strsep(& buf, " "), "%u", & pid) != 1)
                printk(KERN_ALERT "invilade inputs\n");
            // entering Task_yeild(), which is defined in scheduler.c
            // and declared in scheduler.h
            Task_deregister(pid);
            break; 
    }
	return count;
}

static const struct file_operations mp3_file = {
	.owner = THIS_MODULE,
	.read = mp3_read,
	.write = mp3_write,
};


// character device
dev_t dev;
struct cdev mp3_cdev;

// character device callbacks
static int cdev_mmap_callback(struct file *f, struct vm_area_struct *vma) {
    unsigned long page, pos;
    unsigned long start = vma->vm_start;
    unsigned long size = vma->vm_end - vma->vm_start;

#ifdef DEBUG
    printk(KERN_DEBUG "mmap callback called\n");
#endif

    pos = (unsigned long)vmem_buf;
    while (size > 0) {
        page = vmalloc_to_pfn((void *)pos);
        if (remap_pfn_range(vma, start, page, PAGE_SIZE, PAGE_SHARED)) {
            return -EAGAIN;
        }

        start += PAGE_SIZE;
        pos += PAGE_SIZE;
        if (size > PAGE_SIZE)
            size -= PAGE_SIZE;
        else
            size = 0;
    }

#ifdef DEBUG
    printk(KERN_DEBUG "finish mmap\n");
#endif

    return 0;
}

static const struct file_operations cdev_fops = {
    .owner = THIS_MODULE,
    .open = NULL,
    .release = NULL,
    .mmap = cdev_mmap_callback
};


// mp3_init - Called when module is loaded
int __init mp3_init(void)
{
    int ret;
    #ifdef DEBUG
    printk(KERN_ALERT "MP3 MODULE LOADING\n");
    #endif
    // /proc file system initialization.
    proc_dir = proc_mkdir(DIRECTORY, NULL);
    proc_entry = proc_create(FILENAME, 0666, proc_dir, & mp3_file); 
    printk(KERN_ALERT "MP3 MODULE LOADING\n");
   
    // init /proc/devices/node
    if (!CDEV_MAJOR) {
        ret = alloc_chrdev_region(&dev, 0, 1, CDEV_NAME);

        if (ret) {
            printk(KERN_ERR "Failed to allocate major\n");
            return -1;
        }
    } else {
        dev = MKDEV(CDEV_MAJOR, 0);
    }

    cdev_init(&mp3_cdev, &cdev_fops);
    mp3_cdev.owner = THIS_MODULE;
    mp3_cdev.ops = &cdev_fops;
    cdev_add(&mp3_cdev, dev, 1);


    // global variables initialization.
	TaskList_init();
    
    #ifdef DEBUG 
    printk(KERN_ALERT "MP3 MODULE LOADED\n");
    #endif
    return 0;   
}

// mp1_exit - Called when module is unloaded
void __exit mp3_exit(void)
{
    #ifdef DEBUG
    printk(KERN_ALERT "MP3 MODULE UNLOADING\n");
    #endif
    
    
    //remove proc file system.
    remove_proc_entry("status", proc_dir);
    remove_proc_entry("mp3", NULL);
   
    TaskList_destroy();

    printk(KERN_ALERT "UNLOADING PART 1\n");
    
    cdev_del(&mp3_cdev);
    unregister_chrdev_region(dev, 1);

   
    #ifdef DEBUG
    printk(KERN_ALERT "MP3 MODULE UNLOADED\n");
    #endif

    return;
}

// Register init and exit funtions
module_init(mp3_init);
module_exit(mp3_exit);
