#ifndef __sudoku__ImageProcess__
#define __sudoku__ImageProcess__

#include "include/Header.h"


using namespace cv;

namespace sudoku{
    

class SortRects{
public:
    SortRects(Rect input){
        x=input.tl().x;
        y=input.tl().y;
        height = input.height;
        width  = input.width;
    }
    int x,y,height,width;
    bool operator < (const SortRects &s)const{return x< s.x;}
};


class SortSudokus{
public:
    SortSudokus(Rect input){
        x=input.tl().x;
        y=input.tl().y;
        height = input.height;
        width  = input.width;
    }
    SortSudokus(Rect input,Point contour_center){
        x=input.tl().x;
        y=input.tl().y;
        height = input.height;
        width  = input.width;
        center = contour_center;
    }
    int x,y,height,width;
    Point center;
    bool operator < (const SortSudokus &s)const{
        if(s.y - y < -20) return false;
        if (s.y - y > 40) {
            return true;
        }else if (s.x - x > 60){
            return true;
        }
        return false;
    }
};


typedef struct{
    vector<Rect> SudokuArea;
    vector<Rect> LedArea;
    Mat graydown;
    Mat for18led;
    Mat thresdown;
    Mat LedRoi;
    vector<SortSudokus> sudokus;
    Mat board_binary;
}ImgThreadOutput;

class ImageProcess{
public:
    ImageProcess();
    ImageProcess(const InitParams &params);
    ~ImageProcess(){};
    void process(Mat input,int isledset,int isnew,ImgThreadOutput &out);
    void dataout(Mat &output,Mat &outgrayup,Mat &outgraydown,Mat &thresholdimg);
private:
    bool GetRegion();
    void Process(int isnew);
    void ThresholdProcess(int isnew);
    void ContourProcess(int isnew);
    void PredictLost();
    void DeleteRedund();
    Point get_contour_center(const vector<Point> contour);
public:
    Mat grayup,graydown,thresdown,thresdown1st,thres4led;
    Mat inputgray;
    Mat imginit,img;
    Mat imgup,imgdown;
    Mat LedRoi,RoiBinary,RoiBinary_nouse;
    vector<SortRects> Lednums;
    vector<SortSudokus> sudokus;
    vector<Rect> LedArea;
    vector<Rect> SudokuArea;
	vector<SortRects> old_Lednums;
    vector<SortSudokus> old_sudokus;
    vector<Rect> old_LedArea;
    vector<Rect> old_SudokuArea;
    
    Mat for18led;
    int top,topx,twidth;
    int sudokunew_board_thres;
private:
    int leddoflag;
    int ltop,ltopx,ltwidth;
    Mat contour_nouse;
    std::vector<std::vector<Point> > contours,smallcontours;
    std::vector<std::pair<std::vector<Point>,cv::Rect> > contoursfirst;
    std::vector<std::vector<Point> > innercontour;
    int debug_flag;
	int lostflag;
    int shight_max;
    Mat usekernel = getStructuringElement(MORPH_ELLIPSE,Size(9,9));
    Mat kernel = getStructuringElement(MORPH_ELLIPSE,Size(13,13));
    Mat erodekernel = getStructuringElement(MORPH_ELLIPSE,Size(5,5));
    vector<Mat> splited;
    Mat Ledroi1st;

    bool RegionDetect;
    Rect RegionRect;
    Mat R_thres;
    std::vector<std::vector<Point> > RegionContours;
    vector<Rect> ScoreBoards;
};
    
}


#endif /* defined(__sudoku__ImageProcess__) */
