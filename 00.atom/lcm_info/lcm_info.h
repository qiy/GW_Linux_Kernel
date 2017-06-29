#ifndef _LCMINFO_H_
#define _LCMINFO_H_

#define LCMINFO_IOC_MAGIC  'Z'

#define LCMINFO_IOC_TEST   _IO(LCMINFO_IOC_MAGIC, 0)
#define LCMINFO_IOC_CLEAN  _IO(LCMINFO_IOC_MAGIC, 1)

struct lcminfo_dev {
    struct cdev cdev;
    char *data;
    unsigned int size;
    struct semaphore sem;
    spinlock_t lcminfo_lock;
    unsigned int lcminfo_u_owner;
    unsigned int lcminfo_u_count;
    struct proc_dir_entry *lcmlist_using_entry;
    struct proc_dir_entry *lcmlist_supported_entry;
};
#endif 
