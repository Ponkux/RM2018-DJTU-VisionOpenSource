#include "include/Sudoku/InitParam.h"
#include "include/Sudoku/TString.h"

/**
  * @brief initialize parameters of the program
  */
void InitParam(InitParams &params,const FileStorage& fs){
    fs["armor_thres_red"]>>params.armor_thres_red;
    fs["armor_thres_blue"]>>params.armor_thres_blue;
    fs["armor_exp_red"]>>params.armor_exp_red;
    fs["armor_exp_blue"]>>params.armor_exp_blue;
    fs["sudoku_exp"]>>params.sudoku_exp;
    fs["sudokunew_exp"]>>params.sudokunew_exp;
    fs["sudoku_led_thres"]>>params.sudoku_led_thres;
    fs["sudokunew_led_thres"]>>params.sudokunew_led_thres;
    fs["sudokunew_board_thres"]>>params.sudokunew_board_thres;
    fs["bulletadd_exp"]>>params.bulletadd_exp;
    fs["bullet_thres"]>>params.bullet_thres;
    fs["x"]>>params.x;
    fs["y"]>>params.y;
    fs["z"]>>params.z;
    fs["debug_flag"]>>params.debug_flag;
    fs["debug_mode"]>>params.debug_mode;
    fs["writemov"]>>params.writemov;
    fs["usevideo"]>>params.usevideo;
    fs["video_path"]>>params.video_path;

    fs["mono_small_armor_720p_const"]>>params.mono_small_armor_720p_const;
    fs["mono_small_armor_1080p_const"]>>params.mono_small_armor_1080p_const;
    fs["mono_big_armor_720p_const"]>>params.mono_big_armor_720p_const;
    fs["mono_big_armor_1080p_const"]>>params.mono_big_armor_1080p_const;
    fs["mono_f_720p"]>>params.mono_f_720p;
    fs["mono_f_1080p"]>>params.mono_f_1080p;
    fs["mono_base_720p"]>>params.mono_base_720p;
    fs["mono_base_1080p"]>>params.mono_base_1080p;
    fs["mono_x"]>>params.mono_x;
    fs["mono_y"]>>params.mono_y;
    fs["mono_z"]>>params.mono_z;
} 

int SaverParam(const string &fs){
    int returnvalue;
    FileStorage file(fs,FileStorage::READ);
    file["number"]>>returnvalue;
    file.release();
    FileStorage file1(fs,FileStorage::WRITE);
    file1<<"number"<<(returnvalue + 1);
    file1.release();
    return returnvalue;
}

