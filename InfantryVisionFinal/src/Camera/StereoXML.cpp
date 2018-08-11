#include "include/Camera/StereoXML.h"

/**
  * @brief calculate the world's corrdinate of matching points from left and right camera
  * @param left/right: left and right points
  * @param Position  : the corrdinate output
  * @return none
  */
void StereoXML::Calculation(const Point &left,const Point &right,AbsPosition &Position){
    //std::cout<<"left y position:"<<left.y<<std::endl;
    VisionDis = left.x - right.x;
    Position.z = Param.distance_const/VisionDis;
    Position.x = (left.x - Param.width/2)*Position.z/Param.f;
    Position.y = (Param.lightbase- left.y)*Position.z/Param.f;
}

StereoXML::StereoXML(){

}

StereoXML::StereoXML(const string &xmlfile){
    FileStorage fs(xmlfile,FileStorage::READ);
    fs["height"]>>Param.height;
    fs["width"]>>Param.width;
    fs["distance_const"]>>Param.distance_const;
    fs["f"]>>Param.f;
    fs["lightbase"]>>Param.lightbase;
    fs.release();
}

void StereoXML::Init(const string &xmlfile){
    FileStorage fs(xmlfile,FileStorage::READ);
    fs["height"]>>Param.height;
    fs["width"]>>Param.width;
    fs["distance_const"]>>Param.distance_const;
    fs["f"]>>Param.f;
    fs["lightbase"]>>Param.lightbase;
    fs.release();
}
