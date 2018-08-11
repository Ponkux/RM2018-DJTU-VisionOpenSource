#include "include/Sudoku/digitver/HandWritingSolver.h"

namespace sudoku{

/**
  * @brief predict the number by caffe model
  *        this is a multi-thread function
  */
void * NumberPredict::HandWritingProcess(void *arg){
  NumberPredict *pt=((struct param *)arg)->pt;
  int i = ((struct param *)arg)->index;
  int isnew = ((struct param *)arg)->isnew;
  if(!isnew){
    threshold(pt->roi[i],pt->roi[i],0,255,THRESH_OTSU + THRESH_BINARY_INV);
    resize(pt->roi[i],pt->roi[i],Size(28,28));
  }else{
    Mat proc1,proc2;
    proc1 = pt->roi[i].clone();
    //bilateralFilter(pt->roi[i],proc1,5,100,3);
    int h = pt->roi[i].rows;
    int w = pt->roi[i].cols;
    int sizemax = MAX(h,w);
    float rate = 80.0/(float)sizemax;
    h = h * rate;
    w = w * rate;
    resize(proc1,proc1,Size(w,h));
    Thresfor18(proc1,proc2);
    //Filterfor18(proc1,proc2);
    Mat finalmat = Mat::zeros(100,100,CV_8UC1);
    proc2.copyTo(finalmat(Rect(0.5*(100-w),0.5*(100-h),w,h)));
    pt->roi[i] = finalmat;
    resize(pt->roi[i],pt->roi[i],Size(28,28));
  }

  std::vector<Prediction> predictions = pt->classifier[i].Classify(pt->roi[i]);
  Prediction p = predictions[0];
  int value = atoi(p.first.c_str());
  if(value == 0){
      p = predictions[1];
      value = atoi(p.first.c_str());
      //cout<<i<<"th value is 0:"<<endl;
  }

  pt->result_from_thread[i] = value;
  pt->threadflag[i]++;
  pthread_detach(pthread_self());
}

void NumberPredict::Thresfor18(const Mat &input3,Mat &out){
    cvtColor(input3,out,CV_BGR2GRAY);
    threshold(out,out,0,255,THRESH_OTSU);
}

NumberPredict::NumberPredict
            (const string& model_file,
             const string& trained_file,
             const string& mean_file,
             const string& label_file,
             const string& low_trained_file)
{
    for(int i=0;i<9;i++){
      threadflag[i] = 0;
      classifier[i].Initialize(model_file, trained_file, mean_file, label_file);
    }

    lowrate.Initialize(model_file, low_trained_file, mean_file, label_file);
    kernel = getStructuringElement(MORPH_RECT, Size(3,3));
}

/**
  * @brief get the predict of numbers on the region of sudoku board
  * @param isnew    : input 0 : detect the small power rune. other: detect the big power rune
  * @param input    : rectangle region of the sudoku boards
  * @param Binarying: the binary image input
  * @param img      : output with results shows on the image
  * @return
  */
void NumberPredict::process(int isnew,vector<Rect> input,Mat Binaryimg,Mat &img){
    size_t size = input.size();
    oldresult = result;
    if(size < 2 || size > 9) {
        return;
    }
    Judgedata.clear();
    Position.clear();
    sudoku_repeat.clear();
    Point OutValue;
    result.clear();
    int inputsize = input.size();
    for(int i=0;i<inputsize;i++){
        OutValue.x = input[i].tl().x+10;
        OutValue.y = input[i].tl().y-5;
        if(OutValue.y < 0 ) OutValue.y = 0;
        if(OutValue.x > 1200) OutValue.x = 1200;
        Position.push_back(OutValue);
        Mat img_proc = Binaryimg.clone();
        roi[i] = img_proc(input[i]); // ith roi
        __ok[i].pt = this;
        __ok[i].index = i;
        __ok[i].isnew = isnew;
        threadflag[i] = 0;
        pthread_create(&tid[i],NULL,NumberPredict::HandWritingProcess,&__ok[i]);
        pthread_join(tid[i], NULL);
    }
    for(int i=0;i<inputsize;i++){
        if(threadflag[i] == 0) i = 0;
    }

    for(int i=0;i<inputsize;i++){
      Judgedata.push_back(make_pair(0,result_from_thread[i]));
      result.push_back(result_from_thread[i]);
    }

    // cout digits
    /*cout<<"digits:"<<endl;
    cout<<result[0]<<" "<<result[1]<<" "<<result[2]<<endl;
    cout<<result[3]<<" "<<result[4]<<" "<<result[5]<<endl;
    cout<<result[6]<<" "<<result[7]<<" "<<result[8]<<endl;
    */
    for(int i=0;i<input.size();i++){ // print
        stringstream dig;
        dig<<result[i];
        putText(img, dig.str(), Position[i], CV_FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0,0,255));
    }

}

}
