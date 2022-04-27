
#include <stdint.h>
#include <stdio.h>
//#include "global_settings.h"
#include "bmpTogcode.h"
#include "bmp_io.h"

void greyscale_image(struct image_struct* image_data){
   uint16_t pix_ave;
   for(int y=0;y<image_data->height;y++){
      for(int x=0;x<image_data->width;x++){
         pix_ave = 0;
         for(int c=0;c<image_data->colors_per_pixel;c++){
            pix_ave = pix_ave + image_data->pix_data[y][x][c];
         }
         pix_ave = pix_ave / image_data->colors_per_pixel;
         for(int c=0;c<image_data->colors_per_pixel;c++){
            image_data->pix_data[y][x][c] = pix_ave;
         }
      }
   }
}

void invert_image(struct image_struct* image_data){
   for(int y=0;y<image_data->height;y++){
      for(int x=0;x<image_data->width;x++){
         for(int c=0;c<image_data->colors_per_pixel;c++){
            image_data->pix_data[y][x][c] = image_data->max_bit_depth_val - image_data->pix_data[y][x][c];
         }
      }
   }
}

//void flip_image_vertical(struct image_struct* image_data){
//   uint16_t temp_data[image_data->height][image_data->width][image_data->colors_per_pixel];
//   //uint16_t temp_data[50][50][50];
//   for(int y=0;y<image_data->height;y++){
//      for(int x=0;x<image_data->width;x++){
//         for(int c=0;c<image_data->colors_per_pixel;c++){
//            temp_data[y][x][c] = image_data->pix_data[image_data->height - 1 - y][x][c];
//         }
//      }
//   }
//   for(int y=0;y<image_data->height;y++){
//      for(int x=0;x<image_data->width;x++){
//         for(int c=0;c<image_data->colors_per_pixel;c++){
//            image_data->pix_data[y][x][c] = temp_data[y][x][c];
//         }
//      }
//   }
//}

void copy_image(struct image_struct* image_src, struct image_struct* image_dst){
   for(int y=0;y<image_dst->height;y++){
      for(int x=0;x<image_dst->width;x++){
         for(int c=0;c<image_dst->colors_per_pixel;c++){
            image_dst->pix_data[y][x][c] = image_src->pix_data[y][x][c];
         }
      }
   }
}
