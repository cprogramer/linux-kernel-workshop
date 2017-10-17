
#define DDEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/errno.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Petar Misic");
MODULE_DESCRIPTION("My first module :)");

char output[] = "ZuehlkeCamp2017\n";

static ssize_t simple_read(struct file *opened_file,
	char __user *user, size_t amount, loff_t *offset)
{
	return simple_read_from_buffer(user, amount, offset,
		output, sizeof(output));
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

static const struct file_operations my_fops = {
	.owner			= THIS_MODULE,
	.write			= simple_write,
	.read			= simple_read
};

static struct miscdevice misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "Zuehlke",
	.fops = &my_fops,
	.mode = 0666
};

module_misc_device(misc_device);
