
#define DDEBUG

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Petar Misic");
MODULE_DESCRIPTION("My first module :)");

static __init int hello_init(void) {
	pr_debug("Hello world!\n");
	return 0;
}

static void hello_exit(void) {
	pr_debug("Goodbye world!\n");
	return;
}

module_init(hello_init);
module_exit(hello_exit);