#include "include/Serial/serial.h"
#include "include/Serial/CRC_Check.h"
#include <opencv2/opencv.hpp>

unsigned char rdata[255];
unsigned char Tdata[30];
static double serialtimer,timerlast;

void TransformTarPos(int &fd,const VisionData &data){
    serialtimer = (double)cv::getTickCount();
    if((serialtimer - timerlast) / cv::getTickFrequency() > 1.0) return;
    Tdata[0] = 0xA5;

    Tdata[1] = data.x.c[0];
    Tdata[2] = data.x.c[1];
    Tdata[3] = data.x.c[2];
    Tdata[4] = data.x.c[3];

    Tdata[5] = data.y.c[0];
    Tdata[6] = data.y.c[1];
    Tdata[7] = data.y.c[2];
    Tdata[8] = data.y.c[3];

    Tdata[9] = data.z.c[0];
    Tdata[10] = data.z.c[1];
    Tdata[11] = data.z.c[2];
    Tdata[12] = data.z.c[3];
    printf("x:%f,y:%f,z:%f\r\n",data.x.f,data.y.f,data.z.f);
    Append_CRC16_Check_Sum(Tdata,19);
    write(fd,Tdata,19);
}

void TransformData(int &fd,const VisionData &data){
    //std::cout<<"mode:"<<data.mode<<"yaw:"<<data.yaw_angle.d<<" pitch:"<<data.pitch_angle.d<<std::endl;
    serialtimer = (double)cv::getTickCount();
    if((serialtimer - timerlast) / cv::getTickFrequency() > 1.0) return;
    Tdata[0] = 0xA5;

    Tdata[1] = data.mode;

    Tdata[2] = data.pitch_angle.c[0];
    Tdata[3] = data.pitch_angle.c[1];

    Tdata[4] = data.yaw_angle.c[0];
    Tdata[5] = data.yaw_angle.c[1];

    Tdata[6] = data.yaw_speed.c[0];
    Tdata[7] = data.yaw_speed.c[1];

    Tdata[8] = data.shoot_speed.c[0];
    Tdata[9] = data.shoot_speed.c[1];
    Tdata[10] = data.shoot_speed.c[2];
    Tdata[11] = data.shoot_speed.c[3];

    Tdata[12] = data.z.c[0];
    Tdata[13] = data.z.c[1];
    Tdata[14] = data.z.c[2];
    Tdata[15] = data.z.c[3];

    Append_CRC16_Check_Sum(Tdata,18);

    write(fd,Tdata,18);
}

void TransformSudoku(int &fd,const VisionData &data){
    serialtimer = (double)cv::getTickCount();
    if((serialtimer - timerlast) / cv::getTickFrequency() > 1.0) return;
    Tdata[0] = 0xC6;

    Tdata[1] = data.mode;

    Tdata[2] = data.pitch_angle.c[0];
    Tdata[3] = data.pitch_angle.c[1];

    Tdata[4] = data.yaw_angle.c[0];
    Tdata[5] = data.yaw_angle.c[1];

    Tdata[6] = 0;
    Tdata[7] = 0;

    Tdata[8] = data.which;
    Tdata[9] = data.pos;

    Tdata[10] = 0;
    Tdata[11] = 0;

    Tdata[12] = data.z.c[0];
    Tdata[13] = data.z.c[1];
    Tdata[14] = data.z.c[2];
    Tdata[15] = data.z.c[3];

    Append_CRC16_Check_Sum(Tdata,18);

    write(fd,Tdata,18);
}

void TransformBulletNums(int &fd,int16_t number){
    serialtimer = (double)cv::getTickCount();
    if((serialtimer - timerlast) / cv::getTickFrequency() > 1.0) return;
    Tdata[0] = 0x1B;
    Tdata[1] = 0;
    Tdata[2] = (uint8_t)(number>>8);
    Tdata[3] = (uint8_t)number;\
    Append_CRC16_Check_Sum(Tdata,18);
    write(fd,Tdata,18);
}

// 0xA5 |mode | CRC8| \r | \n |
void GetMode(int &fd,CarData &data){
    int bytes;
    ioctl(fd, FIONREAD, &bytes);
    if(bytes == 0) return;
    bytes = read(fd,rdata,6);
    timerlast = (double)cv::getTickCount();
    if(rdata[0] = 0xA5 && Verify_CRC8_Check_Sum(rdata,4)){
        data.mode  = (int)rdata[1];
        data.level = (int)rdata[2]-50;
        printf("receive mode:%d\r\n",data.mode);
    }
    ioctl(fd, FIONREAD, &bytes);
    if(bytes>0){
        read(fd,rdata,bytes);
    }
}

int set_disp_mode(int fd,int option)
{
   int err;
   struct termios term;
   if(tcgetattr(fd,&term)==-1){
     perror("Cannot get the attribution of the terminal");
     return 1;
   }
   if(option)
        term.c_lflag|=ECHOFLAGS;
   else
        term.c_lflag &=~ECHOFLAGS;
   err=tcsetattr(fd,TCSAFLUSH,&term);
   if(err==-1 && err==EINTR){
        perror("Cannot set the attribution of the terminal");
        return 1;
   }
   return 0;
}

void InitSerial(int &fd){
    fd = open(UART_DEVICE, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        perror(UART_DEVICE);
        exit(1);
    }
    printf("Open...\n");
    set_speed(fd,115200);
    if (set_Parity(fd,8,1,'N') == FALSE)  {
        printf("Set Parity Error\n");
        exit (0);
    }
    set_disp_mode(fd,0);
	printf("Open successed\n");
}



int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
    B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,
    115200, 38400, 19200, 9600, 4800, 2400, 1200,  300, };
void set_speed(int fd, int speed){
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd, &Opt);
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
        if  (speed == name_arr[i]) {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if  (status != 0) {
                perror("tcsetattr fd1");
                return;
            }
            tcflush(fd,TCIOFLUSH);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
/**
 *@brief   设置串口数据位，停止位和效验位
 *@param  fd     类型  int  打开的串口文件句柄
 *@param  databits 类型  int 数据位   取值 为 7 或者8
 *@param  stopbits 类型  int 停止位   取值为 1 或者2
 *@param  parity  类型  int  效验类型 取值为N,E,O,,S
 */
int set_Parity(int fd,int databits,int stopbits,int parity)
{
    struct termios options;
    if  ( tcgetattr( fd,&options)  !=  0) {
        perror("SetupSerial 1");
        return(FALSE);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits) /*设置数据位数*/
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr,"Unsupported data size\n"); return (FALSE);
    }
    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;break;
        default:
            fprintf(stderr,"Unsupported parity\n");
            return (FALSE);
    }
    /* 设置停止位*/
    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr,"Unsupported stop bits\n");
            return (FALSE);
    }
    /* Set input parity option */
    if (parity != 'n')
        options.c_iflag |= INPCK;
    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        perror("SetupSerial 3");
        return (FALSE);
    }
    //options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    options.c_lflag &= ~(ICANON);
    options.c_lflag &= ~(ICANON | ISIG);
    options.c_iflag &= ~(ICRNL | IGNCR);
    options.c_lflag &= ~(ECHO | ECHOE);
    options.c_oflag  &= ~OPOST;   /*Output*/
    if (tcsetattr(fd,TCSAFLUSH,&options) != 0)
    {
        perror("SetupSerial 3");
        return (FALSE);
    }
    return (TRUE);
}
////////////////////////////////////////////////////////////////////////////////

