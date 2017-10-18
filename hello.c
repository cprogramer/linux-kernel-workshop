
#define DDEBUG

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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Petar Misic");
MODULE_DESCRIPTION("My first module :)");

struct identity {
	struct list_head list;
	char name[32];
	int id;
};

static struct list_head *my_list;

char output[] = "ZuehlkeCamp2017\n";
struct dentry *root;
struct dentry *id;

int identity_create(char *name,int id)
{
	struct identity *new_ones;
	new_ones = kmalloc(sizeof(struct identity), GFP_KERNEL);
    if(strlen(name) >= 31) {
        return -EINVAL;
    }
    strcpy(new_ones->name, name);
    new_ones->id = id;
    list_add(&new_ones->list, my_list);
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
        	kfree(entry);
        }
    }
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
    my_list = kmalloc(sizeof(struct list_head),GFP_KERNEL);
    INIT_LIST_HEAD(my_list);
    identity_create("Hello",52);
    identity_create("Pera",26);

    pr_info("I found %s",identity_find(26)->name);
    identity_destroy(26);
    if(identity_find(26) == NULL) {
        pr_info("I found nothing man!\n");
    }

    root = debugfs_create_dir("zuehlke", NULL);
    id = debugfs_create_file("id", 0666, root, NULL, &id_fops);
    
    return 0;
}


static void __exit misc_exit(void)
{
	identity_destroy_all();
    kfree(my_list);
    misc_deregister(&misc_device);
    debugfs_remove_recursive(root);
}

module_init(misc_init);
module_exit(misc_exit);
