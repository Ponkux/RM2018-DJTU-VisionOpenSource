#include "include/WatchDog.h"

WatchDog::WatchDog(const char *path)
{
    mkfifo(path,0666);
    fd = open(path,O_WRONLY);
    ftruncate(fd,0);
    lseek(fd,0,SEEK_SET);
}

void WatchDog::feed(void){
    write(fd,buf,3);
}
