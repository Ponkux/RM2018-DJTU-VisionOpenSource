#ifndef sudoku_Header_h
#define sudoku_Header_h

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include <vector>
#include <opencv2/core/core.hpp>
#include <string>
#include <math.h>
#include <caffe/caffe.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <utility>

using namespace caffe;  // NOLINT(build/namespaces)
using std::string;
using std::cout;
using std::endl;

typedef struct{
    int armor_thres_red;
    int armor_thres_blue;
    int debug_flag;
	int debug_mode;
    int x;
    int y;
    int z;
    int armor_exp_red;
    int armor_exp_blue;
    int sudoku_exp;
    int sudokunew_exp;
    int sudoku_led_thres;
    int sudokunew_led_thres;
    int sudokunew_board_thres;
    int bulletadd_exp;
    int bullet_thres;

    int writemov;
    int usevideo;
    string video_path;

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

}InitParams;

typedef struct{
    typedef enum{
        small_armor,
        big_armor
    }armor_type;
    cv::Point center;
    float area;
    float distance;//mm
    armor_type armor;
}Monodata;
 
template<typename T>
void CheckValue(const string &dataname,T dataold,T datanew){
    if(dataold != datanew){
        cout<<dataname<<":"<<datanew;
    }
}

#endif
