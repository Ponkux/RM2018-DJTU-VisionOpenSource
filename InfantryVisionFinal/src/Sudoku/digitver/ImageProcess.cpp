#include "include/Sudoku/digitver/ImageProcess.h"

namespace sudoku{

ImageProcess::ImageProcess(){

}

ImageProcess::ImageProcess(const InitParams &params){
    debug_flag= params.debug_flag;
    sudokunew_board_thres = params.sudokunew_board_thres;
    contours.reserve(2000);
    RegionContours.reserve(2000);
}

Point ImageProcess::get_contour_center(const vector<Point> contour){
    Point center;
    Moments mu;
    mu = moments(contour,false);
    center.x = mu.m10/mu.m00;
    center.y = mu.m01/mu.m00;
    return center;
}
/**
  * @brief get the position of the region of sudoku board and led board
  * @param input   : the image for process
  * @param isledset: input 0 : donot detect led. other:detect led
  * @param isnew   : input 0 : detect the small power rune. other: detect the big power rune
  * @return none
  */
void ImageProcess::process(Mat input,int isledset,int isnew,ImgThreadOutput &out){
    leddoflag = isledset;
    img = input;
    imgdown = img.clone();
    Process(isnew);
    out.graydown = this->graydown;
    out.LedArea = this->LedArea;
    out.SudokuArea = this->SudokuArea;
    out.sudokus = this->sudokus;
    out.for18led = this->for18led;
    out.thresdown = this->thresdown;
    out.LedRoi = this->LedRoi;
    out.board_binary = this->R_thres;
}

void ImageProcess::Process(int isnew){
    ThresholdProcess(isnew);
    ContourProcess(isnew);
}
/**
  * @brief get the region of the big power rune base on the score board's position
  */
bool ImageProcess::GetRegion(){
    ScoreBoards.clear();
    cvtColor(imgdown, R_thres, CV_BGR2GRAY);
    if(leddoflag){
        for18led = R_thres.clone();
    }
    threshold(R_thres, R_thres, sudokunew_board_thres, 255, THRESH_BINARY);
    findContours(R_thres, RegionContours,CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
    int Rsize = RegionContours.size();
    for(int i=0;i<Rsize;i++){
        Rect bound = boundingRect(RegionContours[i]);
        int area = contourArea(RegionContours[i]);
        if(area < 600 || area > 4000) continue;
        vector<Point> approx;
        approxPolyDP(RegionContours[i],approx,10,true);
        if(approx.size() == 4){
            float div = (float)bound.width / (float)bound.height;
            if(div > 1.6 && div < 2.5){
                int innersize = countNonZero(R_thres(bound));
                if((float)innersize / (float) area > 0.9){
                    ScoreBoards.push_back(bound);
                }
            }
        }
    }
    std::sort(ScoreBoards.begin(),ScoreBoards.end(),
                       [](const Rect &a1,const Rect &a2){
                           return a1.x < a2.x;
                       });
    if(ScoreBoards.size() != 10 && ScoreBoards.size() > 0){
        vector<vector<Rect>> Groups;
        vector<Rect> Group;
        Group.push_back(ScoreBoards[0]);
        for(int i=1;i<ScoreBoards.size();i++){
            if(abs(ScoreBoards[i].x - ScoreBoards[i-1].x) < 100){
                Group.push_back(ScoreBoards[i]);
            }else{
                Groups.push_back(Group);
                Group.clear();
                Group.push_back(ScoreBoards[i]);
            }
        }
        Groups.push_back(Group);
        ScoreBoards.clear();
        for(int i=0;i<Groups.size();i++){
            if(Groups[i].size() == 5){
                for(int j=0;j<5;j++){
                    ScoreBoards.push_back(Groups[i][j]);
                }
            }
        }
    }
    if(ScoreBoards.size() == 10){
        vector<Point> region_pts;
        for(int i=0;i<10;i++){
            if(i<5){
                region_pts.push_back(ScoreBoards[i].br());
                region_pts.push_back(Point(ScoreBoards[i].br().x,ScoreBoards[i].br().y - ScoreBoards[i].height));
            }else{
                region_pts.push_back(ScoreBoards[i].tl());
                region_pts.push_back(Point(ScoreBoards[i].tl().x,ScoreBoards[i].tl().y + ScoreBoards[i].height));
            }
            if(debug_flag){
                rectangle(img,ScoreBoards[i],Scalar(255,0,255),2);
            }
        }

        RegionRect = boundingRect(region_pts);
        if(RegionRect.x + RegionRect.width >= 1280){
            RegionRect.width = 1280 - RegionRect.x;
        }
        if(RegionRect.y + RegionRect.height >= 720){
            RegionRect.height = 720 - RegionRect.y;
        }
        if(debug_flag){
            rectangle(img,RegionRect,Scalar(255,0,255),2);
        }
        return true;
    }
    return false;
}

void ImageProcess::ThresholdProcess(int isnew){
    if(!isnew){
        cvtColor(imgdown, graydown, CV_BGR2GRAY);
        threshold(graydown, thresdown, 0, 255, THRESH_OTSU);
    }else{
        RegionDetect = GetRegion();
        if(RegionDetect){
            imgdown = imgdown(RegionRect);
            thresdown1st = R_thres(RegionRect);
        }else{
            GaussianBlur(imgdown,imgdown,Size(7,7),10);
            split(imgdown,splited);
            Mat GR = (splited[1] - splited[0]);
            graydown = ((splited[2] + splited[1]) & (GR.mul(GR)));
            threshold(graydown,thresdown1st,0,255,THRESH_OTSU);
        }
        erode(thresdown1st,thresdown,erodekernel);
        dilate(thresdown,thresdown,kernel);
    }
}

void ImageProcess::ContourProcess(int isnew){
    sudokus.clear();
    SudokuArea.clear();
    Lednums.clear();
    LedArea.clear();
    thresdown.copyTo(contour_nouse);
    findContours(contour_nouse, contours,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    size_t size = contours.size();
    Rect bound;
    float whdiv;
    float innersize;
    size_t bigsize = 0;
    contoursfirst.clear();
    if(size == 0) return;
    if(!isnew){
        for(size_t i=0;i<size;i++){
          bound = boundingRect(contours[i]);
          whdiv = (float)bound.width/(float)bound.height;
          if(whdiv>1.2 && whdiv<3.0 && bound.area() > 3000 && bound.area() < 15000){
            contoursfirst.push_back(std::make_pair(contours[i],bound));
            if(bigsize < bound.area()){
              bigsize = bound.area();
            }
          }
        }
        size = contoursfirst.size();
        bigsize /= 3.5;
        for(size_t i=0;i<size;i++){
              bound = contoursfirst[i].second;
              //float sboundrate = (float)bound.width / (float)bound.height;
              //std::cout<<"boundrate:"<<sboundrate<<std::endl;
              if(bound.area() > bigsize){// && sboundrate > 1.4 && sboundrate < 2.0){
                  innersize = countNonZero(thresdown(bound));
                  if((float)innersize/(float)bound.area()>0.6){
                   // SortSudokus su(bound,get_contour_center(contoursfirst[i].first));
                    SortSudokus su(bound);
                    sudokus.push_back(su);
                    //std::cout<<"area"<<bound.area()<<std::endl;
                  }
              }
         }
    }else{\
        int c_maxsize = 0;
        for(int i=0;i<size;i++){
            bound = boundingRect(contours[i]);
            if(!RegionDetect){
                if(bound.x < 200 || bound.x > 1000) continue;
            }
            float rate = (float)bound.height / (float)bound.width;
            if(rate > 0.7 && rate < 3.5 && bound.area() > 1500 && bound.area() < 6000){
                innersize = ((float)countNonZero(thresdown1st(bound)))/(float)bound.area();
                if(innersize>0.15 && innersize < 0.8){
                    if(c_maxsize < bound.area()) c_maxsize = bound.area();
                }
            }
        }
        c_maxsize /= 2;
        for(int i=0;i<size;i++){
            bound = boundingRect(contours[i]);
            if(!RegionDetect){
                if(bound.x < 200 || bound.x > 1000) continue;
            }
            float rate = (float)bound.height / (float)bound.width;
            //std::cout<<bound.area()<<std::endl;
            if(!RegionDetect && bound.y < 50) continue;
            if(rate > 0.7 && rate < 2.5 && bound.area() > 1500 && bound.area() < 6000 && bound.area() > c_maxsize){
                 innersize = ((float)countNonZero(thresdown1st(bound)))/(float)bound.area();
                 if(innersize>0.15 && innersize < 0.8){
                    if(RegionDetect){
                        bound.x += RegionRect.tl().x;
                        bound.y += RegionRect.tl().y;
                    }
                    SortSudokus su(bound);
                    sudokus.push_back(su);
                 }
                //}
            }
        }
    }
    if(sudokus.size() > 9){
        DeleteRedund();
    }
    if(sudokus.size() < 20){
          shight_max = 0;
          std::sort(sudokus.begin(),sudokus.end());
          if(isnew && sudokus.size() > 5 && sudokus.size() < 9){
            PredictLost();
            //std::sort(sudokus.begin(),sudokus.end());
          }
          int ssize = sudokus.size();
          for(int i=0;i<ssize;i++){
              bound = Rect_<int>(sudokus[i].x, sudokus[i].y, sudokus[i].width, sudokus[i].height);
              //std::cout<<"x:"<<bound.x<<" y:"<<bound.y<<" size:"<<bound.area()<<std::endl;
              SudokuArea.push_back(bound);
              if(bound.height > shight_max) shight_max = bound.height;
              if(debug_flag){
                rectangle(img, bound, Scalar(255,0,255),2);//draw contour
                //std::cout<<"x:"<<bound.x<<" y:"<<bound.y<<std::endl;
              }
              //cout<<bound.tl()<<endl;
          }
          //std::cout<<"pass"<<std::endl;
          ltop = top;
          ltopx = topx;
          ltwidth = twidth;
        if(leddoflag){
          if(SudokuArea.size() == 9){
              top=SudokuArea[0].tl().y;
              topx = SudokuArea[0].tl().x;
              twidth = SudokuArea[2].br().x - topx;
              for(int i=1;i<3;i++){
                if(SudokuArea[i].tl().y < top){
                    top = SudokuArea[i].tl().y;
                }
              }
          }
          if(top > 0 && top < 720 && twidth > 20 && (topx+twidth) < 1280){
            //LedRoi = Mat(thresdown.clone(), Rect_<int>(topx, 0, twidth, top));
          }else{
            top = ltop;
            topx = ltopx;
            twidth = ltwidth;
          }
          if(top > 0 && top < 720 && twidth > 20 && (topx+twidth) < 1280){
              if(!isnew){
                  Ledroi1st = Mat(graydown, Rect_<int>(topx, 0, twidth, top));
              }else{
                  Ledroi1st = Mat(for18led, Rect_<int>(topx, 0, twidth, top)); // here
              }
              threshold(Ledroi1st,LedRoi,0,255,THRESH_OTSU);
              dilate(LedRoi,LedRoi,usekernel);
          }
          if(LedRoi.rows < 20 && LedRoi.cols < 20) return;
          LedRoi.copyTo(RoiBinary_nouse);
          findContours(RoiBinary_nouse, smallcontours,CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
          size = smallcontours.size();
          int boundmax = 0;
          for(size_t i=0;i<size;i++){
              bound = boundingRect(smallcontours[i]);
              bound.x += topx;
              if(!isnew){
                  float sizerate = (float)bound.height/bound.width;
                  float ledhrate = (float)shight_max/bound.height;
                  if(ledhrate > 1.0 && ledhrate < 2.0 && sizerate>1.0){ // 400 1000
                      SortRects Forsort(bound);
                      Lednums.push_back(Forsort);
                      //rectangle(img,bound,Scalar(255,255,255),1);
                  }
              }else{
                  float sizerate = (float)bound.height/bound.width;
                  if(sizerate>1.0){
                      if(bound.area() > boundmax) boundmax = bound.area();
                      SortRects Forsort(bound);
                      Lednums.push_back(Forsort);
                      //rectangle(img,bound,Scalar(255,255,255),1);
                  }
              }
            }
            if(Lednums.size() > 5 ){
                boundmax /=3;
                vector<SortRects>::iterator it;
                for(it = Lednums.begin();it != Lednums.end();){
                    if(((*it).height * (*it).width > boundmax) && ((*it).width < (*it).height)) ++it;
                    else it = Lednums.erase(it);
                }
            }
            //std::cout<<"ledsize:"<<Lednums.size()<<std::endl;
          if(Lednums.size() == 5){
              std::sort(Lednums.begin(),Lednums.end());
              for(int i=0;i<Lednums.size();i++){
                  bound = Rect_<int>(Lednums[i].x, Lednums[i].y, Lednums[i].width, Lednums[i].height);
                  if(debug_flag){
                     rectangle(img, bound, Scalar(255,0,255)); // draw contour
                  }
                  LedArea.push_back(bound);
              }
          }
    }
}

}

//use it only if vector sudokus has been already sorted!
/**
  * @brief predict the lost position of the sudoku board
  */
void ImageProcess::PredictLost(){
    size_t size = sudokus.size();
    int expectnum = 9-size;
    int xmin=INT_MAX,ymin=INT_MAX;
    int linesnow = 0,rowsnow = 0,linesdis,colsdis = INT_MAX;
    int areamax = sudokus[0].height * sudokus[0].width;
    if(size == 6){
        for(int i=0;i<5;i++){
            if(sudokus[i+1].y - sudokus[i].y > 60){
                linesdis = sudokus[i+1].y - sudokus[i].y;
                linesnow++;
            }
            if(sudokus[i+1].x - sudokus[i].x < 0){
                rowsnow++;
            }
        }
    }else{
        for(int i=0;i<size-1;i++){
            if(sudokus[i+1].y - sudokus[i].y > 60){
                linesdis = sudokus[i+1].y - sudokus[i].y;
                break;
            }
        }
        linesnow = 3;
    }

    if(linesnow == 2 || rowsnow == 2){
        return;
    }
    for(int i=0;i<size-1;i++){
        int dis = sudokus[i+1].x - sudokus[i].x;
        int area = sudokus[i+1].width * sudokus[i+1].height;
        if(dis > 20 && dis < colsdis) colsdis = dis;
        areamax = area > areamax ? area : areamax;
        xmin = sudokus[i].x < xmin ? sudokus[i].x : xmin;
        ymin = sudokus[i].y < ymin ? sudokus[i].y : ymin;
    }
    int boundline = sqrt(areamax);
    int xmaxdis = 0.5*colsdis;
    int ymaxdis = 0.5*linesdis;
    int expx,expy;
    for(int i=0;i<size;i++){
        if(expectnum == 0) break;\
        if(i == 0){
            expx = xmin;
            expy = ymin;
        }else{
            if(i%3==0){
                expx = xmin;
                expy = sudokus[i-3].y + linesdis;
            }else{
                expx = sudokus[i-1].x + colsdis;
                expy = sudokus[i-1].y;
            }
            //expx = xmin + (i%3)*colsdis;
            //expy = ymin + (i/3)*linesdis;
        }
        if(expx+boundline > 1280 || expy + boundline > 720) break;
        if(abs(sudokus[i].x - expx) > xmaxdis || abs(sudokus[i].y - expy) > ymaxdis){
            Rect boundinsert = Rect_<int>(expx,expy,boundline,boundline);
            sudokus.insert(sudokus.begin()+i,boundinsert);
            expectnum--;
            size++;
            i=0;
        }
        if(size == 8 && i == 7){
            expx = sudokus[i].x + colsdis;
            expy = sudokus[i].y;
            if(expx+boundline > 1280 || expy + boundline > 720) break;
            Rect boundinsert = Rect_<int>(expx,expy,boundline,boundline);
            sudokus.push_back(boundinsert);
            break;
        }
    }
}
/**
  * @brief delete the redund region from the detection part
  */
void ImageProcess::DeleteRedund(){
    int centerindex,mindis = INT_MAX,dis = 0;
    for(int i=0;i<sudokus.size();i++){
        dis = abs(sudokus[i].x - 640);
        if(mindis > dis){
            mindis = dis;
            centerindex = i;
        }
    }
    int centerx = sudokus[centerindex].x;
    int centery = sudokus[centerindex].y;
    std::vector<std::pair<SortSudokus,int>> Forminus;
    for(int i=0;i<sudokus.size();i++){
        dis = sqrt(pow(centerx - sudokus[i].x,2)+pow(centery - sudokus[i].y,2));
        Forminus.push_back(std::make_pair(sudokus[i],dis));
    }
    sort(Forminus.begin(),Forminus.end(),
         [](const std::pair<SortSudokus,int> &a1,const std::pair<SortSudokus,int> &a2){
        return a1.second < a2.second;
    });
    sudokus.clear();
    for(int i=0;i<9;i++){
        sudokus.push_back(Forminus[i].first);
    }
}

    
}

