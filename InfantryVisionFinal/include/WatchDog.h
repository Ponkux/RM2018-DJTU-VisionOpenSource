#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>

class WatchDog
{
public:
    WatchDog(const char *path);
    void feed(void);
private:
    int fd;
    const char buf[3] = {'1','\r','\n'};
};

#endif // WATCHDOG_H
