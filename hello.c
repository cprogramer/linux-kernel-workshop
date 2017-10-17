
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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Petar Misic");
MODULE_DESCRIPTION("My first module :)");

char output[] = "ZuehlkeCamp2017\n";
struct dentry *root;
struct dentry *id;
struct dentry *jiffie;
u64 start;
struct timeval secup;

static ssize_t simple_read(struct file *opened_file,
	char __user *user, size_t amount, loff_t *offset)
{
	return simple_read_from_buffer(user, amount, offset,
		output, sizeof(output));
}

static ssize_t jiffie_read(struct file *opened_file,
	char __user *user, size_t amount, loff_t *offset)
{
	char tmp[100];
	jiffies_to_timeval(get_jiffies_64() - start, &secup);
	sprintf(tmp,"Up and running for %ld seconds!\n",secup.tv_sec);
	return simple_read_from_buffer(user, amount, offset,
		tmp, strlen(tmp));
}

static ssize_t simple_write(struct file *opened_file,
	const char __user *user, size_t amount, loff_t *offset)
{
	int osize = strlen(output);
	char tmp[osize + 1];
	int isize = simple_write_to_buffer(tmp, osize, offset, user, amount);

	if (isize == osize && strncmp(output, user, osize) == 0)
		return strlen(output);
	return -EINVAL;
}


static const struct file_operations id_fops = {
	.owner			= THIS_MODULE,
	.write			= simple_write,
	.read			= simple_read
};

static const struct file_operations jiffie_fops = {
	.owner			= THIS_MODULE,
	.read			= jiffie_read
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
    start = get_jiffies_64();
    root = debugfs_create_dir("zuehlke", NULL);
    id = debugfs_create_file("id", 0666, root, NULL, &id_fops);
    jiffie = debugfs_create_file("jiffies", 0444, root, NULL, &jiffie_fops);
    pr_info("I'm in\n");
    return 0;
}

static void __exit misc_exit(void)
{
    misc_deregister(&misc_device);
    debugfs_remove_recursive(root);
    pr_info("I'm out\n");
}

module_init(misc_init);
module_exit(misc_exit);
