//
//  StereoCalib.h
//  stereo_calibrate_toe
//
//  Created by ding on 17/8/17.
//  Copyright (c) 2017年 ding. All rights reserved.
//

#ifndef __stereo_calibrate_toe__StereoCalib__
#define __stereo_calibrate_toe__StereoCalib__

#include "Header.h"

typedef struct{
    int height;
    int width;
    float distance_const;
    float f;
    //float yparam_const;
    //float ground;
    //float init_z;
    float lightbase;
}CalibParam;

class StereoCalib{
public:
    StereoCalib(const string &path){
        filehead = path;
        this->zpos = 0;
        this->ypos = 0;
        this->xpos = 0;
    };
    void KeyParam(int key);
    void Process(Mat &left,Mat &right);
private:
    void distanceCalib();
    void X_calib();
    void Y_calib();
    void FindTarget();
    void Position_show();
    void center_g(const vector<Point> contour,Point &center);
    void OtherOption();
public:
    Mat thresleft_cp,thresright_cp;
private:
    int rows,cols;
    int key;
    int calib_process = 0;
    int VisionDis = 0;
    vector<vector<Point> > contours_left,contours_right,Triangle_left,Triangle_right;
    vector<Vec4i> hierarchy_L,hierarchy_R;
    vector<Point> approx_left,approx_right;
    Point left_center,right_center;
    vector<Mat> split_left,split_right;
    Mat left_diff[2],right_diff[2];
    Mat init_left,init_right;
    int first_dis,second_dis;
    float xpos,ypos,zpos;
    float x_cal_z,x_cal_x1,x_cal_x2;
    Mat thresleft,thresright;
    int first_y,y_cal_z;
    double tan_theta;
    CalibParam Param;
    string filehead;
};



#endif /* defined(__stereo_calibrate_toe__StereoCalib__) */
