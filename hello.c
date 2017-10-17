
#define DDEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/jiffies.h>

u64 start;
u64 end;
struct timeval time;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Petar Misic");
MODULE_DESCRIPTION("My first module :)");

static __init int hello_init(void)
{
	start = get_jiffies_64();
	pr_debug("Hello world!\n");
	return 0;
}

static void hello_exit(void)
{
	end = get_jiffies_64();

	jiffies_to_timeval(end - start, &time);
	pr_debug("Goodbye world!\n");
	pr_debug("We were loaded for %ld seconds!\n", time.tv_sec);
}

module_init(hello_init);
module_exit(hello_exit);
