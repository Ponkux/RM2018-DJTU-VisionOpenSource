#ifndef __Stereo_xml_use__StereoXML__
#define __Stereo_xml_use__StereoXML__

#include "include/Header.h"

using namespace cv;

typedef struct{
    float x;
    float y;
    float z;
    int index;
}AbsPosition;

typedef struct{
    int height;
    int width;
    float distance_const;
    float f;
    float lightbase;
}CalibParam;

class StereoXML{
public:
    StereoXML();
    StereoXML(const string &xmlfile);
    void Init(const string &xmlfile);
    void Calculation(const Point &left,const Point &right,AbsPosition &Position);
private:
    CalibParam Param;
    int VisionDis;
};


#endif /* defined(__Stereo_xml_use__StereoXML__) */
