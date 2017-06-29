
在kernel-3.18/drivers/misc/mediatek/Makefile中添加：
obj-$(CONFIG_LCMINFO)»        += lcm_info/ 
在kernel-3.18/drivers/misc/mediatek/lcm_info/Kconfig中添加：
source "drivers/misc/mediatek/lcm_info/Kconfig"

1:dev
i6168:/dev # ls -l lcminfo                                                                                                                  
crw------- 1 root root 255,   0 2016-12-01 08:00 lcminfo

2:sys/class
1|i6168:/sys/class # ls -l lcminfo/                                                                                                         
total 0
lrwxrwxrwx 1 root root 0 2016-12-01 08:11 lcminfo -> ../../devices/virtual/lcminfo/lcminfo


