#ifndef __sudoku__HandWritingSolver__
#define __sudoku__HandWritingSolver__


#include "include/Sudoku/digitver/caffe_prototype.h"
#include "include/Sudoku/digitver/LogicProcess.h"
#include <pthread.h>

using namespace cv;

//static std::vector<Prediction> predictions[9];
namespace sudoku{

class NumberPredict;

struct param{
  NumberPredict *pt;
  int index;
  int isnew;
};

class NumberPredict{

public:
    NumberPredict(const string& model_file,
                   const string& trained_file,
                   const string& mean_file,
                   const string& label_file,
                   const string& low_trained_file);
    
    ~NumberPredict(){};
    void process(int isnew,vector<Rect> input,const Mat Binaryimg,Mat &img);
    static void Thresfor18(const Mat &input3,Mat &out);
private:
    static void *HandWritingProcess(void *arg);
public:
    Mat roi[9];
    vector<int> result,oldresult;      //value:  1 2 3
    vector<std::pair<int,int> > sudoku_repeat;            //        4 5 6
private:                                                  //        7 8 9
    Classifier classifier[9],lowrate;
    int64_t lostcount;

    Mat kernel;
    vector<std::pair<int,int> > Judgedata;      // 0: index 1: digit
    vector<Point> Position;
    pthread_t tid[9];
    pthread_mutex_t lock;
    struct param __ok[9];
    int threadflag[9];
    int result_from_thread[9];
    int result2_from_thread[9];
};
    
}




#endif /* defined(__sudoku__HandWritingSolver__) */
