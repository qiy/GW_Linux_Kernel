
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
#define ATOM_BUF_SIZE 100
typedef struct
{
    int atom_flag;
    char atom_data_buf1[ATOM_BUF_SIZE];
}atom_data_buf;



#define ATOM							0XAA
#define ATOM_WRITE_INT                   _IOW(ATOM, 0x10, atom_data_buf) 
#define ATOM_WRITE_OFF                   _IOW(ATOM, 0x11, int) 
#define ATOM_WRITE_ON                    _IOW(ATOM, 0x12, int) 

atom_data_buf _atom_buf1;
int atom_flag_led;
int main(int argc, char **argv) 
{ 
    int fd = -1;  
    int val = 0; 
    int err = 0;

    char buf_flag[10] = {0};
    char m_flag[20] = "amtom_for_test"; 
    
    fd = open("/dev/kd_atom_led", O_RDWR);
	if (fd < 0) {
		//LOGD("Couldn't open");
		return -1;
	}

    atom_flag_led = 1;
    strcat(_atom_buf1.atom_data_buf1,m_flag);
    snprintf(_atom_buf1.atom_data_buf1 ,ATOM_BUF_SIZE-strlen(m_flag) , "atom_Led=%1d", atom_flag_led);

    err = ioctl(fd,ATOM_WRITE_INT,&_atom_buf1);
  
    if (err) {
			printf("read als one raw: %d(%s)\n", err, strerror(err));
//			return err;
	}

    printf("read one to two\n");
    err = ioctl(fd,ATOM_WRITE_ON,&atom_flag_led);
  
    if (err) {
			printf("read als two raw: %d(%s)\n", err, strerror(err));
//            return err;
	}

	close(fd);

exit(EXIT_SUCCESS); 
} 
