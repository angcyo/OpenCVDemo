
#ifndef SRC_IMAGE_H
#define SRC_IMAGE_H

#include <stdint.h>
#include "bmp_io.h"

void greyscale_image(struct image_struct* image_data);
void invert_image(struct image_struct* image_data);
//void flip_image_vertical(struct image_struct* image_data);
void copy_image(struct image_struct* image_src, struct image_struct* image_dst);

#endif //SRC_IMAGE_H