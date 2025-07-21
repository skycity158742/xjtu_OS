#include <linux/init.h>        // 包含模块初始化和清理函数的定义
#include <linux/module.h>      // 包含加载模块时需要的函数和符号定义
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/init_task.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/kdev_t.h>

MODULE_LICENSE("GPL");

#define MAJOR_NUM 400          // 主设备号
#define MAXNUM 100             // 缓冲区大小

static ssize_t globalvar_read(struct file *, char *, size_t, loff_t*);
static ssize_t globalvar_write(struct file *, const char *, size_t, loff_t*);

// 文件操作结构体
struct file_operations globalvar_fops = {
    .read = globalvar_read,
    .write = globalvar_write,
};

// 自定义设备结构体
struct Scull_Dev {
    struct cdev devm;               // 字符设备
    struct semaphore sem;           // 信号量
    wait_queue_head_t outq;         // 等待队列
    int flag;                       // 阻塞唤醒标志
    char *read, *write, *end;       // 读，写，尾指针
    char buffer[MAXNUM + 1];        // 字符缓冲区
};

struct Scull_Dev globalvar;
static struct class *my_class;
int major = MAJOR_NUM;

// 模块初始化
static int init_mymodule(void) {
    int result = 0;
    int err = 0;
    dev_t dev = MKDEV(major, 0);    // 创建设备编号

    if (major) {
        result = register_chrdev_region(dev, 1, "ch_device");
    } else {
        result = alloc_chrdev_region(&dev, 0, 1, "ch_device");
        major = MAJOR(dev);
    }
    if (result < 0) return result;

    // 初始化字符设备
    cdev_init(&globalvar.devm, &globalvar_fops);
    globalvar.devm.owner = THIS_MODULE;
    err = cdev_add(&globalvar.devm, dev, 1);
    if (err) {
        printk("Error adding cdev\n");
        return err;
    } else {
        printk("ch_device register success\n");
        sema_init(&globalvar.sem, 1);            // 初始化信号量
        init_waitqueue_head(&globalvar.outq);   // 初始化等待队列
        globalvar.read = globalvar.buffer;
        globalvar.write = globalvar.buffer;
        globalvar.end = globalvar.buffer + MAXNUM;
        globalvar.flag = 0;                     // 阻塞唤醒标志置 0
    }

    // 创建设备文件 `/dev/ch_device`
    my_class = class_create(THIS_MODULE, "ch_device");
    device_create(my_class, NULL, dev, NULL, "ch_device");

    return 0;
}

// 模块卸载
static void cleanup_mymodule(void) {
    device_destroy(my_class, MKDEV(major, 0));
    class_destroy(my_class);
    cdev_del(&globalvar.devm);
    unregister_chrdev_region(MKDEV(major, 0), 1);
}

// 读操作实现
static ssize_t globalvar_read(struct file *filp, char *buf, size_t len, loff_t *off) {
    if (wait_event_interruptible(globalvar.outq, globalvar.flag != 0)) {
    	

        return -ERESTARTSYS;
    }

    if (down_interruptible(&globalvar.sem)) {
        return -ERESTARTSYS;  // 获取信号量失败
    }

    globalvar.flag = 0;
    printk("read!\n");
    printk("the read is %c\n",*globalvar.read);

    if (globalvar.read < globalvar.write) {
        len = min(len, (size_t)(globalvar.write - globalvar.read));
    } else {
        len = min(len, (size_t)(globalvar.end - globalvar.read));
    }
	printk("the len is %zu\n",len);
    if (copy_to_user(buf, globalvar.read, len)) {
    	printk("fail!\n");
        up(&globalvar.sem);
        return -EFAULT;
    }
	printk("read success!\n");
    globalvar.read += len;
    if (globalvar.read == globalvar.end) globalvar.read = globalvar.buffer;

    up(&globalvar.sem);
    return len;
}

// 写操作实现
static ssize_t globalvar_write(struct file *filp, const char *buf, size_t len, loff_t *off) {
    if (down_interruptible(&globalvar.sem)) {
        return -ERESTARTSYS;
    }

    if (globalvar.read <= globalvar.write) {
        len = min(len, (size_t)(globalvar.end - globalvar.write));
    } else {
        len = min(len, (size_t)(globalvar.read - globalvar.write - 1));
    }

    if (copy_from_user(globalvar.write, buf, len)) {
        up(&globalvar.sem);
        return -EFAULT;
    }
	printk("write success!\n");
    globalvar.write += len;
    if (globalvar.write == globalvar.end) globalvar.write = globalvar.buffer;

    up(&globalvar.sem);
    globalvar.flag = 1;
    wake_up_interruptible(&globalvar.outq);  // 唤醒阻塞的读进程
    return len;
}

module_init(init_mymodule);
module_exit(cleanup_mymodule);
