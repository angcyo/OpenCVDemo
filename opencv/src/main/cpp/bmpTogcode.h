#include "opencv2/opencv.hpp"
#include <vector>
using namespace cv;
using namespace std;
#ifndef SRC_MAIN_H
#define SRC_MAIN_H

#define MAX_STRING 256

struct program_struct{
   uint8_t done;
   uint8_t processing;
   uint8_t image_loaded;
   char command[MAX_STRING];
};

//bool rect_rank_x(vector<Rect> &vec_rects, vector<vector<int>> &combine_list);
//bool isOverlap(const Rect &rc1, const Rect &rc2);
void boundRect(struct bound_struct &bound, Mat &srcImage);
int bmpTogcodeFromPath(char *inputPath, char *outputPath);
int bmpTogcodeFromPath(const char *inputPath,const char *outputPath,double scale_out, double xy_mm_step, uint16_t angle_mode, double anglec);
#endif // SRC_MAIN_H
