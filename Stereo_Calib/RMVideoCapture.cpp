/*******************************************************************************************************************
Copyright 2017 Dajiang Innovations Technology Co., Ltd (DJI)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
documentation files(the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and 
to permit persons to whom the Software is furnished to do so, subject to the following conditions : 

The above copyright notice and this permission notice shall be included in all copies or substantial portions of
the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*******************************************************************************************************************/

#include "RMVideoCapture.hpp"
#include "linux/videodev2.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

RMVideoCapture::RMVideoCapture(const char * device, int size_buffer) : video_path(device) {
    fd = open(device, O_RDWR);
    buffer_size = size_buffer;
    buffr_idx = 0;
    cur_frame = 0;
    capture_width = 0;
    capture_height = 0;
    mb = new MapBuffer[buffer_size];
}

void RMVideoCapture::restartCapture(){
    close(fd);
    fd = open(video_path, O_RDWR);
    buffr_idx = 0;
    cur_frame = 0;
}

RMVideoCapture::~RMVideoCapture(){
    close(fd);
    delete [] mb;
}

void RMVideoCapture::cvtRaw2Mat(const void * data, cv::Mat & image){
    if (format == V4L2_PIX_FMT_MJPEG){
        cv::Mat src(capture_height, capture_width, CV_8UC3, (void*) data);
        image = cv::imdecode(src, 1);
    }
    else if(format == V4L2_PIX_FMT_YUYV){
        cv::Mat yuyv(capture_height, capture_width, CV_8UC2, (void*) data);
        cv::cvtColor(yuyv, image, CV_YUV2BGR_YUYV);
    }
}

RMVideoCapture & RMVideoCapture::operator >> (cv::Mat & image) {
//    std::cout << "current buffr idx: " << buffr_idx << std::endl;
    struct v4l2_buffer bufferinfo = {0};
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = buffr_idx;
    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
        perror("VIDIOC_DQBUF Error");
        exit(1);
    }

    //std::cout << "raw data size: " << bufferinfo.bytesused << std::endl;
    cvtRaw2Mat(mb[buffr_idx].ptr, image);

    //memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = buffr_idx;

    // Queue the next one.
    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
        perror("VIDIOC_DQBUF Error");
        exit(1);
    }
    ++buffr_idx;
    buffr_idx = buffr_idx >= buffer_size ? buffr_idx - buffer_size : buffr_idx;
    ++cur_frame;
    return *this;
}

bool RMVideoCapture::initMMap(){
    struct v4l2_requestbuffers bufrequest = {0};
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = buffer_size;

    if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
        perror("VIDIOC_REQBUFS");
        return false;
    }

    for(int i = 0; i < buffer_size; ++i){
        struct v4l2_buffer bufferinfo = {0};
        bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufferinfo.memory = V4L2_MEMORY_MMAP;
        bufferinfo.index = i; /* Queueing buffer index 0. */

        // Put the buffer in the incoming queue.
        if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
            perror("VIDIOC_QUERYBUF");
            return false;
        }

        mb[i].ptr = mmap(
            NULL,
            bufferinfo.length,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd,
            bufferinfo.m.offset);
        mb[i].size = bufferinfo.length;

        if(mb[i].ptr == MAP_FAILED){
            perror("MAP_FAILED");
            return false;
        }
        memset(mb[i].ptr, 0, bufferinfo.length);

        // Put the buffer in the incoming queue.
        memset(&bufferinfo, 0, sizeof(bufferinfo));
        bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufferinfo.memory = V4L2_MEMORY_MMAP;
        bufferinfo.index = i;
        if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
            perror("VIDIOC_QBUF");
            return false;
        }
    }
    return true;
}

bool RMVideoCapture::startStream(){
    cur_frame = 0;
    refreshVideoFormat();
    if(initMMap() == false)
        return false;

    __u32 type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        perror("VIDIOC_STREAMON");
        return false;
    }
    return true;
}

bool RMVideoCapture::closeStream(){
    cur_frame = 0;
    buffr_idx = 0;
    __u32 type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("VIDIOC_STREAMOFF");
        return false;
    }
    for(int i = 0; i < buffer_size; ++i){
        munmap(mb[i].ptr, mb[i].size);
    }
    return true;
}

bool RMVideoCapture::setExposureTime(bool auto_exp, int t){
    if (auto_exp){
        struct v4l2_control control_s;
        control_s.id = V4L2_CID_EXPOSURE_AUTO;
        control_s.value = V4L2_EXPOSURE_AUTO;
        if( xioctl(fd, VIDIOC_S_CTRL, &control_s) < 0){
            printf("Set Auto Exposure error\n");
            return false;
        }
    }
    else {
        struct v4l2_control control_s;
        control_s.id = V4L2_CID_EXPOSURE_AUTO;
        control_s.value = V4L2_EXPOSURE_MANUAL;
        if( xioctl(fd, VIDIOC_S_CTRL, &control_s) < 0){
            printf("Close MANUAL Exposure error\n");
            return false;
        }

        control_s.id = V4L2_CID_EXPOSURE_ABSOLUTE;
        control_s.value = t;
        if( xioctl(fd, VIDIOC_S_CTRL, &control_s) < 0){
            printf("Set Exposure Time error\n");
            return false;
        }
    }
    return true;
}

bool RMVideoCapture::setBlueBalance(int val){
    struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
    ctrl.value = V4L2_WHITE_BALANCE_MANUAL;
    int ret = xioctl(fd,VIDIOC_S_CTRL,&ctrl);
    if(ret < 0){
        printf("Error on read blue balance!\r\n");
        return false;
    }
    ctrl.id = V4L2_CID_GAMMA;
    ctrl.value = val;
    ret = xioctl(fd,VIDIOC_S_CTRL,&ctrl);
    if(ret < 0){
        printf("Error on set blue balance!\r\n");
        return false;
    }
    printf("blue balance val:%d\r\n",ctrl.value);
}

bool RMVideoCapture::changeVideoFormat(int width, int height, bool mjpg){
    closeStream();
    restartCapture();
    setVideoFormat(width, height, mjpg);
    startStream();
    return true;
}

bool RMVideoCapture::setVideoFormat(int width, int height, bool mjpg){     
    if (capture_width == width && capture_height == height)
        return true;
    capture_width = width;
    capture_height = height;
    cur_frame = 0;
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    if (mjpg == true)
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    else
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)){
        printf("Setting Pixel Format\n");
        return false;
    }
    return true;
}

bool RMVideoCapture::refreshVideoFormat(){
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt)) {
        perror("Querying Pixel Format\n");
        return false;
    }
    capture_width = fmt.fmt.pix.width;
    capture_height = fmt.fmt.pix.height;
    format = fmt.fmt.pix.pixelformat;
    return true;
}


bool RMVideoCapture::getVideoSize(int & width, int & height){
    if (capture_width == 0 || capture_height == 0){
        if (refreshVideoFormat() == false)
            return false;
    }
    width = capture_width;
    height = capture_height;
    return true;
}

bool RMVideoCapture::setVideoFPS(int fps){
    struct v4l2_streamparm stream_param = {0};
    stream_param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_param.parm.capture.timeperframe.denominator = fps;
    stream_param.parm.capture.timeperframe.numerator = 1;

    if (-1 == xioctl(fd, VIDIOC_S_PARM, &stream_param)){
        printf("Setting Frame Rate\n");
        return false;
    }
    return true;
}

bool RMVideoCapture::setBufferSize(int bsize){
    if (buffer_size != bsize){
        buffer_size = bsize;
        delete [] mb;
        mb = new MapBuffer[buffer_size];
    }
}

void RMVideoCapture::info(){
    struct v4l2_capability caps = {};
    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
            perror("Querying Capabilities\n");
            return;
    }

    printf( "Driver Caps:\n"
            "  Driver: \"%s\"\n"
            "  Card: \"%s\"\n"
            "  Bus: \"%s\"\n"
            "  Version: %d.%d\n"
            "  Capabilities: %08x\n",
            caps.driver,
            caps.card,
            caps.bus_info,
            (caps.version>>16)&&0xff,
            (caps.version>>24)&&0xff,
            caps.capabilities);
    camnum = caps.bus_info[17] -48;

    struct v4l2_cropcap cropcap = {0};
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl (fd, VIDIOC_CROPCAP, &cropcap))  {
            perror("Querying Cropping Capabilities\n");
            return;
    }

    printf( "Camera Cropping:\n"
            "  Bounds: %dx%d+%d+%d\n"
            "  Default: %dx%d+%d+%d\n"
            "  Aspect: %d/%d\n",
            cropcap.bounds.width, cropcap.bounds.height, cropcap.bounds.left, cropcap.bounds.top,
            cropcap.defrect.width, cropcap.defrect.height, cropcap.defrect.left, cropcap.defrect.top,
            cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

    struct v4l2_fmtdesc fmtdesc = {0};
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    char fourcc[5] = {0};
    char c, e;
    printf("  FMT : CE Desc\n--------------------\n");
    while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
            strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
            c = fmtdesc.flags & 1? 'C' : ' ';
            e = fmtdesc.flags & 2? 'E' : ' ';
            printf("  %s: %c%c %s\n", fourcc, c, e, fmtdesc.description);
            fmtdesc.index++;
    }

    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt)) {
        perror("Querying Pixel Format\n");
        return;
    }
    strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
    printf( "Selected Camera Mode:\n"
            "  Width: %d\n"
            "  Height: %d\n"
            "  PixFmt: %s\n"
            "  Field: %d\n",
            fmt.fmt.pix.width,
            fmt.fmt.pix.height,
            fourcc,
            fmt.fmt.pix.field);

    struct v4l2_streamparm streamparm = {0};
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_G_PARM, &streamparm)) {
        perror("Querying Frame Rate\n");
        return;
    }
    printf( "Frame Rate:  %f\n====================\n",
            (float)streamparm.parm.capture.timeperframe.denominator /
            (float)streamparm.parm.capture.timeperframe.numerator);
}

int RMVideoCapture::xioctl(int fd, int request, void *arg){
    int r;
    do r = ioctl (fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}




