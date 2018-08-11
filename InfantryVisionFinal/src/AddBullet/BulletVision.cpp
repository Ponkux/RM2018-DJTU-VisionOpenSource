#include "include/AddBullet/BulletVision.h"

void BulletVision::init(const string &svmfile,const InitParams &params,int thresvalue){
    thres = thresvalue;
    kernel = getStructuringElement(MORPH_ELLIPSE,Size(9,9));
    svm = cv::ml::SVM::StatModel::load<cv::ml::SVM>(svmfile);
    transnum = 0;
}

void BulletVision::ParamFresh(const InitParams &params){
    debug_flag = params.debug_flag;
    thres = params.bullet_thres;
}
int BulletVision::findtruesend(){
    int num = 0;
    int size = table.size();
    if(size == 0) return 0;
    for(int i=0;i<size;i++){
        if(table[i] > num) num = i;
    }
    return num;
}

void BulletVision::process(Mat &img){
    LedAreas.clear();
    split(img,splited);
    led_gray = splited[2] - splited[0];
    //cvtColor(img,led_gray,CV_BGR2GRAY);
    //threshold(led_gray,led_binary,thres,255,THRESH_BINARY);
    threshold(led_gray,led_binary,0,255,THRESH_OTSU);
    //cvtColor(img,led_grayfull,CV_BGR2GRAY);
    //dilate(led_grayfull,led_grayfull,kernel);
    //dilatethres = led_grayfull & led_binary;
    dilate(led_binary,dilatethres,kernel);
    dilatethres.copyTo(led_binary_nouse);
    findContours(led_binary_nouse, contours,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    int maxsize = 0;
    size_t contoursize = contours.size();
    for(int i=0;i<contoursize;i++){
        if(contours[i].size() > maxsize) maxsize = contours[i].size();
    }\
    maxsize /= 3;
    if(maxsize < 20){
        BulletCount = 0;
        return;
    }
    for(int i=0;i<contoursize;i++){
        if(contours[i].size() > maxsize){
            Rect putin = boundingRect(contours[i]);
            if(putin.width < putin.height*1.2 && putin.width > putin.height* 0.2 && putin.area() > 300){
                LedAreas.push_back(boundingRect(contours[i]));
            }
        }
    }
    if(LedAreas.size() != 3){
        BulletCount = 0;
        return;
    }
    std::sort(LedAreas.begin(),LedAreas.end(),[](const Rect a1,const Rect a2){
                                return a1.x > a2.x;
                                 });
    BulletCount = 0;
    for(int i=0;i<3;i++){
        int val;
        if(LedAreas[i].height > 2.0*LedAreas[i].width){
            val = 1;
        }else{
            HOGDescriptor *hog = new cv::HOGDescriptor(cvSize(28, 28), cvSize(14, 14), cvSize(7, 7), cvSize(7, 7), 9);
            vector<float>descriptors;
            svmroi = led_binary(LedAreas[i]);
            resize(svmroi,svmroi,Size(20,20));
            threshold(svmroi,svmroi,128,255,THRESH_BINARY_INV);
            Mat inner = Mat::ones(28,28,CV_8UC1)+254;
            svmroi.copyTo(inner(Rect(4,4,20,20)));
            hog->compute(inner,descriptors, cv::Size(1, 1), cv::Size(0, 0));
            Mat SVMPredictMat = Mat(1, (int)descriptors.size(), CV_32FC1);
            memcpy(SVMPredictMat.data,descriptors.data(),descriptors.size()*sizeof(float));
            val = svm->predict(SVMPredictMat);
            if(i == 2) val = 0;
        }
        BulletCount += val*pow(10,i);
        if(BulletCount > 160) BulletCount = 160;
        if(debug_flag){
            stringstream value;
            value<<val;
            rectangle(img,LedAreas[i],Scalar(255,255,255),2);
            putText(img,value.str(),Point(LedAreas[i].tl().x,LedAreas[i].tl().y - 60),CV_FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255,0,255));
        }
    }
    if(debug_flag){
        std::cout<<"BulletCount: "<<transnum<<std::endl;
    }
}
