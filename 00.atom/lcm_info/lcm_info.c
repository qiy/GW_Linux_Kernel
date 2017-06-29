/**
  *---------------------------------------------------------------------------------
  * @file    rfidReader.c
  * @author  phone
  * @version V1.0
  * @date    2015-07-25
  * @brief   RC522内核驱动标准字符设备驱动函数
  *---------------------------------------------------------------------------------
  * @attention
  *
  * @{实验平台:tiny4412开发板
  * @{雅典娜程序整理	
  * @{源码：PHone
  *---------------------------------------------------------------------------------
  */ 


/*---------------------------------------------------------------------------------
首先是包含必要的头文件
---------------------------------------------------------------------------------*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>

#include "lcm_info.h"
#include "../lcm/inc/lcm_drv.h"

#define DRIVER_VERSION  "0.1 2016-11-08"
#define DEVICE_NAME     "lcminfo"
#define LCMINFO_MAJOR    255
#define LCMINFO_SIZE     4096

extern unsigned int lcm_count;
extern LCM_DRIVER *lcm_driver_list[];
extern char *mtkfb_find_lcm_driver(void);

static int lcminfo_major = LCMINFO_MAJOR;
static struct class *lcminfo_class;

struct lcminfo_dev *lcminfo_devp;

static ssize_t lcmlist_using_proc_read(struct file *file, char __user *page, size_t size, loff_t *ppos)
{
#define LCM_USING_LENGTH    256
    char *p = page;
    char lcmlist_using[LCM_USING_LENGTH] = {0};

    if (*ppos)
        return 0;

    strcat(lcmlist_using, mtkfb_find_lcm_driver());
  
    p += sprintf(p, "%s\n", lcmlist_using);

    *ppos += p - page;
    return (p - page);
}

static ssize_t lcmlist_supported_proc_read(struct file *file, char __user *page, size_t size, loff_t *ppos)
{
#define LCM_SUPPORT_LENGTH  512
    int i;
    char lcmlist_supported[LCM_SUPPORT_LENGTH] = {0};
    char *p = page;

    if (*ppos) //command call again.
        return 0;

    for (i = 0; i < lcm_count; i++) {
        if (lcm_driver_list[i]->name == NULL)
            break;
        strcat(lcmlist_supported, lcm_driver_list[i]->name);
        strcat(lcmlist_supported, "  ");
    }

    strcat(lcmlist_supported, "\0");
    p += sprintf(p, "%s\n", lcmlist_supported);

    *ppos += p - page;

    return (p - page);
}

static const struct file_operations lcmlist_using_proc_ops = {
    .owner = THIS_MODULE,
    .read = lcmlist_using_proc_read,
};

static const struct file_operations lcmlist_supported_proc_ops = {
    .owner = THIS_MODULE,
    .read = lcmlist_supported_proc_read,
};

static int lcminfo_open(struct inode *inode, struct file *filp)
{
    struct lcminfo_dev *devp;
    devp = container_of(inode->i_cdev, struct lcminfo_dev, cdev);

    spin_lock(&devp->lcminfo_lock);
    /*
    if (devp->lcminfo_u_count && 
            (devp->lcminfo_u_owner != current->cred->uid) &&
            (devp->lcminfo_u_owner != current->cred->euid) &&
            !capable(CAP_DAC_OVERRIDE)) {
        spin_unlock(&devp->lcminfo_lock);
        return -EBUSY;
    }
    */
    if (devp->lcminfo_u_count == 0)
        devp->lcminfo_u_owner = current->cred->uid.val;
    devp->lcminfo_u_count++;
    spin_unlock(&devp->lcminfo_lock);

    filp->private_data = devp;
    printk(KERN_INFO "call lcminfo_open.\n");
    return 0;
}

static int lcminfo_release(struct inode *inode, struct file *filp)
{
    struct lcminfo_dev *devp;
    devp = filp->private_data;
    spin_lock(&devp->lcminfo_lock);
    devp->lcminfo_u_count--;
    spin_unlock(&devp->lcminfo_lock);

    printk(KERN_INFO "call lcminfo_release.\n");
    return 0;
}

static ssize_t lcminfo_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;

    struct lcminfo_dev *devp = filp->private_data;

    if (down_interruptible(&devp->sem)) 
        return -ERESTARTSYS;

    printk(KERN_INFO "%s start.\n", __func__);
    if (p > LCMINFO_SIZE)
        return 0;
    if (count > LCMINFO_SIZE - p)
        count = LCMINFO_SIZE - p;

    if (copy_to_user(buf, devp->data + p, count)) {
        ret = -EFAULT;
    } else {
        *ppos += count;
        ret = count;
    }

    up(&devp->sem);
    printk(KERN_INFO "%s return %d\n", __func__, ret);
    return ret;
}

static ssize_t lcminfo_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;

    struct lcminfo_dev *devp = filp->private_data;

    if (down_interruptible(&devp->sem)) 
        return -ERESTARTSYS;

    printk(KERN_INFO "%s start.\n", __func__);
    if (p >= LCMINFO_SIZE)
        return 0;
    if (count > LCMINFO_SIZE - p)
        count = LCMINFO_SIZE - p;

    if (copy_from_user(devp->data + p, buf, count)) {
        ret = -EFAULT;
    } else {
        *ppos += count;
        ret = count;
    }

    up(&devp->sem);
    printk(KERN_INFO "%s return %d\n", __func__, ret);
    return ret;
}

static loff_t lcminfo_llseek(struct file *filp, loff_t offset, int whence)
{
    loff_t newpos;

    switch (whence) {
    case SEEK_SET: // 0
        newpos = offset;
        break;

    case SEEK_CUR: // 1
        newpos = filp->f_pos + offset;
        break;

    case SEEK_END: // 2
        newpos = LCMINFO_SIZE - 1 + offset;
        break;

    default:
        return -EINVAL;
    }

    if (newpos < 0 || newpos > LCMINFO_SIZE) 
        return -EINVAL;

    filp->f_pos = newpos;
    return newpos;
}

static long lcminfo_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct lcminfo_dev *devp;
    int result = 0;

    devp = filp->private_data;

    if (_IOC_TYPE(cmd) != LCMINFO_IOC_MAGIC)
        return -ENOTTY;

    switch (cmd) {
    case LCMINFO_IOC_TEST:
        printk(KERN_INFO "lcminfo Driver Version: %s\n", DRIVER_VERSION);
        break;

    case LCMINFO_IOC_CLEAN:
        printk(KERN_INFO "lcminfo clean data.\n");
        memset(devp->data, 0, devp->size);
        break;

    default:
        return -EINVAL;
    }
    return result;
}

static const struct file_operations lcminfo_fops = 
{
    .owner = THIS_MODULE,
    .open = lcminfo_open,
    .release = lcminfo_release,
    .read = lcminfo_read, 
    .write = lcminfo_write,
    .llseek = lcminfo_llseek,
    .unlocked_ioctl = lcminfo_unlocked_ioctl,
};

static int lcminfo_setup_cdev(struct lcminfo_dev *dev, int index)
{
    int err, devno = MKDEV(lcminfo_major, index);

    if (!lcminfo_major)//动态内存定义初始化
    {
//        dev->cdev = cdev_alloc();    
        dev->cdev.ops = &lcminfo_fops;
    } 
    else 
        cdev_init(&dev->cdev, &lcminfo_fops);//静态内存定义初始化
    dev->cdev.owner = THIS_MODULE;

    err = cdev_add(&dev->cdev, devno, 1);//核心:注册字符设备，初始化后将它添加到系统中
    if (err)
        return err;
  

    return 0;
}

static int __init lcminfo_init(void)
{
    int result;
    dev_t devno = 0;
    struct device *class_dev;
/*---------/ ① 设备号注册 /---------*/
    /* 普通注册 */
    if (lcminfo_major) 
    {
        devno = MKDEV(lcminfo_major, 0);
        result = register_chrdev_region(devno, 1, DEVICE_NAME);
    } 
    /* 动态注册 */
    else 
    {
        result = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
        lcminfo_major = MAJOR(devno);
    }
    if (result < 0) 
    {
        printk(KERN_WARNING "Error: failed in getting major %d.\n", lcminfo_major);
        goto fail_major;
    }
/*---------/ ① end /---------*/
/* → */
/*---------/ ② 设备内存分配 /---------*/
    lcminfo_devp = kmalloc(sizeof(struct lcminfo_dev), GFP_KERNEL);//参数1大小，类型为：GFP_KERNEL
    if (!lcminfo_devp) {
        result = -ENOMEM;
        printk(KERN_WARNING "Error: failed in kmalloc. Err=%d\n", result);
        goto unregister;
    }
    memset(lcminfo_devp, 0, sizeof(struct lcminfo_dev));//该空间清零

    lcminfo_devp->size = LCMINFO_SIZE;
    lcminfo_devp->data = kmalloc(LCMINFO_SIZE, GFP_KERNEL);
    if (!lcminfo_devp->data) {
        result = -ENOMEM;
        printk(KERN_WARNING "Error: failed in kmalloc. Err=%d\n", result);
        goto cleanup;
    }
    memset(lcminfo_devp->data, 0, LCMINFO_SIZE);
/*---------/ ② end /---------*/
/* → */
/*---------/ ③ cdev内存动静态定义及注册 /---------*/

    result = lcminfo_setup_cdev(lcminfo_devp, 0);
    if (result) {
        printk(KERN_WARNING "Error: failed in adding lcminfo cdev. Err=%d\n", result);
//        goto fail_cdev;
          goto cleanup;
    }
/*---------/ ③ end /---------*/
/* → */
/*---------/ ④ 信号量，互斥锁相关（不建议同时用，建议单用信号量） /---------*/
    sema_init(&lcminfo_devp->sem, 1);  
#if 0
    /* 2.6.25之前对信号量的用法 */
    init_MUTEX(&(dev->sem));
#endif
    spin_lock_init(&lcminfo_devp->lcminfo_lock);//自旋锁 锁初始化
/*---------/ ④ end /---------*/

/*---------/ ⑤ /sys/class/下节点属性 /---------*/
    /* 这个函数用来创建一个struct class的结构体指针，这个指针可用作device_create()函数的参数 */
    /* 在/sys/class/目录下创建设备类别目录xxx */
    lcminfo_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(lcminfo_class)) {
        result = PTR_ERR(lcminfo_class);
        printk(KERN_WARNING "Error: failed in creating class. Err=%d\n", result);
        goto fail_class;
    }
    /* 在/dev/目录和/sys/class/xxx目录下分别创建设备文件xxx */ 
    class_dev = device_create(lcminfo_class, NULL, MKDEV(lcminfo_major, 0), NULL, "%s", DEVICE_NAME);
    if (IS_ERR(class_dev)) {
        result = PTR_ERR(class_dev);
        printk(KERN_WARNING "Error: failed in creating class device. Err=%d\n", result);
        goto fail_device;
    }
    /* 在/sys/class/xxx/xxx目录下创建属性文件val */
//    result = device_create_file(class_dev,attr);
//    if(result){
//        printk(KERN_WARNING"Failed to create attribute val.");
//        goto device_create_file_fail; 
//    }
/*---------/ ⑤ end /---------*/
    //create lcmlist_supported 
    lcminfo_devp->lcmlist_using_entry = 
        proc_create("lcmlist_using", 0, NULL, &lcmlist_using_proc_ops);

    //create lcmlist_using
    lcminfo_devp->lcmlist_supported_entry = 
        proc_create("lcmlist_supported", 0, NULL, &lcmlist_supported_proc_ops);

    if (NULL == lcminfo_devp->lcmlist_using_entry && NULL == lcminfo_devp->lcmlist_supported_entry)
        printk(KERN_WARNING "Error: failed to create lcm info proc entry.\n");

    printk(KERN_INFO "Hello, world!\n");
    return 0;

fail_class:
    cdev_del(&lcminfo_devp->cdev);
fail_device:
    class_destroy(lcminfo_class);
//device_create_file_fail:
//    device_destroy(lcminfo_class,dev);
//fail_cdev:
//    kfree(lcminfo_devp->data);
unregister:
    unregister_chrdev_region(devno, 1);
cleanup:
    kfree(lcminfo_devp);
fail_major:
    return result;
}

static void __exit lcminfo_exit(void)
{
    dev_t devno = MKDEV(lcminfo_major, 0);

/*---------/ ① 删除/proc/,,,文件 /---------*/
//    remove_proc_entry(HELLO_DEVICE_PROC_NAME, NULL); //old kernel ;
    proc_remove(lcminfo_devp->lcmlist_using_entry);
    proc_remove(lcminfo_devp->lcmlist_supported_entry);
/*---------/ ② 销毁设备类别和设备 /---------*/
    if(lcminfo_class)
    {
        device_destroy(lcminfo_class, devno);
        class_destroy(lcminfo_class);
    }
/*---------/ ③ 删除字符设备和释放设备内存 /---------*/
    if(lcminfo_devp)
    {
        cdev_del(&lcminfo_devp->cdev);
        kfree(lcminfo_devp->data);
        kfree(lcminfo_devp);
    }
/* ④ 释放设备号 */
    unregister_chrdev_region(MKDEV(lcminfo_major, 0), 1);

    printk(KERN_INFO "Goodbye, cruel world.\n");
}

module_init(lcminfo_init);
module_exit(lcminfo_exit);

MODULE_AUTHOR("nikos.ning@dewav.com");
MODULE_LICENSE("Dual BSD/GPL");



/*-------------------------------------end--------------------------------------------*/


/*---------------------------------------------------------------------------------
 最后，定义模块加载和卸载方法，这里只要是执行设备注册和初始化操作：
---------------------------------------------------------------------------------*/

