#include "sudokuposcal.h"

SudokuPosCal::SudokuPosCal()
{

}

SudokuPosCal::SudokuPosCal(const string &file,const InitParams &params){
    FirstCal.Init(file);
    cam_x = params.x;
    cam_y = params.y;
    cam_z = params.z;
}
/**
  * @brief calculate the position of the shoot target of sudoku board
  * @param L/R: left and right target position
  * @param robotlevel: the level of infantry robot(0,1,2,3)
  * @return none
  */
void SudokuPosCal::cal(const Point &L,const Point &R,int robotlevel){
    float shootspeed;
    switch(robotlevel){
        case 0:
        case 1:shootspeed = 20.0;break;
        case 2:shootspeed = 24.0;break;
        case 3:shootspeed = 28.0;break;
        default:shootspeed = 20.0;break;
    }
    FirstCal.Calculation(L,R,sudokupos);
    sudokupos.x -= cam_x;
    sudokupos.y -= cam_y;
    sudokupos.z -= cam_z;
    float shoot_distance = sqrtf(sudokupos.x * sudokupos.x + sudokupos.y * sudokupos.y + sudokupos.z * sudokupos.z)*0.001; // m
    float flytime = shoot_distance / shootspeed;
    float gravity_offset = 4.905 * flytime * flytime;
    //std::cout<<"x:"<<sudokupos.x<<"y:"<<sudokupos.y<<"z:"<<sudokupos.z<<std::endl;
    Data.yaw_angle.d = atan(sudokupos.x/sudokupos.z)*1303.7972938;
    if(fabs(Data.yaw_angle.d) > 500.0) Data.yaw_angle.d = 0;
    Data.pitch_angle.d = -atan((sudokupos.y + gravity_offset*1000.0)/sudokupos.z)*1303.7972938;
    if(fabs(Data.pitch_angle.d) > 500.0) Data.pitch_angle.d = 0;
}

