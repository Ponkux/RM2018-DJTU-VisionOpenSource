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

#pragma once
#include "opencv2/core.hpp"

class RMVideoCapture {
public:
    RMVideoCapture(const char * device, int size_buffer = 1);
    ~RMVideoCapture();
    bool startStream();
    bool closeStream();
    bool setBlueBalance(int val);
    bool setExposureTime(bool auto_exp, int t);
    bool setVideoFormat(int width, int height, bool mjpg = 1);
    bool changeVideoFormat(int width, int height, bool mjpg = 1);
    bool getVideoSize(int & width, int & height);

    bool setVideoFPS(int fps);
    bool setBufferSize(int bsize);
    void restartCapture();
    int getFrameCount(){
        return cur_frame;
    }

    void info();

    RMVideoCapture& operator >> (cv::Mat & image);
public:
    int camnum;
private:
    void cvtRaw2Mat(const void * data, cv::Mat & image);
    bool refreshVideoFormat();
    bool initMMap();
    int xioctl(int fd, int request, void *arg);

private:
    struct MapBuffer {
        void * ptr;
        unsigned int size;
    };
    unsigned int capture_width;
    unsigned int capture_height;
    unsigned int format;
    int fd;
    unsigned int buffer_size;
    unsigned int buffr_idx;
    unsigned int cur_frame;
    MapBuffer * mb;
    const char * video_path;
};

