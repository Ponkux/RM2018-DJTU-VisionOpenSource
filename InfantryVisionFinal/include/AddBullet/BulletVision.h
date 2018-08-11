#include "include/Header.h"
#include <map>
using namespace cv;

class BulletVision{
public:
    BulletVision(){};
    void init(const string &svmfile,const InitParams &params,int thresvalue);
    void process(Mat &img);
    void ParamFresh(const InitParams &params);
    int findtruesend();
public:
    Mat led_binary;
    int BulletCount;
    Mat kernel;
    Mat dilatethres;
    int transnum;
    std::map<int,int> table;
private:
    Ptr<cv::ml::SVM> svm;
    Mat led_gray,led_grayfull,led_binary_nouse,svmin,svmroi;
    vector<Mat> splited;
    vector<vector<Point>> contours;
    int thres;
    vector<Rect> LedAreas;
    int debug_flag;

};
