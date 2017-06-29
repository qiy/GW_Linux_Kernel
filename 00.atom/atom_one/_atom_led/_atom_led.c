
#include <sys/types.h> 
#include <sys/stat.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <unistd.h> 
#include <string.h> 
#include <dirent.h> 
#include <string.h> 
#include <sys/mount.h> 
#include <sys/ioctl.h>

//#define CUST_REPLACE_HALL_WITH_PS

//#define ALSPS							0X84
//#define ALSPS_WRITE_CALIBRATE                   _IOW(ALSPS, 0x19, int) 

#define ATOM							0XAA
#define ATOM_WRITE_INT                   _IOW(ATOM, 0x10, int) 
#define ATOM_WRITE_OFF                   _IOW(ATOM, 0x11, int) 
#define ATOM_WRITE_ON                    _IOW(ATOM, 0x12, int) 

int main(int argc, char **argv) 
{ 
    int fd = -1;  
    int val = 0; 
    int err = 0;
	fd = open("/dev/kd_atom_led", O_RDWR);
	if (fd < 0) {
		//LOGD("Couldn't open");
		return -1;
	}
    err = ioctl(fd,ATOM_WRITE_ON,&val);
  
    if (err) {
			//FLPLOGE("read als raw: %d(%s)\n", errno, strerror(errno));
			return err;
	}
	close(fd);

exit(EXIT_SUCCESS); 
} 
