/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifdef WIN32
#include "win_test.h"
#include "stdio.h"
#include "kd_flashlight.h"
#else
#ifdef CONFIG_COMPAT

#include <linux/fs.h>
#include <linux/compat.h>

#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <asm/io.h>
#include <asm/uaccess.h>
//#include <mach/upmu_sw.h>
#endif
#ifdef CONFIG_COMPAT
#include <linux/fs.h>
#include <linux/compat.h>
#endif
//#include "kd_flashlight.h"
//#include <mach/mt_pbm.h>
#include "../inc/atom_light_out.h"


/******************************************************************************
 * Definition
******************************************************************************/

/* device name and major number */
#define ATOM_DEVNAME            "kd_atom_led"

/******************************************************************************
 * Debug configuration
******************************************************************************/
#ifdef WIN32
//#define logI(fmt, ...)    {printf(fmt, __VA_ARGS__); printf("\n"); }
#else
#define PFX "[KD_CAMERA_FLASHLIGHT]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    pr_debug(PFX "%s: " fmt, __func__ , ##arg)

/*#define DEBUG_KD_STROBE*/
#ifdef DEBUG_KD_STROBE
//#define logI PK_DBG_FUNC
#else
//#define logI(a, ...)
#endif
#endif

ATOM_FUNCTION_STRUCT *g_atom_InitFunc;

//  
struct platform_device *_atom_plt_dev = NULL;
struct pinctrl *_atom_ctrl = NULL;
struct pinctrl_state *_atom_en_h = NULL;
struct pinctrl_state *_atom_en_l = NULL;
//
struct pinctrl_state *_atom_pin_pwm2_out_pwm = NULL;

#ifdef CONFIG_OF
static const struct of_device_id _atom_of_ids[] = {
		{ .compatible = "mediatek,flashlight", },
		{}
};
#endif
// end 

/* ============================== */
/* functions */
/* ============================== */

// max add 
static int _atom_gpio_init(struct platform_device *pdev)
{
	int ret = 0;
	printk("[lynn--%s@%d]:   \n",__func__,__LINE__);

	_atom_ctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(_atom_ctrl)) {
		dev_err(&pdev->dev, "Cannot find flashlight pinctrl!");
		ret = PTR_ERR(_atom_ctrl);
	}

	_atom_en_h = pinctrl_lookup_state(_atom_ctrl, "flashlight_en1");
	if (IS_ERR(_atom_en_h)) {
		ret = PTR_ERR(_atom_en_h);
		pr_debug("%s : pinctrl err, _atom_en_h\n", __func__);
	}

	_atom_en_l = pinctrl_lookup_state(_atom_ctrl, "flashlight_en0");
	if (IS_ERR(_atom_en_l)) {
		ret = PTR_ERR(_atom_en_l);
		pr_debug("%s : pinctrl err, _atom_en_l\n", __func__);
	}

    _atom_pin_pwm2_out_pwm = pinctrl_lookup_state(_atom_ctrl, "pin_pwm2_out_pwm");
	if (IS_ERR(_atom_pin_pwm2_out_pwm)) {
		ret = PTR_ERR(_atom_pin_pwm2_out_pwm);
		pr_debug("%s : pinctrl err, _atom_pin_pwm2_out_pwm\n", __func__);
	}
	printk("[lynn--%s@%d]:  %d \n",__func__,__LINE__,ret);
	return ret;
}
//
void _atom_en_pin_output_low(void)
{
	printk("[max--%s@%d]: flash en pin low  \n",__func__,__LINE__);
	pinctrl_select_state(_atom_ctrl, _atom_en_l);
}

void _atom_en_pin_output_high(void)
{
	printk("[max--%s@%d]: flash en pin high  \n",__func__,__LINE__);
	pinctrl_select_state(_atom_ctrl, _atom_en_h);
}

//static void _atom_sub_led_pin_pwm2_out_pwm(void)
//{
//	printk("[lynn--%s@%d]: flash en sub pin high  \n",__func__,__LINE__);
//	pinctrl_select_state(_atom_ctrl, _atom_pin_pwm2_out_pwm);
//}

// end
static int globalInit(void)
{

	return 0;
}

static int checkAndRelease(void)
{
	return 0;
}

/* ======================================================================== */

static long _atom_ioctl_core(struct file *file, unsigned int cmd, unsigned long arg)
{

	int i4RetValue = 0;

    switch (cmd) {
	case ATOM_WRITE_INT:
		i4RetValue = 1;
		break;
	case ATOM_WRITE_OFF:
		printk("FLASH_IOC_IS_LOW_POWER");
         g_atom_InitFunc-> atom_ioctl(ATOM_WRITE_OFF,0);
//        _atom_en_pin_output_low();
		break;

    case ATOM_WRITE_ON:
        g_atom_InitFunc-> atom_ioctl(ATOM_WRITE_ON,0);
//        _atom_en_pin_output_high();
        break;
	default:
		break;
	}
	return i4RetValue;
}

static long _atom_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int err;
/* int dir; */
	err = _atom_ioctl_core(file, cmd, arg);
	/* dir  = _IOC_DIR(cmd); */
	/* if(dir &_IOC_READ) */
	{
		/* copy_to_user */
	}
	return err;
}

#ifdef CONFIG_COMPAT

static long my_ioctl_compat(struct file *filep, unsigned int cmd, unsigned long arg)
{
	int err;
	/* int copyRet; */



	/* data = compat_alloc_user_space(sizeof(*data)); */
	/* if (sys_data == NULL) */
	/* return -EFAULT; */
	/* err = compat_arg_struct_user32_to_kernel(data32, data); */
	/* arg2 = (unsigned long)data32; */
	err = _atom_ioctl_core(filep, cmd, arg);

	return err;

}
#endif


static int _atom_open(struct inode *inode, struct file *file)
{
	int i4RetValue = 0;
	static int bInited;

	if (bInited == 0) {
		globalInit();
		bInited = 1;
	}
	printk("[_atom_open] E ~");
	return i4RetValue;
}

static int _atom_release(struct inode *inode, struct file *file)
{
	printk("[_atom_release] E ~");

	checkAndRelease();

	return 0;
}


#ifdef WIN32
int fl_open(struct inode *inode, struct file *file)
{
	return _atom_open(inode, file);
}

int fl_release(struct inode *inode, struct file *file)
{
	return _atom_release(inode, file);
}

long fl_ioctrl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return _atom_ioctl(file, cmd, arg);
}

#else
/* ======================================================================== */
/* ======================================================================== */
/* ======================================================================== */
/* Kernel interface */
static const struct file_operations _atom_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = _atom_ioctl,
	.open = _atom_open,
	.release = _atom_release,
#ifdef CONFIG_COMPAT
	.compat_ioctl = my_ioctl_compat,
#endif
};

/* ======================================================================== */
/* Driver interface */
/* ======================================================================== */
struct _atom_data {
	spinlock_t lock;
	wait_queue_head_t read_wait;
	struct semaphore sem;
};
static struct class *_atom_class;
static struct device *_atom_device;
static struct _atom_data _atom_private;
static dev_t _atom_devno;
static struct cdev _atom_cdev;
/* ======================================================================== */
#define ALLOC_DEVNO
static int _atom_probe(struct platform_device *dev)
{
	int ret = 0, err = 0;
    ATOM_FUNCTION_STRUCT **g_atom_T;
    g_atom_T = &g_atom_InitFunc;
    printk("[_atom_probe]297 start ~");

#ifdef ALLOC_DEVNO
	ret = alloc_chrdev_region(&_atom_devno, 0, 1, ATOM_DEVNAME);
	if (ret) {
		printk("[_atom_probe]302 alloc_chrdev_region fail: %d ~", ret);
		goto _atom_probe_error;
	} else {
		printk("[_atom_probe]305 major: %d, minor: %d ~", MAJOR(_atom_devno),
		     MINOR(_atom_devno));
	}
	cdev_init(&_atom_cdev, &_atom_fops);
	_atom_cdev.owner = THIS_MODULE;
	err = cdev_add(&_atom_cdev, _atom_devno, 1);
	if (err) {
		printk("[_atom_probe]312 cdev_add fail: %d ~", err);
		goto _atom_probe_error;
	}
#else
#define ATOM_MAJOR 247
	ret = register_chrdev(ATOM_MAJOR, ATOM_DEVNAME, &_atom_fops);
	if (ret != 0) {
		printk("[flashlight_probe] Unable to register chardev on major=%d (%d) ~",
		     ATOM_MAJOR, ret);
		return ret;
	}
	_atom_devno = MKDEV(ATOM_MAJOR, 0);
#endif


	_atom_class = class_create(THIS_MODULE, ATOM_DEVNAME);
	if (IS_ERR(_atom_class)) {
		printk("[_atom_probe] Unable to create class, err = %d ~",
		     (int)PTR_ERR(_atom_class));
		goto _atom_probe_error;
	}

	_atom_device =
	    device_create(_atom_class, NULL, _atom_devno, NULL, ATOM_DEVNAME);
	if (NULL == _atom_device) {
		printk("[_atom_probe] device_create fail ~");
		goto _atom_probe_error;
	}

	/* initialize members */
	spin_lock_init(&_atom_private.lock);
	init_waitqueue_head(&_atom_private.read_wait);
	/* init_MUTEX(&flashlight_private.sem); */
	sema_init(&_atom_private.sem, 1);

	_atom_gpio_init(dev);
    constant_atom_Init(g_atom_T);
    if(0 != g_atom_InitFunc)
    {
        g_atom_InitFunc->atom_open(0);
    }
//    _atom_en_pin_output_high();
	printk("[flashlight_probe] Done ~");
	return 0;

_atom_probe_error:
#ifdef ALLOC_DEVNO
	if (err == 0)
		cdev_del(&_atom_cdev);
	if (ret == 0)
		unregister_chrdev_region(_atom_devno, 1);
#else
	if (ret == 0)
		unregister_chrdev(MAJOR(_atom_devno), ATOM_DEVNAME);
#endif
	return -1;
}

static int _atom_remove(struct platform_device *dev)
{

	printk("[_atom_probe] start\n");

#ifdef ALLOC_DEVNO
	cdev_del(&_atom_cdev);
	unregister_chrdev_region(_atom_devno, 1);
#else
	unregister_chrdev(MAJOR(_atom_devno), ATOM_DEVNAME);
#endif
	device_destroy(_atom_class, _atom_devno);
	class_destroy(_atom_class);

	printk("[_atom_probe] Done ~");
	return 0;
}

static void _atom_shutdown(struct platform_device *dev)
{

	printk("[_atom_shutdown] start\n");
	checkAndRelease();
	printk("[_atom_shutdown] Done ~");
}

static struct platform_driver _atom_platform_driver = {
	.probe = _atom_probe,
	.remove = _atom_remove,
	.shutdown = _atom_shutdown,
	.driver = {
		   .name = ATOM_DEVNAME,
		   .owner = THIS_MODULE,
#ifdef CONFIG_OF
	 	   .of_match_table = _atom_of_ids,
#endif
		   },
};

//static struct platform_device _atom_platform_device = {
//	.name = ATOM_DEVNAME,
//	.id = 0,
//	.dev = {
//		}
//};

static int __init _atom_init(void)
{
	int ret = 0;

	printk("[_atom_probe] start ~");

//	ret = platform_device_register(&_atom_platform_device);
//	if (ret) {
//		logI("[_atom_probe] platform_device_register fail ~");
//		return ret;
//	}

	ret = platform_driver_register(&_atom_platform_driver);
	if (ret) {
		printk("[_atom_probe] platform_driver_register fail ~");
		return ret;
	}
    printk("lynn427:ret:%d\n",ret);
	printk("[_atom_probe] done! ~");
	return ret;
}

static void __exit _atom_exit(void)
{
	printk("[flashlight_probe] start ~");
	platform_driver_unregister(&_atom_platform_driver);
	/* to flush work queue */
	/* flush_scheduled_work(); */
	printk("[_atom_probe] done! ~");
}

/* ======================================================== */
module_init(_atom_init);
module_exit(_atom_exit);

MODULE_LICENSE("GPL");
//MODULE_AUTHOR("Jackie Su <jackie.su@mediatek.com>");
//MODULE_DESCRIPTION("Flashlight control Driver");
MODULE_AUTHOR("Phone Feng <lynn.feng@dewav.com>");
MODULE_DESCRIPTION("atom control Driver");

#endif
