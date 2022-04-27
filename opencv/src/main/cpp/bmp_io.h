/* 
 * File:   image_reader.h
 * Author: kmckenzie
 *
 * Created on February 21, 2019, 12:37 PM
 */

#ifndef SRC_BMP_IO_H
#define	SRC_BMP_IO_H

#include <cstdio>
#include <cstdint>
#include <vector>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;
#define MAX_IMAGE_WIDTH 8000
#define MAX_IMAGE_HEIGHT 8000

#define CPP 3
#define BPC 16

struct bmp_header_struct {
   char id[2];                // BM
   uint32_t file_size;        // Total File Size
   uint32_t reserved_0;       // Reserved
   uint32_t image_offset;     // Offset to Start of Pixel Data
   uint32_t header_size;      // Size of BMP Header
   uint32_t image_width;      // Image Width
   uint32_t image_height;     // Image Height
   uint16_t color_planes;     // Number of Color Planes
   uint16_t bpp;              // Bits Per Pixel
   uint32_t compression;      // Compression Method
   uint32_t image_size;       // Image Size (Raw Bitmap Size)
   uint32_t h_resolution;     // Horizontal Resolution
   uint32_t v_resolution;     // Vertical Resolution
   uint32_t colors;           // Number of Colors In Pallete (default 0 is 2^n)
   uint32_t important_colors; // Number of Important Colors Used
};

struct image_struct {
   struct bmp_header_struct header;
   uint16_t pix_data[MAX_IMAGE_HEIGHT][MAX_IMAGE_WIDTH][CPP];
   uint16_t max_bit_depth_val;
   uint8_t loaded;
   uint32_t width;
   uint32_t height;
   uint8_t bit_depth;
   uint8_t colors_per_pixel;
};

struct bound_struct {
	vector<vector<Point>> contours;
	vector<Rect> boundRect;
	vector<vector<int>> combine_list;
};

uint8_t mat2image(Mat *srcImage, struct image_struct* image_data);
uint8_t read_bmp(struct image_struct* image_data, char* image_file_path);
uint8_t write_bmp(struct image_struct* image_data, char* image_file_path);
uint8_t read_bmp_header(struct image_struct* image_data, FILE* image_fp);
uint8_t write_bmp_header(struct image_struct* image_data, FILE* image_fp);
uint8_t read_bmp_data(struct image_struct* image_data, FILE* image_fp);
uint8_t write_bmp_data(struct image_struct* image_data, FILE* image_fp);
void init_bmp_image(struct image_struct* image_data, uint32_t h_res, uint32_t v_res, uint8_t bit_depth, uint8_t colors_per_pixel);
void image_crop(struct image_struct* image_data, uint32_t h_res, uint32_t v_res, uint32_t h_off, uint32_t v_off);
void image_size_position(struct image_struct* image_data, uint32_t h_res, uint32_t v_res, uint32_t h_off, uint32_t v_off);

#endif	/* SRC_BMP_IO_H */