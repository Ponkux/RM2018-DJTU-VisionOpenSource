#include "include/ArmorFind/armorpredict.h"

ArmorPredict::ArmorPredict()
{

}

ArmorPredict::ArmorPredict(const string &File640,const string &File1280,const string &File1920,InitParams param){
    memset(&Result,0,sizeof(AbsPosition));
    Stereo640.Init(File640);
    Stereo1280.Init(File1280);
    Stereo1920.Init(File1920);
    mono_small_armor_720p_const = param.mono_small_armor_720p_const;
    mono_small_armor_1080p_const = param.mono_small_armor_1080p_const;
    mono_big_armor_720p_const = param.mono_big_armor_720p_const;
    mono_big_armor_1080p_const = param.mono_big_armor_1080p_const;
    mono_f_720p = param.mono_f_720p;
    mono_f_1080p = param.mono_f_1080p;
    mono_base_720p = param.mono_base_720p;
    mono_base_1080p = param.mono_base_1080p;
    mono_x = param.mono_x;
    mono_y = param.mono_y;
    mono_z = param.mono_z;
}

void ArmorPredict::Fresh(){
    Positions.clear();
    yaw_old = yaw_out;
    pitch_old = pitch_out;
    yaw_old_speed = yaw_out_speed;
    yaw_out = 0;
    pitch_out = 0;
    yaw_out_speed = 0;
    shoot_speed = 0;
}
/**
  * @brief get the corrdinate of the armor detected and calculate the yaw/pitch offset.
  * @param mono_input: data from armorfind
  * @param videotype : resolution code for the image
  *                    1280 for 1280x720
  *                    1920 for 1920x1080
  * @param time      : current system time get from OpenCV
  * @param robotlevel: current level of the infantry robot (0,1,2,3) // not use this param finally
  * @return
  */
void ArmorPredict::PredictMono(vector<Monodata> &mono_input,int videotype,double time,int robotlevel){
    if(mono_input.size() == 0){
        Data.yaw_angle.d = 0;
        Data.pitch_angle.d = 0;
        return;
    }
    if(time != 0){
        T_last = T_curr;
        T_curr = time;
        T_between = (double)(T_curr - T_last) / getTickFrequency();
        if(T_between > 0.5) T_between = 0;
    }
    int size = mono_input.size();
    for(int i=0;i<size;i++){
        if(mono_input[i].armor == mono_input[i].small_armor){
            if(videotype == 1280){
                mono_input[i].distance = sqrtf(mono_small_armor_720p_const/mono_input[i].area);
            }else{
                mono_input[i].distance = sqrtf(mono_small_armor_1080p_const/mono_input[i].area);
            }
        }else{
            if(videotype == 1280){
                mono_input[i].distance = sqrtf(mono_big_armor_720p_const/mono_input[i].area);
            }else{
                mono_input[i].distance = sqrtf(mono_big_armor_1080p_const/mono_input[i].area);
            }
        }
    }
    std::sort(mono_input.begin(),mono_input.end(),[](const Monodata &d1,const Monodata &d2){
                                                    return d1.distance < d2.distance;
                                                });
    //std::cout<<"distance:"<<mono_input[0].distance<<std::endl;
    float cal_x,cal_y;
    if(videotype == 1280){
        cal_x = (mono_input[0].center.x-640)*mono_input[0].distance/mono_f_720p;
        cal_y = (mono_base_720p-mono_input[0].center.y)*mono_input[0].distance/mono_f_720p;
    }else{
        cal_x = (mono_input[0].center.x-960)*mono_input[0].distance/mono_f_1080p;
        cal_y = (mono_base_1080p-mono_input[0].center.y)*mono_input[0].distance/mono_f_1080p;
    }
    //std::cout<<"x:"<<cal_x<<" y:"<<cal_y<<std::endl;
    float last_x = mono_Pos.x;
    mono_Pos.x = cal_x - mono_x;
    mono_Pos.y = cal_y - mono_y;
    mono_Pos.z = mono_input[0].distance - mono_z;
    if(mono_Pos.z <= 0) return;
    int16_t yangle = (int16_t)(-atan(mono_Pos.x/mono_Pos.z)*1303.7972938);
    Data.yaw_speed.d = 0;
    if(T_between != 0 && time != 0 && Data.yaw_angle.d != 0){
        Data.yaw_speed.d = yangle - Data.yaw_angle.d;
         if(abs(last_x - mono_Pos.x) < 4) Data.yaw_speed.d = 0;
    }
    Data.yaw_angle.d = yangle;
    Data.pitch_angle.d =(int16_t)(atan(mono_Pos.y/mono_Pos.z)*1303.7972938);
    Data.shoot_speed.f = 0;
    Data.z.f = mono_input[0].distance;
    //std::cout<<"speed:"<<Data.yaw_speed.d<<std::endl;
    //std::cout<<"dis:"<<Data.z.f<<std::endl;
    //std::cout<<"yaw:"<<Data.yaw_angle.d<<"pitch"<<Data.pitch_angle.d<<std::endl;
}

/**
  * @brief get the position of the armor measured by the binocular vision system
  * @param Left/Right: positions of armor get from the image of left/right camera
  * @param videotype : resolution code for the image
  *                    1280 for 1280x720
  *                    1920 for 1920x1080
  * @param time      : current system time get from OpenCV
  * @param robotlevel: current level of the infantry robot (0,1,2,3)
  */
void ArmorPredict::Predict(vector<Point> Left, vector<Point> Right,int videotype,double time,int robotlevel){
    Fresh();
    T_last = T_curr;
    T_curr = time;
    T_between = (double)(T_curr - T_last) / getTickFrequency();
    if(T_between > 0.5) T_between = 0;
    if(Left.size() == 0 && Right.size() == 0){
        yaw_out = 0;
        pitch_out = 0;
        yaw_out_speed = 0;
        memset(&Data,0,sizeof(VisionData));
        LeftLast = Left;
        RightLast = Right;
    }else if(Left.size() != 0 && Right.size() == 0){
        if(LeftLast.size() != 0){
            yaw_out = 0;
            pitch_out = 0;
            shoot_speed  = -1.0;
            memset(&Data,0,sizeof(VisionData));
        }else{
            yaw_out = 0;
            pitch_out = 0;
            yaw_out_speed = 0;
            memset(&Data,0,sizeof(VisionData));
        }
    }else if(Right.size() != 0 && Left.size() == 0){
        if(RightLast.size() != 0){
            yaw_out = 0;
            pitch_out = 0;
            yaw_out_speed = 1.0;// right motion
            memset(&Data,0,sizeof(VisionData));
        }else{
            yaw_out = 0;
            pitch_out = 0;
            yaw_out_speed = 0;
            memset(&Data,0,sizeof(VisionData));
        }
    }else{
        auto sort_point = [](const Point &a1,const Point &a2){
            return a1.x < a2.x;
          };
        std::sort(Left.begin(),Left.end(),sort_point);
        std::sort(Right.begin(),Right.end(),sort_point);
        size_t Lsize = Left.size();
        size_t Rsize = Right.size();
        for(int i=0,j=0;i<Lsize,j<Rsize;i++,j++){
            if(Left[i].x > Right[j].x){
                if(videotype == 1920){
                    AbsPosition Pos;
                    Stereo1920.Calculation(Left[i],Right[j],Pos);
                    if(Pos.z > 30){
                        Pos.x -= x_trans;
                        Pos.y -= y_trans;
                        Pos.z -= z_trans;
                        Pos.index = i;
                        Positions.push_back(Pos);
                    }else{
                        i++;
                    }
                }else{
                    AbsPosition Pos;
                    Stereo1280.Calculation(Left[i],Right[j],Pos);
                    if(Pos.z > 30){
                        Pos.x -= x_trans;
                        Pos.y -= y_trans;
                        Pos.z -= z_trans;
                        Pos.index = i;
                        Positions.push_back(Pos);
                    }else{
                        i++;
                    }
                }
            }else{
                i++;
            }
        }
    }
    if(Positions.size() != 0){
        if(Positions.size() != 1){
            sort(Positions.begin(),Positions.end(),PosSort);
        }
        if(Positions.size() > 1){
            if(fabs(Positions[0].z - Positions[1].z) < 100 && OldResult.z != 0){
                float q0 = fabs(OldResult.x - Positions[0].x);
                float q1 = fabs(OldResult.x - Positions[1].x);
                Result = q0 < q1 ? Positions[0] : Positions[1];
            }else{
                Result = Positions[0];
            }
        }else{
            Result = Positions[0];
        }

        ForDebug.x = Positions[0].x + x_trans;
        ForDebug.y = Positions[0].y + y_trans;
        ForDebug.z = Positions[0].z + z_trans;
        Data.x.f = Result.x;
        Data.y.f = Result.y;
        Data.z.f = Result.z;
        Leftcenter = Left[Result.index];
        Rightcenter = Right[Result.index];
        if(Result.z < 30){
            yaw_out = 0;
            pitch_out = 0;
        }else{
            AngleFit(Result,&pitch_out,&yaw_out,&shoot_speed,robotlevel);
        }
        if(fabs(yaw_out) > 500 ||fabs(pitch_out) > 500) {
            yaw_out = yaw_old;
            pitch_out = pitch_old;
        }
        if(fabs(OldResult.x - Result.x) > 1000){
            yaw_out_speed = 0;
        }else{
            if(yaw_out != 0.0 && yaw_old != 0.0){
                yaw_out_speed = (yaw_out - yaw_old)/T_between;//encoder/s
            }
        }
    }
    //std::cout<<"yaw:"<<yaw_out<<" pitch:"<<pitch_out<<std::endl;
    OldResult = Result;
    OldPositions = Positions;
    LeftLast = Left;
    RightLast = Right;
    Data.pitch_angle.d = (int16_t)pitch_out;
    Data.yaw_angle.d   = (int16_t)yaw_out;
    Data.yaw_speed.d   = (int16_t)yaw_out_speed;
    Data.shoot_speed.f   = shoot_speed;
    Data.mode = 1;
    //std::cout<<"Z:"<<ForDebug.z<<std::endl;
    //std::cout<<"shootspeed:"<<shoot_speed<<std::endl;
}

/**
  * @brief calculate the distance between each 2 3-D points
  * @param a1 a2: 3-D points
  * @return distance
  */
float ArmorPredict::CenterDis(const AbsPosition a1,const AbsPosition a2){
    float dx2 = a2.x - a1.x;
    float dy2 = a2.y - a1.y;
    float dz2 = a2.z - a1.z;
    return sqrtf(dx2*dx2 + dy2*dy2 + dz2*dz2);
}

/**
  * @brief kalman filter for point. never use
  */
void ArmorPredict::Classical_kalman(Kalman4Point &KF){
    Point2f newbias,K,Kg;                       //Kalmanfliter
    KF.P_old.x   = KF.P_Predict.x;
    KF.P_old.y   = KF.P_Predict.y;
    newbias.x = KF.P_old.x - KF.P_now.x;
    newbias.y = KF.P_old.y - KF.P_now.y;
    K.x       = fabs(2.0*newbias.x);
    K.y       = fabs(2.0*newbias.y);
    Kg.x      = sqrt((KF.bias.x*KF.bias.x+KF.err*KF.err)/((KF.bias.x*KF.bias.x+KF.err*KF.err)+K.x*K.x));
    Kg.y      = sqrt((KF.bias.y*KF.bias.y+KF.err*KF.err)/((KF.bias.y*KF.bias.y+KF.err*KF.err)+K.y*K.y));
    KF.bias.x    = sqrt((1-Kg.x)*(KF.bias.x*KF.bias.x+KF.err*KF.err));
    KF.bias.y    = sqrt((1-Kg.y)*(KF.bias.y*KF.bias.y+KF.err*KF.err));
    KF.P_Predict.x   = KF.P_now.x+Kg.x*(newbias.x);
    KF.P_Predict.y   = KF.P_now.y+Kg.y*(newbias.y);
}
/**
  * @brief calculate the yaw/pitch offset and the expect speed of the bullet
  * @param input: position of the target
  * @param pitch: pitch offset
  * @param yaw  : yaw offset
  * @param shootspeed: the speed of the bullet (m/s)
  * @return
  */
void ArmorPredict::AngleFit(const AbsPosition input,float *pitch,float *yaw,float *shootspd,int level){
    float flytime,gravity_offset;
    float shoot_distance = sqrtf(input.x * input.x + input.y * input.y + input.z * input.z)*0.001; // m
    // expect bullet fly time is 0.1s
    *shootspd = shoot_distance * 10.0;//
    *yaw = atan(input.x/input.z)*1303.7972938;
    int shootspeedmax;
    // max bullet speed:
    // level 0 / level 1: 22m/s
    // level 2          : 25m/s
    // level 3          : 28m/s
    switch(level){
        case 0:
        case 1:shootspeedmax = 22;break;
        case 2:shootspeedmax = 25;break;
        case 3:shootspeedmax = 28;break;
    }

    if(*shootspd != 0){
        if(*shootspd > shootspeedmax){
            *shootspd = shootspeedmax;
            flytime = shoot_distance / (*shootspd);
            gravity_offset = 4.905 * flytime * flytime;
        }else if(*shootspd < 14.0){
            *shootspd = 14.0;
            flytime = shoot_distance / (*shootspd);
            gravity_offset = 4.905 * flytime * flytime;
        }else{
            gravity_offset = 0.04905;
        }
    }
    //std::cout<<"shootspeed:"<<*shootspd<<std::endl;
    *pitch = -atan((input.y+gravity_offset*1000.0)/input.z)*1303.7972938;
}

void ArmorPredict::ShowResult(Mat &Limg,Mat &Rimg){
    circle(Limg,Leftcenter,5,Scalar(255,0,0),-1);
    circle(Rimg,Rightcenter,5,Scalar(255,0,0),-1);
    ostringstream strx,stry,strz,strspeed;
    strx<<"x:"<<ForDebug.x<<"mm";
    stry<<"y:"<<ForDebug.y<<"mm";
    strz<<"z:"<<ForDebug.z<<"mm";
    //strspeed<<"speed:"<<yaw_out_speed<<"mm/s";
    putText(Limg,strspeed.str(),Point(100,50),FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0,255,255));
    putText(Limg,strx.str(),Point(100,80),FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0,255,255));
    putText(Limg,stry.str(),Point(100,110),FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0,255,255));
    putText(Limg,strz.str(),Point(100,140),FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0,255,255));
}

