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

#include <linux/hrtimer.h>
#include <linux/sched/rt.h>
#include <linux/ktime.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/leds.h>

#include "../../inc/atom_light_out.h"
//#include "../atom_pwm/atom_pwm.h"

#define plog(x...) printk("lynn_pwm_led " x)

#if 0
#define US (1000)
#define MS (1000000)
//#define FIX_LOW_TIME 10*US 
//static long long ppm_values[2] = {FIX_LOW_TIME,90*US};

#define FIX_LOW_TIME 10*MS 
static long long ppm_values[2] = {FIX_LOW_TIME,90*MS};
static struct hrtimer work_timer;
//static struct work_struct workTimeOut;
#define  POLL_INTERVAL 10	//单位是ms
static bool timer_running = false;

void timer_start(void)
{
    ktime_t poll_delay;
    printk("%s\n", __func__);
    timer_running = true;

    poll_delay = ktime_set(0, POLL_INTERVAL * NSEC_PER_MSEC);
//    poll_delay = ktime_set(0,100*1000); //(s,ns)
    hrtimer_start(&work_timer, poll_delay, HRTIMER_MODE_REL);
}

void timer_stop(void)
{
    plog("%s\n", __func__);
    if(timer_running)
    {
        hrtimer_cancel(&work_timer);
        timer_running = false;
    }
}

static enum hrtimer_restart work_timeOutFunc(struct hrtimer *timer)
{
//	FL_Disable();
//	PK_DBG("ledTimeOut_callback\n");
    static int index = 0;  
    static ktime_t ktime;
//    ktime_t now = ktime_get();
//    plog("%s\n", __func__);
    if(2 == index)
        index = 0;
    ktime.tv64 = ppm_values[index];
    hrtimer_forward(timer, timer->base->get_time(), ktime);  
    index++; 
//    if(index >= 10)
//        index = 0;
    if(ktime.tv64 == FIX_LOW_TIME)
//    if(index <= 2)
        _atom_en_pin_output_low();
    else
        _atom_en_pin_output_high();
     
//    now = ktime_get();
//    msleep(50); /* CPU sleep */  can;  
//    timer_start(); 
 
//    return 0; // == HRTIMER_NORESTART
//    return HRTIMER_NORESTART; //定时器不重新启动,定时器回调函数只执行一次；
    return  HRTIMER_RESTART;
}

int __init dw_fp_key_sys_init(void)
{

    plog("%s\n", __func__);
    hrtimer_init(&work_timer, CLOCK_MONOTONIC,HRTIMER_MODE_REL);
    work_timer.function = &work_timeOutFunc;
    return 0;
}

#endif


//----------------
#if 1

//#define MS (1000000)
#define US (1000)
//#define FIX_LOW_TIME 10*MS 
//static long long ppm_values[2] = {FIX_LOW_TIME,90*MS};

static struct hrtimer work_timer;
static struct work_struct workTimeOut;
#define  POLL_INTERVAL 1	//单位是ms
static bool timer_running = false;

void timer_start(void)
{
    ktime_t poll_delay;
    printk("%s\n", __func__);
    timer_running = true;

    poll_delay = ktime_set(0, POLL_INTERVAL * US);
//    poll_delay = ktime_set(0,100*1000); //(s,ns)
    hrtimer_start(&work_timer, poll_delay, HRTIMER_MODE_REL);
}

void timer_stop(void)
{
    plog("%s\n", __func__);
    if(timer_running)
    {
        hrtimer_cancel(&work_timer);
        timer_running = false;
    }
}

static enum hrtimer_restart dw_fp_key_work(struct hrtimer *timer)
{
    schedule_work(&workTimeOut);
    return HRTIMER_NORESTART; //定时器必须重新启动 
//    HRTIMER_NORESTART  HRTIMER_RESTART //    使用该值就会卡死：（log：Unable to handle kernel paging request at virtual address ffffff7fffffdead）

}


static void work_timeOutFunc(struct work_struct *data)
{
//	PK_DBG("ledTimeOut_callback\n");
    static int index = 0;  
//    static ktime_t ktime;
//    ktime_t now = ktime_get();
//    plog("%s\n", __func__);
    if(10 == index)
        index = 0;
//    ktime.tv64 = ppm_values[index];
//    hrtimer_forward(&work_timer, now, ktime);  
    index++; 
//    if(index >= 10)
//        index = 0;
//    if(ktime.tv64 == FIX_LOW_TIME)
    if(index <= 8)
        _atom_en_pin_output_low();
    else
        _atom_en_pin_output_high();
     
    timer_start();   
//  hrtimer_start(&work_timer, poll_delay, HRTIMER_MODE_REL);

//    now = ktime_get();
//    msleep(50); /* CPU sleep */  can;  
    //    return 0;
}

int __init dw_fp_key_sys_init(void)
{
    plog("%s\n", __func__);
	INIT_WORK(&workTimeOut, work_timeOutFunc);
    hrtimer_init(&work_timer, CLOCK_MONOTONIC,HRTIMER_MODE_REL);
    work_timer.function = dw_fp_key_work;
    return 0;
}




#endif
