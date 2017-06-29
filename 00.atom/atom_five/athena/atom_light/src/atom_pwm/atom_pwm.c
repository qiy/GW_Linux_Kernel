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
#include <linux/ktime.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/leds.h>

#include <mt-plat/mt_pwm.h>
#include <mt-plat/upmu_common.h>

//#include "../../inc/atom_light_out.h"
//den分母；mol分子
int open_pwm(int den,int mol)
{
	struct pwm_spec_config pwm_setting;
	
	pwm_setting.pwm_no =  PWM_MIN;   // PWM3; // PWM_MIN;
    pwm_setting.mode = PWM_MODE_OLD;

	printk("[lynn] mt_strobe_set_pwm enter  den=%d,mol=%d \n", den,mol);

	/* We won't choose 32K to be the clock src of old mode because of system performance. */
	/* The setting here will be clock src = 26MHz, CLKSEL = 26M/1625 (i.e. 16K) */
	pwm_setting.clk_src = PWM_CLK_OLD_MODE_BLOCK;

    pwm_setting.PWM_MODE_OLD_REGS.THRESH = mol;

	pwm_setting.clk_div = CLK_DIV128;	//8 == CLK_DIV128	
	pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = den; //Denominator
	
	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
	pwm_set_spec_config(&pwm_setting);

	return 0;
}

void close_pwm(void)
{
    mt_pwm_disable(PWM_MIN, 0); 
}


//EXPORT_SYMBOL(atom_data_VDIrq);
