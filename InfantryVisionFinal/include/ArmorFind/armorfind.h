#ifndef ARMORFIND_H
#define ARMORFIND_H

#include "include/Header.h"

using namespace cv;

static inline bool PairSort(std::pair<Point,Rect> a1,std::pair<Point,Rect> a2){
    return a1.first.x < a2.first.x;
}

static inline bool RotateRectSort(RotatedRect a1,RotatedRect a2){
    return a1.center.x < a2.center.x;
}

class ArmorFind
{
public:
    ArmorFind();
    void process(vector<vector<Point> > contours,const Mat &input,Mat &output,vector<Point> &result,bool ismono);
private:
    void ContourCenter(const vector<Point> contour,Point &center);
    void RotateRectLine(RotatedRect rect,Point &lineup,Point &linedown);
    double Pointdis(const Point &p1,const Point &p2);
    Point PointBetween(const Point &p1,const Point &p2);
    void Clear(void);
    void GetArmorLights(void);
    void GetArmors(Mat &image,bool ismono);
    double GetK(Point2f L1,Point2f L2);
    void DrawCross(Mat &img,Point center,int size,Scalar color,int thickness = 2);

public:
    Mat binary;
    vector<Point> ArmorCenters,ArmorOldCenters;
    vector<Monodata> monodata;
private:
    int ArmorLostDelay;
    int armormin;
    Mat kernel;
    vector<vector<RotatedRect> > Armorlists;
    vector<int> CellMaxs;
    vector<Mat> splited;
    vector<vector<Point> > final;
    vector<Rect> Target;
    vector<Point> centers;
    vector<std::pair<Point,Rect> > PairContour;
    vector<RotatedRect> RectfirstResult,RectResults;
    Mat armor_roi;

};


#endif // ARMORFIND_H
