#ifndef SERIAL_H
#define SERIAL_H

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "include/Serial/CRC_Check.h"

#define TRUE 1
#define FALSE 0

#define BAUDRATE        115200
#define UART_DEVICE     "/dev/ttyUSB0"

#define ECHOFLAGS (ECHO | ECHOE | ECHOK | ECHONL)

typedef union{
    float f;
    unsigned char c[4];
}float2uchar;

typedef union{
    int16_t d;
    unsigned char c[2];
}int16uchar;

typedef struct{
    unsigned char mode; // 1 auto 2 sudoku
    int16uchar yaw_angle;
    int16uchar pitch_angle;
    int16uchar yaw_speed;
    float2uchar shoot_speed;
    uint8_t    which;
    uint8_t    pos;

    float2uchar x;
    float2uchar y;
    float2uchar z;
}VisionData;

typedef struct{
    int mode; // 0 auto_attack_red 1 auto_attack_blue 50 sudoku 51 sudokunew 52 bulletadd
    int level;//0 1 2 3
}CarData;

//void GetMode(int &fd,int &mode);
//void TransformData(int &fd,int number,int position);
void GetMode(int &fd,CarData &data);
int set_disp_mode(int fd,int option);
void TransformTarPos(int &fd,const VisionData &data);
void TransformData(int &fd,const VisionData &data);
void TransformSudoku(int &fd,const VisionData &data);
void TransformBulletNums(int &fd,int16_t number);
void InitSerial(int &fd);
void set_speed(int fd, int speed);
int set_Parity(int fd,int databits,int stopbits,int parity);

#endif // SERIAL_H
