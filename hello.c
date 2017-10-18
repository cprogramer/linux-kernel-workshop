
#define DDEBUG
#define MAX_NAME 32

#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/debugfs.h>
#include <linux/stddef.h>
#include <linux/jiffies.h>
#include <asm/page.h>
#include <linux/slab.h>
#include <linux/seqlock.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Petar Misic");
MODULE_DESCRIPTION("My first module :)");

struct identity {
	struct list_head list;
	char name[MAX_NAME];
	int id;
};

static struct list_head *my_list;
static struct kmem_cache *cache;
static wait_queue_head_t my_wait_queue;
static struct task_struct *my_kthread; 

char output[] = "ZuehlkeCamp2017\n";
char cache_name[] = "my list cache";
static int id_counter;
struct dentry *root;
struct dentry *id;
seqlock_t lock;

int identity_create(char *name,int id)
{
	struct identity *new_ones;
	new_ones = kmem_cache_alloc(cache, GFP_KERNEL);
    if(strlen(name) >= 31) {
        return -EINVAL;
    }
    strcpy(new_ones->name, name);
    new_ones->id = id;
    list_add_tail(&new_ones->list, my_list);
    return 0;
}

int identity_create_trunc(char *name,int id)
{
    struct identity *new_ones;
    new_ones = kmem_cache_alloc(cache, GFP_KERNEL);
    strncpy(new_ones->name, name, MAX_NAME - 1);
    new_ones->id = id;
    list_add_tail(&new_ones->list, my_list);
    return 0;
}

int my_thread(void *data)
{
    int res;
    while(!kthread_should_stop()) {
        res = wait_event_interruptible_timeout(my_wait_queue, kthread_should_stop(), HZ);
        if(!res) {
            pr_info("condition zero!\n");
        }
    }
    return 0;
}

struct identity *identity_find(int id)
{
	struct list_head *ptr;
    struct identity *entry;

    list_for_each(ptr, my_list) {
        entry = list_entry(ptr, struct identity, list);
        if (entry->id == id) {
            return entry;
        }
    }
    return NULL;
}


void identity_destroy(int id)
{
    struct list_head *ptr,*next;
    struct identity *entry;

    list_for_each_safe(ptr, next, my_list) {
        entry = list_entry(ptr, struct identity, list);
        if (entry->id == id) {
            list_del(&entry->list);
            kmem_cache_free(cache, entry);
        }
    }
}

struct identity *pop_identity(void)
{
    struct identity *entry = NULL;

    write_seqlock(&lock);
    if(my_list->next != NULL) {
        entry = list_entry(my_list->next, struct identity, list);
        list_del(&entry->list);
    }
    write_sequnlock(&lock);

    return entry;
}

int write(char *name)
{
    int res;
    write_seqlock(&lock);
    res = identity_create_trunc(name, ++id_counter);
    write_sequnlock(&lock);
    wake_up(&my_wait_queue);
    return res;
}

int identity_destroy_all(void)
{
	struct list_head *ptr,*next;
    struct identity *entry;
    
    list_for_each_safe(ptr, next, my_list) {
        entry = list_entry(ptr, struct identity, list);
        identity_destroy(entry->id);
    }

    return 0;
}


static ssize_t simple_read(struct file *opened_file,
	char __user *user, size_t amount, loff_t *offset)
{
	return simple_read_from_buffer(user, amount, offset,
		output, sizeof(output));
}

static const struct file_operations id_fops = {
	.owner			= THIS_MODULE,
	.read			= simple_read
};

static struct miscdevice misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "Zuehlke"
};

static int __init misc_init(void)
{
    int error;

    error = misc_register(&misc_device);
    if (error) {
        pr_err("can't misc_register :(\n");
        return error;
    }
    init_waitqueue_head(& my_wait_queue);
    seqlock_init(&lock);
    id_counter = 0;
    my_list = kmalloc(sizeof(struct list_head),GFP_KERNEL);
    cache = kmem_cache_create(cache_name, sizeof(struct identity), 0, 0, NULL);
    INIT_LIST_HEAD(my_list);
    if((error = identity_create("Hello",52)) != 0) {
        goto error_handle;
    }
    if((error = write("Pera")) != 0) {
        goto error_handle;
    }

    pr_info("I found %s",identity_find(1)->name);
    pr_info("I found %s",pop_identity()->name);
    identity_destroy(26);
    if(identity_find(26) == NULL) {
        pr_info("I found nothing man!\n");
    }

    root = debugfs_create_dir("zuehlke", NULL);
    id = debugfs_create_file("id", 0666, root, NULL, &id_fops);

    my_kthread = kthread_run(my_thread, NULL, "Zuehlke Thread!");
    
    return 0;

error_handle:
    identity_destroy_all();
    kfree(my_list);
    return error;
}


static void __exit misc_exit(void)
{
    kthread_stop(my_kthread);
    wake_up(&my_wait_queue);
	identity_destroy_all();
    kfree(my_list);
    kmem_cache_destroy(cache);
    misc_deregister(&misc_device);
    debugfs_remove_recursive(root);
}

module_init(misc_init);
module_exit(misc_exit);
