#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/wait.h>
#include <opencv2/opencv.hpp>

int find_pid_by_name( char* ProcName, int* foundpid)
{
    DIR             *dir;
    struct dirent   *d;
    int             pid, i;
    char            *s;
    int pnlen;

    i = 0;
    foundpid[0] = 0;
    pnlen = strlen(ProcName);

    /* Open the /proc directory. */
    dir = opendir("/proc");
    if (!dir)
    {
        printf("cannot open /proc");
        return -1;
    }

    /* Walk through the directory. */
    while ((d = readdir(dir)) != NULL) {

        char exe [PATH_MAX+1];
        char path[PATH_MAX+1];
        int len;
        int namelen;

        /* See if this is a process */
        if ((pid = atoi(d->d_name)) == 0)       continue;

        snprintf(exe, sizeof(exe), "/proc/%s/exe", d->d_name);
        if ((len = readlink(exe, path, PATH_MAX)) < 0)
            continue;
        path[len] = '\0';

        /* Find ProcName */
        s = strrchr(path, '/');
        if(s == NULL) continue;
        s++;

        /* we don't need small name len */
        namelen = strlen(s);
        if(namelen < pnlen)     continue;

        if(!strncmp(ProcName, s, pnlen)) {
            /* to avoid subname like search proc tao but proc taolinke matched */
            if(s[pnlen] == ' ' || s[pnlen] == '\0') {
                foundpid[i] = pid;
                i++;
            }
        }
    }

    foundpid[i] = 0;
    closedir(dir);

    return  0;

}


using namespace std;

int main()
{
    int i, rv, pid_tt[128];
    mkfifo("/home/ubuntu/Desktop/InfantryVisionFinal/watchdog_fifo",0666);
    int fd = open("/home/ubuntu/Desktop/InfantryVisionFinal/watchdog_fifo",O_RDONLY);
    if(fd < 0)std::cout<<"err"<<std::endl;
    int bytes;
    char buf[255];
    double time,timelast;
    pid_t pid;
start_label:
    rv = find_pid_by_name("InfantryVisionFinal", pid_tt);

    if(!rv) {
        pid = pid_tt[0];
        while(pid == 0){
            rv = find_pid_by_name("InfantryVisionFinal", pid_tt);
            pid = pid_tt[0];
        }
    }
    printf("%d\r\n",pid_tt[0]);
    //pid_t pid = pid_tt[0];
    //std::cout<<"pid:"<<pid_tt[0]<<std::endl;

    time = (double)cv::getTickCount();
    timelast = time;
    while(1){
         time = (double)cv::getTickCount();
         if((time - timelast) /  cv::getTickFrequency() > 1.0){
             std::cout<<"kill process"<<std::endl;
             timelast = time;
             kill(pid,SIGKILL);
             goto start_label;
         }
         ioctl(fd, FIONREAD, &bytes);
         if(bytes < 0){
             lseek(fd,0,SEEK_SET);
             std::cout<<"restart success"<<std::endl;
             continue;
         }
         if(bytes == 0) continue;
         timelast = (double)cv::getTickCount();
         read(fd,buf,bytes);
         //std::cout<<"fresh"<<std::endl;
    }
    return 0;
}
