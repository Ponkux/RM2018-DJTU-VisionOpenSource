#include "include/Sudoku/digitver/LogicProcess.h"
namespace sudoku{

/**
  * @brief return the result for shooting(shoot which one and which position)
  * @param Ledsorted        : led number
  * @param HandWritingsorted: sudoku board number
  * @return pair.first : which one
  *         pair.second: the number's position in the sudoku board
  */
std::pair<int,int> LogicProcess::process(vector<int> Ledsorted,vector<int> HandWritingsorted)
{
  if(Ledsorted.size() !=5 || HandWritingsorted.size() < 3){
    return result;
  }
  minhandwritingcount = HandWritingNums.size() < HandWritingsorted.size() ? HandWritingNums.size() : HandWritingsorted.size();
  if(LedNums.size() == 0){
    LedNums = Ledsorted;
    HandWritingNums = HandWritingsorted;  //init
    flag = 1;
  }else{
    int count = 0;
    for(int i=0;i<5;i++){
        if(LedNums[i] != Ledsorted[i]) count++;  // led change?
    }
    LedNums = Ledsorted;
    if(count > 2){  // led has change!
        std::cout<<"led has change!!!!!"<<std::endl;
        startflag = 0;
        flag = 1;
    }else{
      LedChangeCount = 0;
      startflag++;
      int changecount = 0;
      for(int i=0;i<minhandwritingcount;i++){
          if(HandWritingNums[i] != HandWritingsorted[i]) changecount++;
      }
      if(changecount > 4){
        if(startflag < 25){ // change not yet
          flag = 1;
        }else{
          flag++;
        }
        HandWritingNums = HandWritingsorted;
      }
    }
  }
  if(startflag < 5) return result;
    result.first = flag; // ?st number
    int target;
    if(flag > 5 || flag == 0){
        target = 0;
    }else{
        target = Ledsorted[flag-1];
    }
    if(target == 0){
        result.second = 0;
        return result;
    }
    for(int i=0;i<minhandwritingcount;i++){
        if(HandWritingsorted[i] == target){
            result.second = i+1;
            break;
        }
    }
    return result;
}

void LogicProcess::DrawTarget(cv::Mat &input,vector<cv::Rect> targets){
    if(targets.size()>= result.second && result.second != 0){
        cv::rectangle(input,targets[result.second-1], cv::Scalar(255,0,0),2);
    }
}

}




