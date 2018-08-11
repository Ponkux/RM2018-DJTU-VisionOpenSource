#ifndef __sudoku__LogicProcess__
#define __sudoku__LogicProcess__

#include "include/Header.h"

namespace sudoku{

class LogicProcess{
public:
    LogicProcess(){
        LedNums.clear();
        HandWritingNums.clear();
        startflag = 0;
        flag = 1;
        LedChangeCount = 0;
    };
    ~LogicProcess(){};
    std::pair<int,int> process(vector<int> Ledsorted,vector<int> HandWritingsorted);
    void DrawTarget(cv::Mat &input,vector<cv::Rect> targets);
    std::pair<int,int> result,last;
private:
    vector<int> LedNums,HandWritingNums;
    vector<int> LastLedSaver;
    int flag;
    int startflag;
    int led_changedflag;
    int lostflag;
    int minhandwritingcount;
    int LedChangeCount;
};
    
}


#endif /* defined(__sudoku__LogicProcess__) */
