#ifndef __sudoku__InitParam__
#define __sudoku__InitParam__

#include "include/Header.h"
using namespace std;
using namespace cv;

void InitParam(InitParams &params,const FileStorage& fs);
int SaverParam(const string &fs);

#endif /* defined(__sudoku__InitParam__) */
