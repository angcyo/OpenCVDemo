#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "bmp_io.h"

#define BMP_HEADER_LENGTH 54

uint8_t mat2image(Mat *srcImage, struct image_struct* image_data)
{
	for (int v = 0; v < srcImage->rows; v++)
	{
		for (int h = 0; h < srcImage->cols; h++)
		{
			for (int c = 0; c < 3; c++)
			{
				image_data->pix_data[v][h][c] = srcImage->at<Vec3b>(v, h)[c];
			}
		}
	}
	image_data->width = srcImage->cols;
	image_data->height = srcImage->rows;
	image_data->bit_depth = srcImage->elemSize() * 8 / 3;
	image_data->colors_per_pixel = srcImage->channels();
	image_data->max_bit_depth_val = 1;
	for (int i = 1; i < image_data->bit_depth; i++) {
		image_data->max_bit_depth_val = (image_data->max_bit_depth_val << 1) + 1;
	}
	return 0;
}

uint8_t read_bmp(struct image_struct* image_data, char* image_file_path){

   FILE* image_fp;

   image_fp = fopen(image_file_path, "rb");
   if(image_fp == 0){
      printf("Couldn't open read file\n");
      return -1;
   } else {
      printf("Successfully opened read file\n");
   }

   if(read_bmp_header(image_data, image_fp)){
      printf("Error Reading Header\n");
      return -1;
   } else {
      printf("Successfully Read Header\n");
   }

   switch(image_data->header.compression){
      case 0 : {
         image_data->colors_per_pixel = 3;
      }
      default : {
         image_data->colors_per_pixel = 3;
         break;
      }
   }

   image_data->width = image_data->header.image_width;
   image_data->height = image_data->header.image_height;
   image_data->bit_depth = image_data->header.bpp/image_data->colors_per_pixel;

   image_data->max_bit_depth_val = 1;
   for(int i=1;i<image_data->bit_depth;i++){
      image_data->max_bit_depth_val = (image_data->max_bit_depth_val << 1) + 1;
   }

   if(read_bmp_data(image_data, image_fp)){
      printf("Error Reading Data\n");
      return -1;
   } else {
      printf("Successfully Read Data\n");
   }

   fclose(image_fp);

   return 0;
}

uint8_t write_bmp(struct image_struct* image_data, char* image_file_path){
   FILE* image_fp;

   image_fp = fopen(image_file_path, "wb+");
   if(image_fp == 0){
      printf("Couldn't open write file\n");
      return -1;
   } else {
      printf("Successfully opened write file\n");
   }

   if(write_bmp_header(image_data, image_fp)){
      printf("Error Writing Header\n");
      return -1;
   } else {
      printf("Successfully Wrote Header\n");
   }

   if(write_bmp_data(image_data, image_fp)){
      printf("Error Writing Data\n");
      return -1;
   } else {
      printf("Successfully Wrote Data\n");
   }

   fclose(image_fp);
   
   return 0;
}

uint8_t read_bmp_header(struct image_struct* image_data, FILE* image_fp){
   uint8_t header_counter = 0;
   char header_data[BMP_HEADER_LENGTH];

   for(header_counter=0;header_counter<BMP_HEADER_LENGTH;header_counter++){
      if(feof(image_fp)){
         printf("Reached End of File Early...\n");
         return -1;
      } else {
         header_data[header_counter] = fgetc(image_fp);
      }
   }
   
   memcpy(&image_data->header.id, &header_data[0], 2);
   memcpy(&image_data->header.file_size, &header_data[2], 4);
   memcpy(&image_data->header.reserved_0, &header_data[6], 4);
   memcpy(&image_data->header.image_offset, &header_data[10], 4);
   memcpy(&image_data->header.header_size, &header_data[14], 4);
   memcpy(&image_data->header.image_width, &header_data[18], 4);
   memcpy(&image_data->header.image_height, &header_data[22], 4);
   memcpy(&image_data->header.color_planes, &header_data[26], 2);
   memcpy(&image_data->header.bpp, &header_data[28], 2);
   memcpy(&image_data->header.compression, &header_data[30], 4);
   memcpy(&image_data->header.image_size, &header_data[34], 4);
   memcpy(&image_data->header.h_resolution, &header_data[38], 4);
   memcpy(&image_data->header.v_resolution, &header_data[42], 4);
   memcpy(&image_data->header.colors, &header_data[46], 4);
   memcpy(&image_data->header.important_colors, &header_data[50], 4);

   if(strcmp(image_data->header.id, "BM")){
      printf("Incorrect Header ID\n");
      return -1;
   }
   if(!image_data->header.image_width || !image_data->header.image_height){
      printf("Invalid Image Size\n");
      return -1;
   }
   
   return 0;
}

uint8_t write_bmp_header(struct image_struct* image_data, FILE* image_fp){
   uint8_t header_counter = 0;
   char header_data[BMP_HEADER_LENGTH];
   
   memcpy(&header_data[0], &image_data->header.id, 2);
   memcpy(&header_data[2], &image_data->header.file_size, 4);
   memcpy(&header_data[6], &image_data->header.reserved_0, 4);
   memcpy(&header_data[10], &image_data->header.image_offset, 4);
   memcpy(&header_data[14], &image_data->header.header_size, 4);
   memcpy(&header_data[18], &image_data->header.image_width, 4);
   memcpy(&header_data[22], &image_data->header.image_height, 4);
   memcpy(&header_data[26], &image_data->header.color_planes, 2);
   memcpy(&header_data[28], &image_data->header.bpp, 2);
   memcpy(&header_data[30], &image_data->header.compression, 4);
   memcpy(&header_data[34], &image_data->header.image_size, 4);
   memcpy(&header_data[38], &image_data->header.h_resolution, 4);
   memcpy(&header_data[42], &image_data->header.v_resolution, 4);
   memcpy(&header_data[46], &image_data->header.colors, 4);
   memcpy(&header_data[50], &image_data->header.important_colors, 4);
   
   for(header_counter=0;header_counter<BMP_HEADER_LENGTH;header_counter++){
      fputc(header_data[header_counter], image_fp);
   }

   return 0;
}

uint8_t read_bmp_data(struct image_struct* image_data, FILE* image_fp/*, uint16_t angle_mode*/){
	uint64_t byte_counter = 0;
	for (int v = 0; v < image_data->header.image_height; v++) {
		for (int h = 0; h < image_data->header.image_width; h++) {
			for (int c = 0; c < 3; c++) {
				if (feof(image_fp)) {
					printf("Reached End of File Early...\n");
					return -1;
				}
				else {
					image_data->pix_data[image_data->height - 1 - v][h][c] = (uint16_t)fgetc(image_fp);
					byte_counter = byte_counter + 1;
				}
			}
		}
		while ((byte_counter % 4) > 0) {
			fgetc(image_fp);
			byte_counter = byte_counter + 1;
		}
	}
   
   return 0;
}

uint8_t write_bmp_data(struct image_struct* image_data, FILE* image_fp){
   uint64_t byte_counter = 0;
   uint8_t fill = 0;

   for(int v=0;v<image_data->header.image_height;v++){
      for(int h=0;h<image_data->header.image_width;h++){
         for(int c=0;c<3;c++){
            fputc(image_data->pix_data[image_data->height - 1 - v][h][c], image_fp);
            byte_counter = byte_counter + 1;
         }
      }
      while((byte_counter % 4) > 0){
         fputc(fill, image_fp);
         byte_counter = byte_counter + 1;
      }
   }

   return 0;
}

void init_bmp_image(struct image_struct* image_data, uint32_t h_res, uint32_t v_res, uint8_t bit_depth, uint8_t colors_per_pixel){
   strcpy(image_data->header.id, "BM");
   image_data->width = h_res;
   image_data->height = v_res;
   image_data->header.header_size = 40;
   image_data->header.image_offset = 54;
   image_data->bit_depth = bit_depth;
   image_data->colors_per_pixel = colors_per_pixel;
   image_data->header.bpp = image_data->bit_depth*colors_per_pixel;
   image_data->header.image_height = v_res;
   image_data->header.image_width = h_res;
   image_data->header.image_size = ((((image_data->header.bpp*h_res)/32) + (((image_data->header.bpp*h_res)%32) > 0))*4 * v_res);
   image_data->header.file_size = image_data->header.image_size + 54;
   image_data->header.colors = 0;
   image_data->header.compression = 0;
   image_data->header.color_planes = 1;
   image_data->header.h_resolution = 0;
   image_data->header.v_resolution = 0;
   image_data->header.reserved_0 = 0;
   image_data->header.important_colors = 0;

   image_data->max_bit_depth_val = 1;
   for(int i=1;i<image_data->bit_depth;i++){
      image_data->max_bit_depth_val = (image_data->max_bit_depth_val << 1) + 1;
   }

}

void image_crop(struct image_struct* image_data, uint32_t h_res, uint32_t v_res, uint32_t h_off, uint32_t v_off){
   uint32_t h_res_safe, v_res_safe;
   uint32_t h_count, v_count;
   uint8_t invalid_image = 0;
   h_count = 0;
   v_count = 0;

   if(h_off > image_data->width){
      invalid_image = 1;
   } else {
      if(h_res > (image_data->width - h_off)){
         h_res_safe = (image_data->width - h_off);
      } else {
         h_res_safe = h_res;
      }
   }

   if(v_off > image_data->height){
      invalid_image = 1;
   } else {
      if(v_res > (image_data->height - v_off)){
         v_res_safe = (image_data->height - v_off);
      } else {
         v_res_safe = v_res;
      }
   }

   if(invalid_image){
      init_bmp_image(image_data, 0, 0, image_data->bit_depth, image_data->colors_per_pixel);
   } else {
      for(int y=0;y<image_data->height;y++){
         h_count = 0;
         for(int x=0;x<image_data->width;x++){
            if((x >= h_off) && (x < (h_off + h_res_safe))){
               if((y >= v_off) && (y < (v_off + v_res_safe))){
                  for(int c=0;c<3;c++){
                     image_data->pix_data[v_count][h_count][c] = image_data->pix_data[y][x][c];
                  }
                  h_count = h_count + 1;
               }
            }
         }
         if((y >= v_off) && (y < (v_off + v_res_safe))){
            v_count = v_count + 1;
         }
      }
      init_bmp_image(image_data, h_res_safe, v_res_safe, image_data->bit_depth, image_data->colors_per_pixel);
   }
}

void image_size_position(struct image_struct* image_data, uint32_t h_res, uint32_t v_res, uint32_t h_off, uint32_t v_off){
   
}