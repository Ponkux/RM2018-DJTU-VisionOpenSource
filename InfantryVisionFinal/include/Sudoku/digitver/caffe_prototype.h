#ifndef _SUDOKU_CAFFE_PROTOTYPE_
#define _SUDOKU_CAFFE_PROTOTYPE_

#include "include/Header.h"
#define CPU_ONLY

/*-------------------caffe prototype---------------*/



/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;

class Classifier {
public:
    Classifier(){};
    void Initialize(const string& model_file,
               const string& trained_file,
               const string& mean_file,
               const string& label_file);
    
    std::vector<Prediction> Classify(const cv::Mat& img, int N = 5);
    
private:
    void SetMean(const string& mean_file);
    
    std::vector<float> Predict(const cv::Mat& img);
    
    void WrapInputLayer(std::vector<cv::Mat>* input_channels);
    
    void Preprocess(const cv::Mat& img,
                    std::vector<cv::Mat>* input_channels);
    
private:
    boost::shared_ptr<Net<float> > net_;
    cv::Size input_geometry_;
    int num_channels_;
    cv::Mat mean_;
    std::vector<string> labels_;
};



/*-------------------caffe prototype---------------*/
#endif
