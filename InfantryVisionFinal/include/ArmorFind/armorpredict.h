#ifndef ARMORPREDICT_H
#define ARMORPREDICT_H

#include "include/Header.h"
#include "include/Camera/StereoXML.h"
#include "include/Serial/serial.h"

using namespace cv;

static inline bool PosSort(AbsPosition a1,AbsPosition a2){
    return a1.z < a2.z;
}

class ArmorPredict
{
public:
    typedef struct{
        Point bias;
        float err;
        Point P_old;
        Point P_now;
        Point P_Predict;
    }Kalman4Point;
    ArmorPredict();
    ArmorPredict(const string &File640,const string &File1280,const string &File1920,InitParams param);
    void SetAxis(float x,float y,float z){
        x_trans = x;
        y_trans = y;
        z_trans = z;
    }
    void Predict(vector<Point> Left,vector<Point> Right,int videotype,double time,int robotlevel);
    void PredictMono(vector<Monodata> &mono_input,int videotype,double time,int robotlevel);
    void ShowResult(Mat &Limg,Mat &Rimg);
public:
    VisionData Data;
    AbsPosition Result,OldResult,ForDebug;

private:
    float CenterDis(const AbsPosition a1,const AbsPosition a2);
    void Classical_kalman(Kalman4Point &KF);
    void Fresh();
    void AngleFit(const AbsPosition input,float *pitch,float *yaw,float *shootspd,int level);
private:
    double T_curr,T_last;
    double T_between;
    Point Leftcenter,Rightcenter;
    float x_trans,y_trans,z_trans;
    StereoXML Stereo640,Stereo1280,Stereo1920;
    vector<Point> LeftLast,RightLast;
    vector<AbsPosition> Positions,OldPositions;
    float yaw_old,pitch_old,yaw_old_speed;
    float yaw_out,pitch_out,yaw_out_speed;
    float shoot_speed;
    float x_old,y_old,z_old;

    float mono_small_armor_720p_const;
    float mono_small_armor_1080p_const;
    float mono_big_armor_720p_const;
    float mono_big_armor_1080p_const;
    float mono_f_720p;
    float mono_f_1080p;
    float mono_base_720p;
    float mono_base_1080p;
    float mono_x;
    float mono_y;
    float mono_z;
    AbsPosition mono_Pos;
};

#endif // ARMORPREDICT_H
