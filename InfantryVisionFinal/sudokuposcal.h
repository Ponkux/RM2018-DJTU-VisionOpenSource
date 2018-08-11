#ifndef SUDOKUPOSCAL_H
#define SUDOKUPOSCAL_H

#include "include/Header.h"
#include "include/Camera/StereoXML.h"
#include "include/Sudoku/InitParam.h"
#include "include/Serial/serial.h"

using namespace cv;

class SudokuPosCal
{
public:
    SudokuPosCal();
    SudokuPosCal(const string &file,const InitParams &params);
    void cal(const Point &L,const Point &R,int robotlevel);
public:
    VisionData Data;
private:
    AbsPosition sudokupos;
    StereoXML FirstCal;
    const float SudokuHeight[3] = {230.0,450.0,670};
    float cam_x,cam_y,cam_z;
};

#endif // SUDOKUPOSCAL_H
