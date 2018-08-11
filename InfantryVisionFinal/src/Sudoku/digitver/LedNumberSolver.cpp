#include "include/Sudoku/digitver/LedNumberSolver.h"

namespace sudoku{
    
void LedNumberSolver::ParamsFresh(const InitParams &params){
	debug_flag = params.debug_flag;
} 
/**
  * @brief predict the number of the led board
  * @param input    : the region of each number on the led board
  * @param ForBinary: the binary image input
  * @param img      : output result on this image
  * @param thres    : the threshold value for the led numbers
  * @return none
  */
void LedNumberSolver::process(vector<Rect> input,const Mat ForBinary,Mat &img,int thres){
    ledpics.clear();
    size_t size = input.size();
    if(size != 5){
      return;
    }
    inputcopy = ForBinary.clone();
    result.clear();
    int val;
    //cout<<"led:";
    for(int i=0;i<input.size();i++){
        if(input[i].height > 2.0*input[i].width){
            val = 1;
        }else{
            HOGDescriptor *hog = new cv::HOGDescriptor(cvSize(28, 28), cvSize(14, 14), cvSize(7, 7), cvSize(7, 7), 9);
            vector<float>descriptors;
            roi = inputcopy(input[i]);
            threshold(roi,roi,thres,255,THRESH_BINARY_INV);
            //threshold(roi,roi,0,255,THRESH_BINARY_INV+THRESH_OTSU);//+THRESH_OTSU);
            resize(roi,roi,Size(20,20));
            Mat inner = Mat::ones(28,28,CV_8UC1)+254;
            roi.copyTo(inner(Rect(4,4,20,20)));
            if(debug_flag){
                ledpics.push_back(inner);
                ostringstream name;
                name<<"led";
                name<<i+1;
                imshow(name.str(),inner);
            }
            hog->compute(inner,descriptors, cv::Size(1, 1), cv::Size(0, 0));
            Mat SVMPredictMat = Mat(1, (int)descriptors.size(), CV_32FC1);
            memcpy(SVMPredictMat.data,descriptors.data(),descriptors.size()*sizeof(float));
            val = svm->predict(SVMPredictMat);
            delete hog;
            hog = nullptr;
        }
        if(debug_flag){
            stringstream value;
            value<<val;
            Point OutValue;
            OutValue.x = input[i].tl().x+5;
            OutValue.y = input[i].tl().y-5;
            putText(img, value.str(), OutValue, CV_FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255,0,255));
        }
        result.push_back(val);
        //cout led digits
        //cout<<val<<" ";
    }
    //cout<<endl;
}

}
