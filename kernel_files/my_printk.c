#include <linux/linkage.h>
#include <linux/kernel.h>

asmlinkage void sys_my_printk(int pid,
	unsigned long s1, unsigned long s2,
	unsigned long e1, unsigned long e2){
	printk("[project1] %d %lu.%lu %lu.%lu\n", pid,
	       s1, s2, e1, e2);
}
