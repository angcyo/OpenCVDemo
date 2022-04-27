
#ifndef SRC_GCODE_H
#define SRC_GCODE_H

#include <stdint.h>
#include "bmp_io.h"
#include "opencv2/opencv.hpp"

using namespace cv;

#define MAX_GCODE_LINE 256

enum gcode_states {
   START_IMAGE_PASS, MOVE_XY_MIN, SEARCH_X, MOVE_XY_MAX, SEARCH_Y, END_IMAGE_PASS
};

struct gcode_struct {
   FILE* fp;
   double mm_per_pixel;
   int32_t xy_speed;
   double xy_mm_step = 0.5;
   uint8_t move_pos;
   uint8_t cutting;
   double x_pos;
   double y_pos;
   int32_t current_pix_height;
   int32_t x_pix;
   int32_t y_pix;
   double x_offset;
   double y_offset;
   enum gcode_states state;

   //ZJQ
   int nowstate;
   uint16_t angle_mode;
   double yx_ratio;
   double yx_cos;
   double yx_sin;
   int scan_mode;
   double scale_out;
   int draw_complete = 0;
   int scan_end_sign = 1;
};


void create_gcode_file(struct gcode_struct* gcode, char* gcode_file_path);
void close_gcode_file(struct gcode_struct* gcode);
void bmp_to_gcode(struct image_struct* image, struct gcode_struct* gcode, struct bound_struct* bound);
void bound_to_gcode(struct gcode_struct* gcode, vector<vector< Point>> &contours, vector<int> &combine_list);
void move_to_position(struct gcode_struct* gcode);
void set_new_pos(struct gcode_struct* gcode, uint8_t add, Rect boundRect);
void set_new_x(struct gcode_struct* gcode, uint8_t add);
void set_new_y(struct gcode_struct* gcode, uint8_t add);
void check_pixel_height(struct gcode_struct* gcode, struct image_struct* image, Rect rect);
uint8_t check_pixel_cutting(struct gcode_struct* gcode, struct image_struct* image);


#endif //SRC_GCODE_H
