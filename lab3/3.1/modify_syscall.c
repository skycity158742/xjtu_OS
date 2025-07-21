#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include <linux/version.h>

//original,syscall 96 function: gettimeofday
// new syscall 96 function: print "No 96 syscall has changed to hello"and return a+b

#define sys_No 96
unsigned long old_sys_call_func;
unsigned long p_sys_call_table; // find in /boot/System.map-'uname -r'
unsigned long orig_cr0;




// *************************************************************************************************************

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0) || LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)


static unsigned long (*kallsyms_lookup_name_sym)(const char *name);

static int _kallsyms_lookup_kprobe(struct kprobe *p, struct pt_regs *regs)
{
        return 0;
}

unsigned long get_kallsyms_func(void)
{
        struct kprobe probe;
        int ret;
        unsigned long addr;

        memset(&probe, 0, sizeof(probe));
        probe.pre_handler = _kallsyms_lookup_kprobe;
        probe.symbol_name = "kallsyms_lookup_name";
        ret = register_kprobe(&probe);
        if (ret)
                return 0;
        addr = (unsigned long)probe.addr;
        unregister_kprobe(&probe);
        return addr;
}

unsigned long generic_kallsyms_lookup_name(const char *name)
{
        /* singleton */
        if (!kallsyms_lookup_name_sym) {
                kallsyms_lookup_name_sym = (void *)get_kallsyms_func();
                if(!kallsyms_lookup_name_sym)
                        return 0;
        }
        return kallsyms_lookup_name_sym(name);
}

#else

unsigned long generic_kallsyms_lookup_name(const char *name)
{
    return kallsyms_lookup_name(name);
}

#endif

// ****************************************************************************************

unsigned int clear_cr0(void)        
{
        unsigned int cr0 = 0;
        unsigned int ret;
        //move the value in reg cr0 to reg rax
        //movl moves a 32-bits operand
        //movq moves a 64-bits operand
        //rax is a 64-bits register
        //an assembly language code
        //asm volatile ("movl %%cr0, %%eax" : "=a"(cr0));//32-bits        
        asm volatile ("movq %%cr0, %%rax" : "=a"(cr0));        //64-bits
        ret = cr0;
        //var cr0 is rax        
        cr0 &= 0xfffeffff; //set 0 to the 17th bit
        //asm volatile ("movl %%eax, %%cr0" :: "a"(cr0));//32-bits
        //note that cr0 above is a variable while cr0 below is a reg.        
        asm volatile ("movq %%rax, %%cr0" :: "a"(cr0));        
        return ret;
}

//recover the value of WP 
void setback_cr0(unsigned int val)
{        
        //asm volatile ("movl %%eax, %%cr0" :: "a"(val));//32-bits
        asm volatile ("movq %%rax, %%cr0" :: "a"(val));//64-bits
}

int hello(const struct pt_regs *regs) //new function
{
    printk(KERN_ALERT"No.96 syscall has changed to function hello\n");
    int a = 0;
    int b = 0;
    a = regs->di;
    b = regs->si;
    return a+b;
}

void modify_syscall(void)
{
	unsigned long *sys_call_addr;
	p_sys_call_table = (unsigned long )generic_kallsyms_lookup_name("sys_call_table");
	if (!p_sys_call_table) {
        pr_err("Failed to find sys_call_table address\n");
        return;
    	}
        
    sys_call_addr = (unsigned long *)(p_sys_call_table + sys_No * sizeof(void *));
    old_sys_call_func = *(sys_call_addr);
	orig_cr0 = clear_cr0(); 
    *(sys_call_addr) = (unsigned long)&hello; // point to new function
    setback_cr0(orig_cr0);
}

void restore_syscall(void)
{
    unsigned long *sys_call_addr;

    sys_call_addr = (unsigned long *)(p_sys_call_table + sys_No * sizeof(void *));
	orig_cr0 = clear_cr0();
    *(sys_call_addr) = old_sys_call_func; // point to original function
setback_cr0(orig_cr0);
}

static int mymodule_init(void)
{
    modify_syscall();
    return 0;
}

static void mymodule_exit(void)
{
    restore_syscall();
}



module_init(mymodule_init);
module_exit(mymodule_exit);
MODULE_LICENSE("GPL");