#ifndef __sudoku__LedNumberSolver__
#define __sudoku__LedNumberSolver__

#include "include/Header.h"
#include "include/Sudoku/digitver/LogicProcess.h"
using namespace cv;

namespace sudoku{

class LedNumberSolver{
public:
    LedNumberSolver(const string file,InitParams params){
		svm = cv::ml::SVM::StatModel::load<cv::ml::SVM>(file);
        kernel = getStructuringElement(MORPH_RECT, Size(3,3));
        debug_flag = params.debug_flag;
    };
    ~LedNumberSolver(){};
    void ParamsFresh(const InitParams &params);
    void process(vector<Rect> input,const Mat ForBinary,Mat &img,int thres);

public:
    vector<int> result; //from left to right
    std::vector<Mat> ledpics;
    Mat inputcopy;
private:
    Ptr<cv::ml::SVM> svm;
    Mat roi,predictimg;
    Mat kernel;
    int debug_flag;
	long countsave,lostframe;


};

}

#endif /* defined(__sudoku__LedNumberSolver__) */
