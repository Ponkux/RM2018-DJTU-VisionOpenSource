#include "include/Header.h"

#include "include/ArmorFind/armorfind.h"
#include "include/ArmorFind/armorpredict.h"

#include "include/Camera/RMVideoCapture.hpp"
#include "include/Camera/StereoXML.h"

#include "include/Serial/serial.h"

#include "include/Sudoku/digitver/caffe_prototype.h"
#include "include/Sudoku/digitver/HandWritingSolver.h"
#include "include/Sudoku/digitver/ImageProcess.h"
#include "include/Sudoku/digitver/LedNumberSolver.h"
#include "include/Sudoku/digitver/LogicProcess.h"
#include "include/Sudoku/InitParam.h"
#include "include/Sudoku/TString.h"
#include "sudokuposcal.h"

#include "include/AddBullet/BulletVision.h"

#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <signal.h>

#include "include/WatchDog.h"

using namespace cv;

VideoWriter IOSaver;
int picsave = 0;

CarData InfantryInput;

std::pair<int,int> FinalResult;

const string model_file   = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/caffemodels/lenet.prototxt";
const string trained_file = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/caffemodels/mynet_iter_20000.caffemodel";
const string trained_18   = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/caffemodels/net18_iter_20000.caffemodel";
const string mean_file    = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/caffemodels/mean.binaryproto";
const string mean_file18  = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/caffemodels/mean18.binaryproto";
const string label_file   = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/caffemodels/synset_words.txt";
const string low_trained  = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/caffemodels/lowrate/mynet_iter_1000.caffemodel";
const string ParamFile    = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/Params.xml";
const string svmfile      = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/svm/svm_digit.xml";
const string Cam640file   = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/Camera/stereo_calib_640_480.xml";
const string Cam1280file  = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/Camera/stereo_calib_1280_720.xml";
const string Cam1920file  = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/Camera/stereo_calib_1920_1080.xml";
const string saverxml     = "/home/ubuntu/Desktop/InfantryVisionFinal/ExtraFiles/writer.xml";
const string saverpath    = "/home/ubuntu/Desktop/InfantryVisionFinal/debugmov/";
Point SLeftTarget,SRightTarget;

Mat contourThreadkernel = getStructuringElement(MORPH_ELLIPSE,Size(9,9));

void ContourThread(int mode,Mat input,Mat &binary,int thres,vector<vector<Point> > &contours){
    Mat thres_whole;//,inputafter;
    vector<Mat> splited;
    //input.convertTo(inputafter,input.type(),1.5,-200);
    split(input,splited);
    cvtColor(input,thres_whole,CV_BGR2GRAY);

    threshold(thres_whole,thres_whole,100,255,THRESH_BINARY);
    if(mode == 0){
        subtract(splited[2],splited[0],binary);
        threshold(binary,binary,thres,255,THRESH_BINARY);// red
    }else{
        subtract(splited[0],splited[2],binary);
        threshold(binary,binary,thres,255,THRESH_BINARY);// blue
    }
    dilate(binary,binary,contourThreadkernel);
    binary = binary & thres_whole;
    findContours(binary,contours,CV_RETR_TREE,CV_CHAIN_APPROX_SIMPLE);
}

void GetImageThread(RMVideoCapture &cap,Mat &out){
    cap>>out;
}

sudoku::ImageProcess sudokusolve_L,sudokusolve_R;

void sudoku_L_Thread(Mat &out,int procflag,int isnew){
    //sudokusolve_L.process(out,procflag,isnew);
}

void sudoku_R_Thread(Mat &out,int procflag,int isnew){
    //sudokusolve_R.process(out,procflag,isnew);
}

int main()
{
    WatchDog dog("/home/ubuntu/Desktop/InfantryVisionFinal/watchdog_fifo");
    dog.feed();
    FileStorage Params(ParamFile, FileStorage::READ);
    InitParams params;
    InitParam(params,Params);
    Params.release();
    sudoku::NumberPredict HandWritingSolver(model_file,trained_file,mean_file,label_file,low_trained);
    sudoku::NumberPredict HandWritingSolver18(model_file,trained_18,mean_file18,label_file,low_trained);
    sudoku::ImageProcess sudokusolve_L(params),sudokusolve_R(params); // for upper contours : for whole classifier  -- new
    sudoku::LedNumberSolver ledsolver(svmfile,params);
    sudoku::LogicProcess logic;
    sudoku::ImgThreadOutput sudoku_L_output,sudoku_R_output;
    dog.feed();
    if(params.writemov){
        int filenum = SaverParam(saverxml);
        ostringstream camsave;
        camsave<<saverpath<<filenum<<".mp4";
        IOSaver.open(camsave.str(),CV_FOURCC('H','2','6','4'),30,Size(1280,720),true);
    }
    dog.feed();
    int serial_fd;
    InitSerial(serial_fd);
    ArmorFind FinderLeft,FinderRight,FinderUp;
    ArmorPredict Predictor(Cam640file,Cam1280file,Cam1920file,params);
    Predictor.SetAxis(params.x,params.y,params.z);

    SudokuPosCal SPosCalculator(Cam1280file,params);

    BulletVision bulletpredictor;
    bulletpredictor.init(svmfile,params,params.bullet_thres);
    dog.feed();
    // initialize the cameras
    RMVideoCapture cap0("/dev/video0",3);
    if(cap0.fd != -1){
        cap0.setVideoFormat(1920,1080,1);
        cap0.info();
        cap0.setVideoFPS(30);
        cap0.setExposureTime(0,params.armor_exp_red);
        cap0.startStream();
    }else{
        cap0.camnum = -1;
    }
    dog.feed();
    RMVideoCapture cap1("/dev/video1",3);
    if(cap1.fd != -1){
        cap1.setVideoFormat(1920,1080,1);
        cap1.info();
        cap1.setVideoFPS(30);
        cap1.setExposureTime(0,params.armor_exp_red);
        cap1.startStream();
    }else{
        cap1.camnum = -1;
    }
    dog.feed();
    RMVideoCapture cap2("/dev/video2",3);
    if(cap2.fd != -1){
        cap2.setVideoFormat(1920,1080,1);
        cap2.info();
        cap2.setVideoFPS(30);
        cap2.setExposureTime(0,params.armor_exp_red);
        cap2.startStream();
    }else{
        cap2.camnum = -1;
    }
    dog.feed();
    RMVideoCapture capleft,capright,capup;
    bool capstatus[3] = {false,false,false};
    if(cap0.camnum == 3){
        capleft = cap0;
        capstatus[0] = true;
    }else if(cap1.camnum == 3){
        capleft = cap1;
        capstatus[0] = true;
    }else if(cap2.camnum == 3){
        capleft = cap2;
        capstatus[0] = true;
    }else{
        capstatus[0] = false;
    }
    if(cap0.camnum == 4){
        capright = cap0;
        capstatus[1] = true;
    }else if(cap1.camnum == 4){
        capright = cap1;
        capstatus[1] = true;
    }else if(cap2.camnum == 4){
        capright = cap2;
        capstatus[1] = true;
    }else{
        capstatus[1] = false;
    }
    if(cap0.camnum == 1){
        capup = cap0;
        capstatus[2] = true;
    }else if(cap1.camnum == 1){
        capup = cap1;
        capstatus[2] = true;
    }else if(cap2.camnum == 1){
        capup = cap2;
        capstatus[2] = true;
    }else{
        capstatus[2] = false;
    }
    dog.feed();
    Mat left,right,leftout,rightout,leftbinary,rightbinary,imgup,imgupout,imgupbinary;
    double timeall = 0,time = 0;
    int fps = 0;
    int thres_red = params.armor_thres_red;
    int thres_blue = params.armor_thres_blue;
    int exp_now = params.armor_exp_red;
    int vtrans_count = 0;
    int videocount_t = 0,videostop = 0;
    // initialize the debug windows if the program running in the debug mode
    if(params.debug_flag){
        namedWindow("left");
        namedWindow("right");\
        namedWindow("debug");
        namedWindow("1");
        namedWindow("2");
        namedWindow("3");
        namedWindow("4");
        namedWindow("5");
        namedWindow("6");
        namedWindow("7");
        namedWindow("8");
        namedWindow("9");
        createTrackbar("Armor thres red","debug",&thres_red,255);
        createTrackbar("Armor thres blue","debug",&thres_blue,255);
        createTrackbar("armor_exp_red","debug",&params.armor_exp_red,255);
        createTrackbar("armor_exp_blue","debug",&params.armor_exp_blue,255);
        createTrackbar("sudoku_exp","debug",&params.sudoku_exp,255);
        createTrackbar("sudoku_led_thres","debug",&params.sudoku_led_thres,255);
        createTrackbar("sudokunew_exp","debug",&params.sudokunew_exp,255);
        createTrackbar("sudokunew_led_thres","debug",&params.sudokunew_led_thres,255);
        createTrackbar("sudokunew_board_thres","debug",&sudokusolve_L.sudokunew_board_thres,255);
        createTrackbar("sudokunew_board_thres2","debug",&sudokusolve_R.sudokunew_board_thres,255);
        createTrackbar("bullet_exp","debug",&params.bulletadd_exp,255);
        createTrackbar("bullet_thres","debug",&params.bullet_thres,255);

    }
    dog.feed();
    int key = 0;
    int videotype = 1920;
    int pmode,attackmode,lastmode;
    vector<vector<Point> > contours_left,contours_right,contours_up;
    vector<Rect> sudoku_L_saver,sudoku_R_saver;
    vector<Point> ArmorLeftResult,ArmorRightResult,ArmorUpResult;
    auto LinuxReleaseSig = [](int sig){
        IOSaver.release();
        exit(0);
    };
    signal(SIGINT,LinuxReleaseSig);
    signal(SIGTERM,LinuxReleaseSig);
    VideoCapture video_test;
    if(params.debug_flag && params.usevideo){
        video_test.open(params.video_path);
    }
    double starttime = (double)getTickCount();
    double passtime;
    dog.feed();

    while(1){
        dog.feed();
        time = (double)getTickCount();
        GetMode(serial_fd,InfantryInput);
        attackmode = InfantryInput.mode;
        if(attackmode != 50 || attackmode != 51){
            SPosCalculator.Data.which = 0;
            SPosCalculator.Data.pos   = 0;
            logic.result.first = 0;
            logic.result.second = 0;
        }
        if(params.debug_flag){
            attackmode = params.debug_mode;
        }
        if(lastmode != attackmode){
            memset(&Predictor.Data,0,sizeof(VisionData));
            memset(&SPosCalculator.Data,0,sizeof(VisionData));
            lastmode = attackmode;
        }

        switch(attackmode){
            case 0:{
                if(exp_now != params.armor_exp_red){
                    exp_now = params.armor_exp_red;
                    capleft.setExposureTime(0,params.armor_exp_red);
                    capright.setExposureTime(0,params.armor_exp_red);
                    capup.setExposureTime(0,params.armor_exp_red);
                }
                break;
            }
            case 1:{
                if(exp_now != params.armor_exp_blue){
                    exp_now = params.armor_exp_blue;
                    capleft.setExposureTime(0,params.armor_exp_blue);
                    capright.setExposureTime(0,params.armor_exp_blue);
                    capup.setExposureTime(0,params.armor_exp_blue);
                }
                break;
            }
            case 50:{
                if(exp_now != params.sudoku_exp){
                    exp_now = params.sudoku_exp;
                    capleft.setExposureTime(0,params.sudoku_exp);
                    capright.setExposureTime(0,params.sudoku_exp);
                }
                break;
            }
            case 51:{
                if(exp_now != params.sudokunew_exp){
                    exp_now = params.sudokunew_exp;
                    capleft.setExposureTime(0,params.sudokunew_exp);
                    capright.setExposureTime(0,params.sudokunew_exp);
                }
                break;
            }
            case 52:{
                if(exp_now != params.bulletadd_exp){
                    exp_now = params.bulletadd_exp;
                    capleft.setExposureTime(0,params.bulletadd_exp);
                    capright.setExposureTime(0,params.bulletadd_exp);     \
                }
                break;
            }
            case 53:{
                if(exp_now != params.armor_exp_red){
                    exp_now = params.armor_exp_red;
                    capleft.setExposureTime(0,params.armor_exp_red);
                    capright.setExposureTime(0,params.armor_exp_red);
                    capup.setExposureTime(0,params.armor_exp_red);
                }
                break;
            }
            case 54:{
                if(exp_now != params.armor_exp_blue){
                    exp_now = params.armor_exp_blue;
                    capleft.setExposureTime(0,params.armor_exp_blue);
                    capright.setExposureTime(0,params.armor_exp_blue);
                    capup.setExposureTime(0,params.armor_exp_blue);
                }
                break;
            }
        }
        Predictor.Data.mode = InfantryInput.mode;
        if(attackmode < 2 || attackmode == 53 || attackmode == 54){
            if(Predictor.Data.z.f > 2500 && videotype == 1280){
                vtrans_count++;
                if(vtrans_count > 10){
                    videotype = 1920;
                    CamSetMode(capleft,capright,capup,1920,capstatus[2]);
                    vtrans_count = 0;
                    std::cout<<"format:1920"<<std::endl;
                }
            }else if((Predictor.Data.z.f < 2000) && (Predictor.Data.z.f > 1) && (videotype == 1920)){
                vtrans_count++;
                if(vtrans_count > 20){
                    videotype = 1280;
                    CamSetMode(capleft,capright,capup,1280,capstatus[2]);
                    vtrans_count = 0;
                    std::cout<<"format:1280"<<std::endl;
                }
            }else{
                vtrans_count = 0;
            }
        }else{
            if(videotype != 1280){
                videotype = 1280;
                CamSetMode(capleft,capright,capup,1280,capstatus[2]);
                std::cout<<"format:1280"<<std::endl;
            }
        }
        if(params.debug_flag && params.usevideo){
            if(video_test.read(left)){
                right = left.clone();
            }else{
                video_test.set(CV_CAP_PROP_POS_FRAMES,0);
            }
        }else{
            std::thread c1(GetImageThread,std::ref(capleft),std::ref(left));
            std::thread c2(GetImageThread,std::ref(capright),std::ref(right));
            std::thread c3;
            if(capstatus[2] == true && attackmode != 50 && attackmode != 51){
                c3 = std::thread(GetImageThread,std::ref(capup),std::ref(imgup));
            }
            c1.join();
            c2.join();
            if(capstatus[2] == true && attackmode != 50 && attackmode != 51){
                c3.join();
            }
        }
        if(params.writemov){
            passtime = ((double)getTickCount() - starttime) / getTickFrequency();
            //videocount_t++;
            if(passtime < 300.0){
                if(left.rows != 720){
                    Mat matwriter;
                    resize(left,matwriter,Size(1280,720));
                    IOSaver<<matwriter;
                }else{
                    IOSaver<<left;
                }
            }else{
                if(!videostop){
                    videostop = 1;
                    IOSaver.release();
                }
            }
        }
        fps++;
        if(left.rows != 0 && left.cols != 0 && right.rows != 0 && right.cols != 0){
            if(attackmode < 2){
                if(attackmode == 1){
                    std::thread t1(ContourThread,attackmode,left,std::ref(leftbinary),thres_blue,std::ref(contours_left));
                    std::thread t2(ContourThread,attackmode,right,std::ref(rightbinary),thres_blue,std::ref(contours_right));
                    std::thread t3;
                    if(capstatus[2] == true){
                        t3 = std::thread(ContourThread,attackmode,imgup,std::ref(imgupbinary),thres_blue,std::ref(contours_up));
                    }
                    t1.join();
                    t2.join();
                    if(capstatus[2] == true){
                        t3.join();
                    }
                }else{
                    std::thread t1(ContourThread,attackmode,left,std::ref(leftbinary),thres_red,std::ref(contours_left));
                    std::thread t2(ContourThread,attackmode,right,std::ref(rightbinary),thres_red,std::ref(contours_right));
                    std::thread t3;
                    if(capstatus[2] == true){
                        t3 = std::thread(ContourThread,attackmode,imgup,std::ref(imgupbinary),thres_red,std::ref(contours_up));
                    }
                    t1.join();
                    t2.join();
                    if(capstatus[2] == true){
                        t3.join();
                    }
                }
                std::thread af1(&ArmorFind::process,FinderLeft,contours_left,std::ref(left),std::ref(leftout),std::ref(ArmorLeftResult),false);
                std::thread af2(&ArmorFind::process,FinderRight,contours_right,std::ref(right),std::ref(rightout),std::ref(ArmorRightResult),false);
                af1.join();
                af2.join();
                //FinderLeft.process(contours_left,left,leftout,ArmorLeftResult,false);
                //FinderRight.process(contours_right,right,rightout,ArmorRightResult,false);
                Predictor.Predict(ArmorLeftResult,ArmorRightResult,videotype,time,InfantryInput.level);
                //Predictor.Predict(FinderLeft.ArmorCenters,FinderRight.ArmorCenters,videotype,time,InfantryInput.level);
                if(Predictor.Data.pitch_angle.d == 0 && Predictor.Data.yaw_angle.d == 0 && capstatus[2] == true){
                    FinderUp.process(contours_up,imgup,imgupout,ArmorUpResult,true);
                    Predictor.PredictMono(FinderUp.monodata,videotype,time,InfantryInput.level);
                    if(FinderUp.monodata.size() != 0){
                        Predictor.Data.mode = 0x10;
                        circle(imgupout,FinderUp.monodata[0].center,5,Scalar(255,0,0),-1);
                    }
                }
                TransformData(serial_fd,Predictor.Data);
                //TransformTarPos(serial_fd,Predictor.Data);
                //std::cout<<"distance:"<<Predictor.Data.z.f<<std::endl;
                if(params.debug_flag){
                    //std::cout<<"distance:"<<Predictor.Data.z.f<<std::endl;
                    //std::cout<<"y:"<<Predictor.Data.y.f<<" distance:"<<Predictor.Data.z.f<<std::endl;
                    Predictor.ShowResult(leftout,rightout);
                    resize(leftbinary,leftbinary,Size(640,360));
                    resize(rightbinary,rightbinary,Size(640,360));
                    resize(leftout,leftout,Size(640,360));
                    resize(rightout,rightout,Size(640,360));
                    imshow("lthres",leftbinary);
                    imshow("rthres",rightbinary);
                    imshow("left",leftout);
                    imshow("right",rightout);
                    if(capstatus[2] == true){
                        if(!imgupout.empty()){
                            resize(imgupout,imgupout,Size(640,360));
                            imshow("upcam",imgupout);
                        }
                        if(!imgupbinary.empty()){
                            resize(imgupbinary,imgupbinary,Size(640,480));
                            imshow("upthres",imgupbinary);
                        }
                    }
                }
            }else{
                if(attackmode == 50){
                    std::thread s1(&sudoku::ImageProcess::process,sudokusolve_L,std::ref(left),1,0,std::ref(sudoku_L_output));
                    std::thread s2(&sudoku::ImageProcess::process,sudokusolve_R,std::ref(right),0,0,std::ref(sudoku_R_output));
                    s1.join();
                    s2.join();
                    //sudokusolve_L.process(left,1,0);
                    //sudokusolve_R.process(right,0,0);
                    HandWritingSolver.process(0,sudoku_L_output.SudokuArea,sudoku_L_output.graydown,left);
                    ledsolver.process(sudoku_L_output.LedArea,sudoku_L_output.graydown,left,params.sudoku_led_thres);
                    FinalResult = logic.process(ledsolver.result, HandWritingSolver.result);
                    //std::cout<<FinalResult.first<<" "<<FinalResult.second<<std::endl;
                    SPosCalculator.Data.which = FinalResult.first;
                    SPosCalculator.Data.pos   = FinalResult.second;
                    SPosCalculator.Data.mode  = 50;

                    //if(sudokusolve_L.sudokus.size() == 9) sudokustruct_L_saver = sudokusolve_L.sudokus;
                    //if(sudokusolve_R.sudokus.size() == 9) sudokustruct_R_saver = sudokusolve_R.sudokus;
                    if(sudoku_L_output.sudokus.size() == 9) sudoku_L_saver = sudoku_L_output.SudokuArea;
                    if(sudoku_R_output.sudokus.size() == 9) sudoku_R_saver = sudoku_R_output.SudokuArea;
                    if(FinalResult.second != 0){
                        if(sudoku_L_saver.size()>=FinalResult.second && sudoku_R_saver.size()>=FinalResult.second){
                            int sindex = FinalResult.second-1;
                            SLeftTarget.x  = sudoku_L_saver[sindex].tl().x + sudoku_L_saver[sindex].width/2.0;
                            SLeftTarget.y  = sudoku_L_saver[sindex].tl().y + sudoku_L_saver[sindex].height/2.0;
                            SRightTarget.x = sudoku_R_saver[sindex].tl().x + sudoku_R_saver[sindex].width/2.0;
                            SRightTarget.y = sudoku_R_saver[sindex].tl().y + sudoku_R_saver[sindex].height/2.0;
                            //std::cout<<"left:("<<SLeftTarget.x<<","<<SLeftTarget.y<<") right:("<<SRightTarget.x<<","<<SRightTarget.y<<")"<<std::endl;
                        }
                        SPosCalculator.cal(SLeftTarget,SRightTarget,InfantryInput.level);
                        TransformSudoku(serial_fd,SPosCalculator.Data);
                        //std::cout<<"angle:"<<SPosCalculator.Data.pitch_angle.d<<" "<<SPosCalculator.Data.yaw_angle.d<<std::endl;
                    }
                    if(params.debug_flag){
                        if(sudoku_L_output.SudokuArea.size()==9){
                            imshow("1",HandWritingSolver.roi[0]);
                            imshow("2",HandWritingSolver.roi[1]);
                            imshow("3",HandWritingSolver.roi[2]);
                            imshow("4",HandWritingSolver.roi[3]);
                            imshow("5",HandWritingSolver.roi[4]);
                            imshow("6",HandWritingSolver.roi[5]);
                            imshow("7",HandWritingSolver.roi[6]);
                            imshow("8",HandWritingSolver.roi[7]);
                            imshow("9",HandWritingSolver.roi[8]);
                        }
                        logic.DrawTarget(left, sudoku_L_output.SudokuArea);
                        ledsolver.ParamsFresh(params);
                        circle(left,SLeftTarget,4,Scalar(255,0,255),-1);
                        circle(right,SRightTarget,4,Scalar(255,0,255),-1);
                        imshow("left",left);
                        imshow("right",right);
                        //imshow("downr",sudokusolve_R.thresdown);
                        imshow("downl",sudoku_L_output.thresdown);
                        if(sudoku_L_output.LedRoi.rows != 0){
                            imshow("led 1st",sudoku_L_output.LedRoi);
                            //imshow("led 2nd",ledsolver.inputcopy);
                        }
                    }
                }else if(attackmode == 51){
                    Mat linit = left.clone();
                    std::thread s1(&sudoku::ImageProcess::process,sudokusolve_L,std::ref(left),1,1,std::ref(sudoku_L_output));
                    std::thread s2(&sudoku::ImageProcess::process,sudokusolve_R,std::ref(right),0,1,std::ref(sudoku_R_output));
                    s1.join();
                    s2.join();
                    /*Mat im;
                    cvtColor(linit,im,CV_BGR2GRAY);
                    threshold(im,im,0,255,THRESH_OTSU);*/

                    //sudokusolve_L.process(left,1,1);
                    //sudokusolve_R.process(right,0,1);
                    /*std::thread s1(sudoku_L_Thread,std::ref(left),1,1);
                    std::thread s2(sudoku_R_Thread,std::ref(right),0,1);
                    s1.join();
                    s2.join();*/
                    HandWritingSolver18.process(1,sudoku_L_output.SudokuArea,linit,left);
                    ledsolver.process(sudoku_L_output.LedArea,sudoku_L_output.for18led,left,params.sudokunew_led_thres);
                    FinalResult = logic.process(ledsolver.result, HandWritingSolver18.result);
                    //std::cout<<FinalResult.first<<" "<<FinalResult.second<<std::endl;
                    SPosCalculator.Data.which = FinalResult.first;
                    SPosCalculator.Data.pos   = FinalResult.second;
                    SPosCalculator.Data.mode  = 51;
                    if(sudoku_L_output.SudokuArea.size() == 9) sudoku_L_saver = sudoku_L_output.SudokuArea;
                    if(sudoku_R_output.SudokuArea.size() == 9) sudoku_R_saver = sudoku_R_output.SudokuArea;
                    if(FinalResult.second != 0){
                        if(sudoku_L_saver.size()>=FinalResult.second && sudoku_R_saver.size()>=FinalResult.second){
                            int sindex = FinalResult.second-1;
                            SLeftTarget.x  = sudoku_L_saver[sindex].tl().x + sudoku_L_saver[sindex].width/2.0;
                            SLeftTarget.y  = sudoku_L_saver[sindex].tl().y + sudoku_L_saver[sindex].height/2.0;
                            SRightTarget.x = sudoku_R_saver[sindex].tl().x + sudoku_R_saver[sindex].width/2.0;
                            SRightTarget.y = sudoku_R_saver[sindex].tl().y + sudoku_R_saver[sindex].height/2.0;
                            //std::cout<<"left:("<<SLeftTarget.x<<","<<SLeftTarget.y<<") right:("<<SRightTarget.x<<","<<SRightTarget.y<<")"<<std::endl;
                        }
                        SPosCalculator.cal(SLeftTarget,SRightTarget,InfantryInput.level);
                        //std::cout<<SPosCalculator.Data.pitch_angle.d<<" "<<SPosCalculator.Data.yaw_angle.d<<std::endl;
                    }
                    TransformSudoku(serial_fd,SPosCalculator.Data);
                    if(params.debug_flag){
                        //imshow("im",im);
                        logic.DrawTarget(left, sudoku_L_output.SudokuArea);
                        logic.DrawTarget(right, sudoku_R_output.SudokuArea);
                        imshow("left",left);
                        if(sudoku_L_output.LedRoi.rows > 0 && sudoku_L_output.LedRoi.cols > 0){
                            imshow("led 1st",sudoku_L_output.LedRoi);
                        }
                        imshow("l_board",sudoku_L_output.board_binary);
                        imshow("r_board",sudoku_R_output.board_binary);
                        imshow("right",right);
                        //imshow("thresl",sudokusolve_L.thresdown1st);
                        imshow("thresfinall",sudoku_L_output.thresdown);
                        //imshow("thresr",sudokusolve_R.thresdown);
                        //imshow("gray",sudokusolve_L.graydown);
                        if(sudoku_L_output.SudokuArea.size()==9){
                            imshow("1",HandWritingSolver18.roi[0]);
                            imshow("2",HandWritingSolver18.roi[1]);
                            imshow("3",HandWritingSolver18.roi[2]);
                            imshow("4",HandWritingSolver18.roi[3]);
                            imshow("5",HandWritingSolver18.roi[4]);
                            imshow("6",HandWritingSolver18.roi[5]);
                            imshow("7",HandWritingSolver18.roi[6]);
                            imshow("8",HandWritingSolver18.roi[7]);
                            imshow("9",HandWritingSolver18.roi[8]);
                            if(key == 'x'){
                                for(int i=0;i<9;i++){
                                    Mat msaver;
                                    picsave++;
                                    ostringstream ss;
                                    ss<<"/home/ubuntu/Desktop/mnistdata/"<<picsave+70000<<".png";
                                    resize(HandWritingSolver18.roi[i],msaver,Size(28,28));
                                    imwrite(ss.str(),msaver);
                                }
                            }
                            if(key == 'v'){
                                if(ledsolver.ledpics.size() >= 4){
                                    for(int i=0;i<ledsolver.ledpics.size();i++){
                                        Mat ledsaver;
                                        picsave++;
                                        ostringstream ss;
                                        ss<<"/home/ubuntu/Desktop/ledpics/"<<picsave+237<<".png";
                                        ledsaver = ledsolver.ledpics[i];
                                        imwrite(ss.str(),ledsaver);
                                    }
                                }
                            }
                        }
                    }
                }else if(attackmode == 52){
                    bulletpredictor.process(right);
                    if(params.debug_flag){
                        bulletpredictor.ParamFresh(params);
                        //imshow("left",left);
                        imshow("right",right);
                        //imshow("lthres",bulletpredictor.led_binary);
                        imshow("rthres",bulletpredictor.dilatethres);
                    }
                    if(bulletpredictor.BulletCount != 0){
                        /*if(bulletpredictor.BulletCount > bulletpredictor.transnum){
                            bulletpredictor.transnum = bulletpredictor.BulletCount;
                        }*/
                        bulletpredictor.table[bulletpredictor.BulletCount]++;
                        bulletpredictor.transnum = bulletpredictor.findtruesend();
                        TransformBulletNums(serial_fd,(int16_t)bulletpredictor.transnum);
                    }
                }else if(attackmode == 53 && capstatus[2] == true){
                    ContourThread(0,imgup,imgupbinary,thres_red,contours_up);
                    FinderUp.process(contours_up,imgup,imgupout,ArmorUpResult,true);
                    Predictor.PredictMono(FinderUp.monodata,videotype,time,InfantryInput.level);
                    //if(FinderUp.monodata.size() != 0){
                       //Predictor.Data.mode = 0x10;
                        //circle(imgupout,FinderUp.monodata[0].center,5,Scalar(255,0,0),-1);
                    //}
                    TransformData(serial_fd,Predictor.Data);
                    if(params.debug_flag){
                        if(capstatus[2] == true){
                            if(!imgupout.empty()){
                                resize(imgupout,imgupout,Size(640,360));
                                imshow("upcam",imgupout);
                            }
                            if(!imgupbinary.empty()){
                                resize(imgupbinary,imgupbinary,Size(640,480));
                                imshow("upthres",imgupbinary);
                            }
                        }
                    }
                }else if(attackmode == 54 && capstatus[2] == true){
                    ContourThread(1,imgup,imgupbinary,thres_blue,contours_up);
                    FinderUp.process(contours_up,imgup,imgupout,ArmorUpResult,true);
                    Predictor.PredictMono(FinderUp.monodata,videotype,time,InfantryInput.level);
                    //if(FinderUp.monodata.size() != 0){
                        //Predictor.Data.mode = 0x10;
                        //circle(imgupout,FinderUp.monodata[0].center,5,Scalar(255,0,0),-1);
                    //}
                    TransformData(serial_fd,Predictor.Data);
                    if(params.debug_flag){
                        if(capstatus[2] == true){
                            if(!imgupout.empty()){
                                resize(imgupout,imgupout,Size(640,360));
                                imshow("upcam",imgupout);
                            }
                            if(!imgupbinary.empty()){
                                resize(imgupbinary,imgupbinary,Size(640,480));
                                imshow("upthres",imgupbinary);
                            }
                        }
                    }
                }
            }
        }
        if(attackmode != 52){
            if(bulletpredictor.table.size() != 0)bulletpredictor.table.clear();
        }

        key = waitKey(1);
        if(key == 'q') break;
        time = ((double)getTickCount() - time) / getTickFrequency();
        //std::cout<<"Process time:"<<time<<"s"<<std::endl;
        timeall += time;
        if(timeall > 1.0){
            timeall = 0;
            std::cout<<"FPS:"<<fps<<std::endl;
            fps = 0;
        }
    }
    cap0.closeStream();
    cap1.closeStream();
    cap2.closeStream();
    IOSaver.release();
    close(serial_fd);
    return 0;
}
