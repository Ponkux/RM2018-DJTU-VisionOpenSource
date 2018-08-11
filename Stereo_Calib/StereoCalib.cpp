
#include "StereoCalib.h"

void StereoCalib::Process(Mat &left,Mat &right){
    init_left = left;
    init_right = right;
    Triangle_left.clear();
    Triangle_right.clear();
    split(left,split_left);
    split(right, split_right);
    left_diff[0] = split_left[2] - split_left[1];
    right_diff[0] = split_right[2] - split_right[1];
    left_diff[1] = split_left[0] - split_left[1];
    right_diff[1] = split_right[0] - split_right[1];
    int total = right_diff[0].rows*right_diff[0].cols;
    rows = right_diff[0].rows;
    cols = right_diff[1].cols;
    thresleft = left_diff[0] + left_diff[1];
    thresright = right_diff[0] + right_diff[1];
    threshold(thresleft,thresleft,0,255,THRESH_OTSU);
    threshold(thresright,thresright,0,255,THRESH_OTSU);
    thresleft_cp = thresleft.clone();
    thresright_cp = thresright.clone();
    findContours(thresleft, contours_left,hierarchy_L,CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
    findContours(thresright, contours_right,hierarchy_R,CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
    int tlsize = 0;
    for(int i=0;i<contours_left.size();i++){
        approxPolyDP(contours_left[i], approx_left, 10, true);
        if(approx_left.size() == 4 && hierarchy_L[i][2] == -1 && hierarchy_L[i][3] != -1){
            contourArea(contours_left[i]);
            if(contourArea(contours_left[i]) > tlsize){
                tlsize = contourArea(contours_left[i]);
                Triangle_left.clear();
                center_g(contours_left[i], left_center);
                //circle(left, left_center, 5, Scalar(0,0,0),-1);
                Triangle_left.push_back(contours_left[i]);
            }
        }
    }
    for(int i=0;i<Triangle_left.size();i++){\
        if(tlsize==contourArea(Triangle_left[i])){
            center_g(Triangle_left[i], left_center);
        }
    }
    circle(left, left_center, 5, Scalar(0,0,0),-1);
    int trsize = 0;
    for(int i=0;i<contours_right.size();i++){
        approxPolyDP(contours_right[i], approx_right, 10, true);
        if(approx_right.size() == 4 && hierarchy_R[i][2] == -1 && hierarchy_R[i][3] != -1){
            contourArea(contours_right[i]);
            if(contourArea(contours_right[i]) > trsize){
                trsize = contourArea(contours_right[i]);
                Triangle_right.clear();
                center_g(contours_right[i], right_center);
                Triangle_right.push_back(contours_right[i]);
            }
        }
    }
    for(int i=0;i<Triangle_right.size();i++){\
        if(trsize==contourArea(Triangle_right[i])){
            center_g(Triangle_right[i], right_center);
        }
    }
    circle(right, right_center, 5, Scalar(0,0,0),-1);
    drawContours(left, Triangle_left, -1, Scalar(255,0,0),3);
    drawContours(right, Triangle_right, -1, Scalar(255,0,0),3);
    if(Triangle_left.size() == 1 && Triangle_right.size() == 1) VisionDis = abs(left_center.x - right_center.x);
    distanceCalib();
    X_calib();
    Y_calib();
    Position_show();
    OtherOption();
}

void StereoCalib::KeyParam(int key){
    this->key = key;
}

void StereoCalib::Position_show(){
    if(calib_process > 1 && VisionDis != 0){
        zpos = Param.distance_const/VisionDis;
    }
    if(calib_process > 3 && zpos > 0 && Param.f != 0){
        xpos = (left_center.x-cols/2)*zpos/Param.f;
    }
    if(calib_process > 5 && zpos > 0 && Param.f != 0){
        ypos = (Param.lightbase- left_center.y)*zpos/Param.f;
    }
    ostringstream strx,stry,strz;
    strx<<"x:"<<xpos<<"mm";
    stry<<"y:"<<ypos<<"mm";
    strz<<"z:"<<zpos<<"mm";
    putText(init_left, strx.str(), Point(cols-180,rows-100),FONT_HERSHEY_COMPLEX , 1, Scalar(255,0,255));
    putText(init_left, stry.str(), Point(cols-180,rows-70),FONT_HERSHEY_COMPLEX , 1, Scalar(255,0,255));
    putText(init_left, strz.str(), Point(cols-180,rows-40),FONT_HERSHEY_COMPLEX, 1, Scalar(255,0,255));
}

void StereoCalib::distanceCalib(){
    if(calib_process == 0){
        putText(init_left, "Press 'd' for 1st position.", Point(100,50), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255,0,255));
        if(key == 'd'){
            first_dis = VisionDis;
            Param.height = rows;
            Param.width = cols;
            calib_process = 1;
            std::cout<<"Move away 10.0cm,then press 'x'."<<std::endl;
        }

    }
    if(calib_process == 1){
        putText(init_left,"Move away 10.0cm,then press 'x'.", Point(100,50), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255,0,255));
        if(key == 'x'){
            second_dis = VisionDis;
            Param.distance_const = 100.0*((float)(first_dis * second_dis))/((float)(first_dis - second_dis));
            calib_process = 2;
            std::cout<<"Stay and press 'd'."<<std::endl;
        }
    }
}

void StereoCalib::X_calib(){
    if(calib_process == 2){
        putText(init_left, "Stay and press 'd'.", Point(100,50), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255,0,255));
        if(key == 'd'){
            x_cal_z = zpos;
            x_cal_x1 = left_center.x;
            calib_process = 3;
            std::cout<<"Move right 10cm,then press 'x'."<<std::endl;
        }
    }
    if(calib_process == 3){
        putText(init_left, "Move right 10cm,then press 'x'.", Point(100,50), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255,0,255));
        if(key == 'x'){
            x_cal_x2 = left_center.x;
            Param.f = (x_cal_x2 -x_cal_x1) * x_cal_z / 100.0;
            calib_process = 4;
            std::cout<<"Move to first z.Stay and press 'd'."<<std::endl;
        }
    }
}

void StereoCalib::Y_calib(){
    if(calib_process == 4){
        putText(init_left, "Move to first z.Stay and press 'd'.", Point(100,50), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255,0,255));
        if(key == 'd'){
            first_y = left_center.y;
            y_cal_z = zpos;
            calib_process = 5;
            std::cout<<"Move to second z.Stay and press 'x'."<<std::endl;
        }
    }
    if(calib_process == 5){
         putText(init_left, "Move to second z.Stay and press 'x'.", Point(100,50), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255,0,255));
         if(key == 'x'){
             Param.lightbase = (first_y * y_cal_z - left_center.y * zpos) / (y_cal_z - zpos);
             calib_process = 6;
             std::cout<<"finish"<<std::endl;
         }
    }
}


void StereoCalib::OtherOption(){
    if(calib_process == 6){
        filehead.append("stereo_calib.xml");
        FileStorage fs(filehead,FileStorage::WRITE);
        fs<<"height"<<Param.height
        <<"width"<<Param.width
        <<"distance_const"<<Param.distance_const
        <<"f"<<Param.f
        <<"lightbase"<<Param.lightbase;
        fs.release();
        calib_process = 7;
    }
}

void StereoCalib::center_g(const vector<Point> contour,Point &center){
    Moments mu;                                                 // Get the center of a contour
    mu = moments(contour,false);
    center.x=mu.m10/mu.m00;
    center.y=mu.m01/mu.m00;
}
