#include <iostream>
#include "StereoCalib.h"
#include "RMVideoCapture.hpp"
#include "Header.h"

int main()
{
    int exp = 1000,explast = 1000;
    RMVideoCapture cap0("/dev/video0",1);
    cap0.setVideoFormat(1280,720,1);
    cap0.info();
    cap0.setVideoFPS(30);
    cap0.setExposureTime(0,exp);
    cap0.startStream();
    RMVideoCapture cap1("/dev/video1",1);
    cap1.setVideoFormat(1280,720,1);
    cap1.info();
    cap1.setVideoFPS(30);
    cap1.setExposureTime(0,exp);
    cap1.startStream();
    Mat left,right;
    int key;
    StereoCalib calib("/home/ubuntu/Desktop/Camera_xmls/"); // Path for the calibrated file.
    namedWindow("left");
    namedWindow("right");
    namedWindow("adjust");
    createTrackbar("exp time","adjust",&exp,1500);
    while(1){
        key = waitKey(1);
        if(exp != explast){
            explast = exp;
            cap0.setExposureTime(0,exp);
            cap1.setExposureTime(0,exp);
        }
        if(cap0.camnum == 4){
            cap0>>right;
            cap1>>left;
        }else{
            cap0>>left;
            cap1>>right;
        }
        blur(left,left,Size(5,5));
        blur(right,right,Size(5,5));
        calib.Process(left, right);
        calib.KeyParam(key);
        resize(left,left,Size(640,360));
        imshow("left", left);
        //imshow("lbianry",calib.thresleft_cp);
        resize(right,right,Size(640,360));
        imshow("right", right);
        //imshow("rbinary",calib.thresright_cp);
        if(key == 'q') break;
    }
    cap0.closeStream();
    cap1.closeStream();
}
