#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/gpio.h>
//#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/gameport.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>

#include <mach/gpio_const.h>
//#include <mach/mt_gpio.h>
#include <mt-plat/mt_gpio.h>
//#include "cust_gpio_usage.h"
//#include <cust_eint.h>
#include <mach/mt_gpt.h>
#include <linux/wakelock.h>
//#include <mach/eint.h>

#include<linux/interrupt.h>
#include<linux/irq.h>
#include<linux/of_irq.h>

#include <linux/fingerbutton.h>

#if defined(CONFIG_DEWAV_FINGERPRINT)
#include <linux/dw_fp_key_sys.h>
#endif
#define nlog(x...) printk("nicek_FB " x)

#include <mt-plat/mt_gpio.h>
#define BUTTON_PIN 67		//作为中断的GPIO号
#define EINT_MODE GPIO_MODE_06	//中断的模式

static struct device_node *node = NULL;  //找到匹配的irqchip node;
static int fingerbutton_irq = 0;
static struct work_struct irq_work;
static const struct of_device_id fingerbutton_of_match[] = {
	{.compatible = "mediatek,fingerbutton"},
	{},
};

static bool irq_enable = false;
void fingerbutton_enable_irq(void)
{
    nlog("%s\n", __func__);
    if(irq_enable == false)
    {
	enable_irq(fingerbutton_irq);
	irq_enable = true;
    }
}
void fingerbutton_disable_irq(void)
{
    nlog("%s\n", __func__);
    if(irq_enable)
    {
	disable_irq(fingerbutton_irq);
	irq_enable = false;
    }
}
void fingerbutton_disable_irq_nosync(void)
{
    nlog("%s\n", __func__);
    if(irq_enable)
    {
	disable_irq_nosync(fingerbutton_irq);//屏蔽fingerbutton_irq中断，等待中断处理函数结束；
	irq_enable = false;
    }
}

static irqreturn_t fingerbutton_irq_routing(int irq, void* dev)
{
    nlog("%s\n", __func__);
    fingerbutton_disable_irq_nosync();
//    disable_irq_nosync(fingerbutton_irq);
    schedule_work(&irq_work);
    return IRQ_HANDLED;
}

static void fingerbutton_irq_work(struct work_struct *work)
{
    nlog("%s\n", __func__);
    /***** 在这里上报按键按下 *****/
#if defined(CONFIG_DEWAV_FINGERPRINT)
    dwFPKeySys_button_down(1);
#endif
    
    msleep(50);	//去抖
    while(mt_get_gpio_in(BUTTON_PIN) == GPIO_IN_ZERO)	//等待按键释放
	msleep(20);
    /***** 在这里上报按键释放 *****/
#if defined(CONFIG_DEWAV_FINGERPRINT)
    dwFPKeySys_button_down(0);
#endif
    
    msleep(50);	//去抖
    fingerbutton_enable_irq();
}

static int fingerbutton_probe(struct platform_device *pdev)
{
    u32 ints[2] = {0, 0};
    int ret = 0;
    nlog("%s\n", __func__);
    mt_set_gpio_mode(BUTTON_PIN, EINT_MODE);
    mt_set_gpio_dir(BUTTON_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(BUTTON_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(BUTTON_PIN, GPIO_PULL_UP);

    node = of_find_matching_node(node, fingerbutton_of_match);//遍历Device Tree，找到匹配的irqchip
    of_property_read_u32_array(node, "debounce", ints, ARRAY_SIZE(ints));
   
    //读取设备结点node的属性名为debounce，类型为8、16、32、64位整型数组的属性。对于32位处理器来讲，最常用的是of_property_read_u32_array()。
    // 64:int of_property_read_u64(const struct device_node *np, const char *propname, u64 *out_value); 
    gpio_set_debounce(ints[0], ints[1]); //消除之前设置的输出状态对引脚电平的影响
    fingerbutton_irq = irq_of_parse_and_map(node, 0);//获取IRQ number
    nlog("fingerbutton_irq = %d\n", fingerbutton_irq);
    ret = request_irq(fingerbutton_irq,
		      (irq_handler_t)fingerbutton_irq_routing,
		      IRQF_TRIGGER_LOW,
		      "fingerbutton eint",
		      NULL);
    if(ret)
    {
	nlog("request_irq fail\n");
	return ret;
    }
    irq_enable = true;	//注册成功后默认中断是打开状态
//    fingerbutton_enable_irq();

    INIT_WORK(&irq_work, fingerbutton_irq_work);
    return 0;
}

static int fingerbutton_remove(struct platform_device *pdev)
{
    nlog("%s\n", __func__);
    return 0;
}

static int fingerbutton_resume(struct device *pdev)
{
    nlog("%s\n", __func__);
//    fingerbutton_disable_irq();
    return 0;
}

static int fingerbutton_suspend(struct device *pdev)
{
    nlog("%s\n", __func__);
//    fingerbutton_enable_irq();
    return 0;
}

const struct dev_pm_ops fingerbutton_pm_ops={
    .suspend = fingerbutton_suspend,
    .resume = fingerbutton_resume,
};

static struct platform_driver fingerbutton_driver={
    .remove = fingerbutton_remove,
    .probe = fingerbutton_probe,
    .shutdown = NULL,
    .driver = {
	.name = "fingerbutton",
	.pm = &fingerbutton_pm_ops,
	.owner = THIS_MODULE,
	.of_match_table = fingerbutton_of_match,
    },
};

static int __init fingerbutton_init(void)
{
    nlog("%s\n", __func__);
    if(platform_driver_register(&fingerbutton_driver))
    {
	nlog("platform_driver_register fail\n");
	return -1;
    }
    return 0;
}

static void __exit fingerbutton_exit(void)
{
    platform_driver_unregister(&fingerbutton_driver);
}

//module_init(fingerbutton_init);
late_initcall(fingerbutton_init);
module_exit(fingerbutton_exit);

MODULE_AUTHOR("nicek.ni@dewav.com");
MODULE_DESCRIPTION("fingerbutton driver");
MODULE_LICENSE("GPL");


