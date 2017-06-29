#ifndef _ATOM_LIGHT_H
#define _ATOM_LIGHT_H

typedef unsigned char MUINT8;
typedef unsigned short MUINT16;
typedef unsigned int MUINT32;
typedef signed char MINT8;
typedef signed short MINT16;
typedef signed int MINT32;

//buf 
#define ATOM_BUF_SIZE 100
typedef struct
{
    int atom_flag;
    char atom_data_buf1[ATOM_BUF_SIZE];
}atom_data_buf;
//end

#define ATOM                            0XAA
#define ATOM_WRITE_INT                   _IOW(ATOM, 0x10, atom_data_buf) 
#define ATOM_WRITE_OFF                   _IOW(ATOM, 0x11, int) 
#define ATOM_WRITE_ON                    _IOW(ATOM, 0x12, int)

typedef struct {
        int (*atom_open)(void *pArg);
        int (*atom_release)(void *pArg);
        int (*atom_ioctl)(unsigned int cmd, unsigned long arg);
} ATOM_FUNCTION_STRUCT, *P_ATOM_FUNCTION_STRUCT;

//MUINT32

MUINT32 constant_atom_Init(P_ATOM_FUNCTION_STRUCT *pfFunc);
extern void  _atom_en_pin_output_high(void);
extern void _atom_en_pin_output_low(void);


#endif
