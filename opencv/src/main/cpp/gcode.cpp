
#include <stdint.h>
#include <stdio.h>
//#include "global_settings.h"
#include "bmpTogcode.h"
#include "bmp_io.h"
#include "gcode.h"
#include <math.h>

// need to do more advanced move decision
// can we not move motor over areas that don't need cutting anymore?
// don't move to end of lines when we don't have anything left to cut on the ends...
// look at rest of line and when we won't cut for the rest of the line, go ahead and find first pixel to cut on the next line...
// maybe do line search and find first/last postion of cut (min/max x) and move to those locations?
// same for y when searching y direction. That way we aren't moving all the way over when there's nothing left to cut...

void create_gcode_file(struct gcode_struct* gcode, char* gcode_file_path){
   FILE* gcode_fp;

   gcode_fp = fopen(gcode_file_path, "wb+");
   if(gcode_fp == 0){
      printf("Couldn't open gcode file\n");
   } else {
      printf("Successfully opened gcode file\n");
   }

   gcode->fp = gcode_fp;
}

void close_gcode_file(struct gcode_struct* gcode) {
	int close_state = fclose(gcode->fp);
	if (close_state == 0)
		printf("Successfully closed gcode file\n");
	else
		printf("Couldn't close gcode file\n");
}

void bmp_to_gcode(struct image_struct* image, struct gcode_struct* gcode, struct bound_struct* bound){
   uint8_t processing = 1;
   
   gcode->x_pix = 0;
   gcode->y_pix = 0;
   gcode->xy_speed = 3500;
   gcode->mm_per_pixel = 2 * gcode->scale_out / max(image->width, image->height);//计算像素间距
   gcode->x_offset = gcode->mm_per_pixel * image->width / 2;//偏移至中心
   gcode->y_offset = gcode->mm_per_pixel * image->height / 2;
   gcode->move_pos = 1;
   gcode->x_pos = 0.0;
   gcode->y_pos = 0.0;
   gcode->cutting = 0;
   gcode->current_pix_height = image->max_bit_depth_val;
   gcode->state = START_IMAGE_PASS;

   //ZJQ
   for (int i = 0; i < bound->boundRect.size(); i++)
   {
	   printf("process boundRect %d\n", i);
	   if (i != 0)
	   {
		   gcode->state = MOVE_XY_MIN;
		   processing = 1;
	   }

	   while (processing)
	   {
		   switch (gcode->state) {
		   case START_IMAGE_PASS: {
			   fprintf(gcode->fp, "G21\n"); // set units to mm
			   fprintf(gcode->fp, "G90\n"); // set units to mm

			   //ZJQ
			   fprintf(gcode->fp, "G1 F%d\n", gcode->xy_speed);
			   //bound_to_gcode(gcode, bound->contours, bound->combine_list);
			   gcode->state = MOVE_XY_MIN;
			   break;
		   }
		   case MOVE_XY_MIN: {//打印头移至矩形左上角
			   bound_to_gcode(gcode, bound->contours, bound->combine_list[i]);//首先打印矩形内的轮廓

			   gcode->cutting = 1;
			   gcode->x_pix = bound->boundRect[i].x;
			   gcode->y_pix = bound->boundRect[i].y;
			   gcode->x_pos = ((double)gcode->x_pix) * gcode->mm_per_pixel;
			   gcode->y_pos = ((double)gcode->y_pix) * gcode->mm_per_pixel;
			   gcode->move_pos = 1;
			   //ZJQ
			   fprintf(gcode->fp, "M05 S0\n");

			   gcode->state = SEARCH_X;
			   break;
		   }
		   case SEARCH_X: {
			   gcode->move_pos = 1;//首先从左向右运动
			   while ((gcode->move_pos && gcode->x_pix < (bound->boundRect[i].x + bound->boundRect[i].width - 1)) || (!gcode->move_pos && gcode->y_pix < (bound->boundRect[i].y + bound->boundRect[i].height - 1))) {//按行处理循环
				   if (gcode->move_pos) {
					   double y_pix_double = gcode->y_pix, x_pix_double = gcode->x_pix;
					   while (gcode->x_pix < (bound->boundRect[i].x + bound->boundRect[i].width - 1) && gcode->y_pix >= bound->boundRect[i].y) {//一行中按像素点循环处理
						   gcode->x_pos = (((double)gcode->x_pix) * gcode->mm_per_pixel);
						   gcode->y_pos = max(((double)gcode->y_pix) * gcode->mm_per_pixel, ((double)bound->boundRect[i].y * gcode->mm_per_pixel));
						   check_pixel_height(gcode, image, bound->boundRect[i]);//检查像素点的值情况，判断gcode是否移动到这点
						   image->pix_data[gcode->y_pix][gcode->x_pix][0] = 0;
						   if (gcode->draw_complete == 1)
						   {
							   gcode->draw_complete = 0;
							   break;
						   }
						   if (gcode->yx_ratio <= 1.0)//准备判断下一像素点，针对不同扫描角度，保证一次最大运动一个像素
						   {
							   gcode->x_pix = gcode->x_pix + 1;
							   y_pix_double -= gcode->yx_ratio;
							   gcode->y_pix = gcode->y_pix - (int)(gcode->y_pix - y_pix_double + 0.5);
						   }
						   else
						   {
							   x_pix_double += 1 / gcode->yx_ratio;
							   gcode->x_pix = gcode->x_pix + (int)(x_pix_double - gcode->x_pix + 0.5);
							   gcode->y_pix = gcode->y_pix - 1;
						   }
					   }
					   gcode->x_pos = (((double)gcode->x_pix) * gcode->mm_per_pixel);
					   gcode->y_pos = max(((double)gcode->y_pix) * gcode->mm_per_pixel, ((double)bound->boundRect[i].y * gcode->mm_per_pixel));
					   check_pixel_height(gcode, image, bound->boundRect[i]);
					   image->pix_data[gcode->y_pix][gcode->x_pix][0] = 0;
					   if (gcode->draw_complete == 1)
					   {
						   gcode->draw_complete = 0;
					   }
				   }
				   else {
					   double y_pix_double = gcode->y_pix, x_pix_double = gcode->x_pix;
					   while (gcode->x_pix >= bound->boundRect[i].x && gcode->y_pix < (bound->boundRect[i].y + bound->boundRect[i].height - 1)) {
						   gcode->x_pos = (((double)gcode->x_pix) * gcode->mm_per_pixel);
						   gcode->y_pos = min(((double)gcode->y_pix) * gcode->mm_per_pixel, ((double)(bound->boundRect[i].y + bound->boundRect[i].height) * gcode->mm_per_pixel));
						   check_pixel_height(gcode, image, bound->boundRect[i]);
						   image->pix_data[gcode->y_pix][gcode->x_pix][0] = 0;
						   if (gcode->draw_complete == 1)
						   {
							   gcode->draw_complete = 0;
							   break;
						   }
						   if (gcode->yx_ratio <= 1.0)
						   {
							   gcode->x_pix = gcode->x_pix - 1;
							   y_pix_double += gcode->yx_ratio;
							   gcode->y_pix = gcode->y_pix + (int)(y_pix_double - gcode->y_pix + 0.5);
						   }
						   else
						   {
							   x_pix_double -= 1 / gcode->yx_ratio;
							   gcode->x_pix = gcode->x_pix - (int)(gcode->x_pix - x_pix_double + 0.5);
							   gcode->y_pix = gcode->y_pix + 1;
						   }
					   }
					   gcode->x_pos = (((double)gcode->x_pix) * gcode->mm_per_pixel);
					   gcode->y_pos = min(((double)gcode->y_pix) * gcode->mm_per_pixel, ((double)(bound->boundRect[i].y + bound->boundRect[i].height) * gcode->mm_per_pixel));
					   check_pixel_height(gcode, image, bound->boundRect[i]);
					   image->pix_data[gcode->y_pix][gcode->x_pix][0] = 0;
					   if (gcode->draw_complete == 1)
					   {
						   gcode->draw_complete = 0;
					   }
				   }
				   //check_pixel_height(gcode, image, bound->boundRect[i]);
				   gcode->move_pos = (gcode->move_pos) ? 0 : 1;
				   switch (gcode->scan_mode)//运动到下一行，针对不同扫描方式有不同的移动方法
				   {
				   case(0):
					   set_new_pos(gcode, gcode->move_pos, bound->boundRect[i]);
					   break;
				   case(1):
					   set_new_y(gcode, 1);
					   break;
				   case(2):
					   set_new_x(gcode, 1);
					   break;
				   default:
					   set_new_pos(gcode, gcode->move_pos, bound->boundRect[i]);
					   break;
				   }

			   }
			   /*for (int j = bound->boundRect[i].x; j < bound->boundRect[i].x + bound->boundRect[i].width; j++)
			   {
				   for (int k = bound->boundRect[i].y; k < bound->boundRect[i].y + bound->boundRect[i].height; k++)
					   fprintf(gcode->fp, "%d ", image->pix_data[k][j][0] > 127 ? 1 : 0);
				   fprintf(gcode->fp, "\n");
			   }*/
			   if (gcode->scan_end_sign == 1)//扫描结束则切换状态，否则继续对矩形进行扫描
			   {
				   gcode->state = END_IMAGE_PASS;
			   }
			   else
			   {
				   gcode->cutting = 1;
				   gcode->x_pix = bound->boundRect[i].x;
				   gcode->y_pix = bound->boundRect[i].y;
				   gcode->x_pos = ((double)gcode->x_pix) * gcode->mm_per_pixel;
				   gcode->y_pos = ((double)gcode->y_pix) * gcode->mm_per_pixel;
				   gcode->move_pos = 1;
				   gcode->scan_end_sign = 1;
			   }
			   //gcode->state = END_IMAGE_PASS;
			   break;
		   }
		   case END_IMAGE_PASS: {
			   for (int j = bound->boundRect[i].x; j < bound->boundRect[i].x + bound->boundRect[i].width; j++)
			   {
				   for (int k = bound->boundRect[i].y; k < bound->boundRect[i].y + bound->boundRect[i].height; k++)
					   image->pix_data[k][j][0] = 0;
			   }
			   processing = 0;
			   break;
		   }
		   default: {
			   processing = 0;
			   gcode->state = START_IMAGE_PASS;
			   break;
		   }
		   }
	   }
   }
   printf("bmp2gcode sucess!\n");
}

void bound_to_gcode(struct gcode_struct* gcode, vector<vector< Point>> &contours, vector<int> &combine_list)
{
	printf("print outline\n");
	switch (gcode->angle_mode)//针对不同的字体朝向选择偏移的计算方式
	{
	case(0):
		for (int i = 0; i < combine_list.size(); i++)
		{
			fprintf(gcode->fp, "M05 S0\n");
			fprintf(gcode->fp, "G0 X%lf Y%lf\n", gcode->x_offset - (double)contours[combine_list[i]][0].x * gcode->mm_per_pixel, (double)contours[combine_list[i]][0].y * gcode->mm_per_pixel - gcode->y_offset);//关闭激光头移至初始点
			fprintf(gcode->fp, "M03 S255\n");
			for (int j = 1; j < contours[combine_list[i]].size(); j++)
			{
				fprintf(gcode->fp, "G1 X%lf Y%lf\n", gcode->x_offset - (double)contours[combine_list[i]][j].x * gcode->mm_per_pixel, (double)contours[combine_list[i]][j].y * gcode->mm_per_pixel - gcode->y_offset);
			}//打开激光头扫描一圈
			fprintf(gcode->fp, "G1 X%lf Y%lf\n", gcode->x_offset - (double)contours[combine_list[i]][0].x * gcode->mm_per_pixel, (double)contours[combine_list[i]][0].y * gcode->mm_per_pixel - gcode->y_offset);//首尾相连
		}
		break;
	case(1):
		for (int i = 0; i < combine_list.size(); i++)
		{
			fprintf(gcode->fp, "M05 S0\n");
			fprintf(gcode->fp, "G0 X%lf Y%lf\n", (double)contours[combine_list[i]][0].y * gcode->mm_per_pixel - gcode->y_offset, (double)contours[combine_list[i]][0].x * gcode->mm_per_pixel - gcode->x_offset);
			fprintf(gcode->fp, "M03 S255\n");
			for (int j = 1; j < contours[combine_list[i]].size(); j++)
			{
				fprintf(gcode->fp, "G1 X%lf Y%lf\n", (double)contours[combine_list[i]][j].y * gcode->mm_per_pixel - gcode->y_offset, (double)contours[combine_list[i]][j].x * gcode->mm_per_pixel - gcode->x_offset);
			}
			fprintf(gcode->fp, "G1 X%lf Y%lf\n", (double)contours[combine_list[i]][0].y * gcode->mm_per_pixel - gcode->y_offset, (double)contours[combine_list[i]][0].x * gcode->mm_per_pixel - gcode->x_offset);
		}
		break;
	case(2):
		for (int i = 0; i < combine_list.size(); i++)
		{
			fprintf(gcode->fp, "M05 S0\n");
			fprintf(gcode->fp, "G0 X%lf Y%lf\n", (double)contours[combine_list[i]][0].x * gcode->mm_per_pixel - gcode->x_offset, gcode->y_offset - (double)contours[combine_list[i]][0].y * gcode->mm_per_pixel);
			fprintf(gcode->fp, "M03 S255\n");
			for (int j = 1; j < contours[combine_list[i]].size(); j++)
			{
				fprintf(gcode->fp, "G1 X%lf Y%lf\n", (double)contours[combine_list[i]][j].x * gcode->mm_per_pixel - gcode->x_offset, gcode->y_offset - (double)contours[combine_list[i]][j].y * gcode->mm_per_pixel);
			}
			fprintf(gcode->fp, "G1 X%lf Y%lf\n", (double)contours[combine_list[i]][0].x * gcode->mm_per_pixel - gcode->x_offset, gcode->y_offset - (double)contours[combine_list[i]][0].y * gcode->mm_per_pixel);
		}
		break;
	case(3):
		for (int i = 0; i < combine_list.size(); i++)
		{
			fprintf(gcode->fp, "M05 S0\n");
			fprintf(gcode->fp, "G0 X%lf Y%lf\n", gcode->y_offset - (double)contours[combine_list[i]][0].y * gcode->mm_per_pixel, gcode->x_offset - (double)contours[combine_list[i]][0].x * gcode->mm_per_pixel);
			fprintf(gcode->fp, "M03 S255\n");
			for (int j = 1; j < contours[combine_list[i]].size(); j++)
			{
				fprintf(gcode->fp, "G1 X%lf Y%lf\n", gcode->y_offset - (double)contours[combine_list[i]][j].y * gcode->mm_per_pixel, gcode->x_offset - (double)contours[combine_list[i]][j].x * gcode->mm_per_pixel);
			}
			fprintf(gcode->fp, "G1 X%lf Y%lf\n", gcode->y_offset - (double)contours[combine_list[i]][0].y * gcode->mm_per_pixel, gcode->x_offset - (double)contours[combine_list[i]][0].x * gcode->mm_per_pixel);
		}
		break;
	default:
		for (int i = 0; i < combine_list.size(); i++)
		{
			fprintf(gcode->fp, "M05 S0\n");
			fprintf(gcode->fp, "G0 X%lf Y%lf\n", gcode->x_offset - (double)contours[combine_list[i]][0].x * gcode->mm_per_pixel, (double)contours[combine_list[i]][0].y * gcode->mm_per_pixel - gcode->y_offset);
			fprintf(gcode->fp, "M03 S255\n");
			for (int j = 1; j < contours[combine_list[i]].size(); j++)
			{
				fprintf(gcode->fp, "G1 X%lf Y%lf\n", gcode->x_offset - (double)contours[combine_list[i]][j].x * gcode->mm_per_pixel, (double)contours[combine_list[i]][j].y * gcode->mm_per_pixel - gcode->y_offset);
			}
			fprintf(gcode->fp, "G1 X%lf Y%lf\n", gcode->x_offset - (double)contours[combine_list[i]][0].x * gcode->mm_per_pixel, (double)contours[combine_list[i]][0].y * gcode->mm_per_pixel - gcode->y_offset);
		}
		break;
	}
}

void move_to_position(struct gcode_struct* gcode){
	//ZJQ
	//double half_pixel = gcode->mm_per_pixel / 2;
	switch (gcode->angle_mode)//针对不同字体朝向选择偏移的计算方式
	{
	case(0):
		fprintf(gcode->fp, "G%d X%lf Y%lf\n", gcode->nowstate, gcode->x_offset - gcode->x_pos, gcode->y_pos - gcode->y_offset);
		break;
	case(1):
		fprintf(gcode->fp, "G%d X%lf Y%lf\n", gcode->nowstate, gcode->y_pos - gcode->y_offset, gcode->x_pos - gcode->x_offset);
		break;
	case(2):
		fprintf(gcode->fp, "G%d X%lf Y%lf\n", gcode->nowstate, gcode->x_pos - gcode->x_offset, gcode->y_offset - gcode->y_pos);
		break;
	case(3):
		fprintf(gcode->fp, "G%d X%lf Y%lf\n", gcode->nowstate, gcode->y_offset - gcode->y_pos, gcode->x_offset - gcode->x_pos);
		break;
	default:
		fprintf(gcode->fp, "G%d X%lf Y%lf\n", gcode->nowstate, gcode->x_offset - gcode->x_pos, gcode->y_pos - gcode->y_offset);
		break;
	}
}

void check_pixel_height(struct gcode_struct* gcode, struct image_struct* image, Rect rect){
   if(check_pixel_cutting(gcode, image) || ((!check_pixel_cutting(gcode, image)) && gcode->x_pix >= (rect.x + rect.width - 1)) || ((!check_pixel_cutting(gcode, image)) && gcode->y_pix >= (rect.y + rect.height - 1))){//判断像素值是否大于阈值或处于矩形边界
      if(gcode->cutting == 0) {//上一次是未开激光头，此次才打开激光头移动
		  if (gcode->move_pos)
		  {
			  if (gcode->yx_ratio <= 1.0)
			  {
				  gcode->x_pos -= gcode->mm_per_pixel;
				  gcode->y_pos += (gcode->mm_per_pixel) * gcode->yx_ratio;
			  }
			  else
			  {
				  gcode->x_pos -= (gcode->mm_per_pixel) / gcode->yx_ratio;
				  gcode->y_pos += gcode->mm_per_pixel;
			  }
		  }
		  else
		  {
			  if (gcode->yx_ratio <= 1.0)
			  {
				  gcode->x_pos += gcode->mm_per_pixel;
				  gcode->y_pos -= (gcode->mm_per_pixel) * gcode->yx_ratio;
			  }
			  else
			  {
				  gcode->x_pos += (gcode->mm_per_pixel) / gcode->yx_ratio;
				  gcode->y_pos -= gcode->mm_per_pixel;
			  }
		  }//用于在移动时向内缩一个像素，防止线段超出文字边界
         move_to_position(gcode);//向当前点移动
		 gcode->scan_end_sign = 0;//进行了打开激光头移动则表明当前矩形的扫描未结束
		 if (gcode->move_pos)
		 {
			 if (gcode->yx_ratio <= 1.0)
			 {
				 gcode->x_pos += gcode->mm_per_pixel;
				 gcode->y_pos -= (gcode->mm_per_pixel) * gcode->yx_ratio;
			 }
			 else
			 {
				 gcode->x_pos += (gcode->mm_per_pixel) / gcode->yx_ratio;
				 gcode->y_pos -= gcode->mm_per_pixel;
			 }
		 }
		 else
		 {
			 if (gcode->yx_ratio <= 1.0) 
			 {
				 gcode->x_pos -= gcode->mm_per_pixel;
				 gcode->y_pos += (gcode->mm_per_pixel) * gcode->yx_ratio;
			 }
			 else
			 {
				 gcode->x_pos -= (gcode->mm_per_pixel) / gcode->yx_ratio;
				 gcode->y_pos += gcode->mm_per_pixel;
			 }
		 }
		 fprintf(gcode->fp, "M05 S0\n");
		 gcode->cutting = 1;
		 gcode->nowstate = 0;
		 gcode->draw_complete = 1;
      }
   } else {
      if(gcode->cutting){//关激光头并移动
         move_to_position(gcode);
		 fprintf(gcode->fp, "M03 S255\n");
		 gcode->cutting = 0;
		 gcode->nowstate = 1;
      }
   }
}

uint8_t check_pixel_cutting(struct gcode_struct* gcode, struct image_struct* image){
   if(image->pix_data[gcode->y_pix][gcode->x_pix][0] < gcode->current_pix_height){
      return 1;
   } else {
      return 0;
   }
}

void set_new_pos(struct gcode_struct* gcode, uint8_t add_pix, Rect boundRect){
   if(add_pix){
	   if (gcode->y_pix < (boundRect.y + boundRect.height - 1))
	   {
		   gcode->y_pos = gcode->y_pos + max(gcode->xy_mm_step / gcode->yx_cos, gcode->mm_per_pixel);
		   //gcode->y_pos = (gcode->xy_mm_step / gcode->yx_cos) * (int)((gcode->y_pos + gcode->xy_mm_step / gcode->yx_cos + 0.5) / (gcode->xy_mm_step / gcode->yx_cos));
		   gcode->y_pix = (int32_t)(gcode->y_pos / gcode->mm_per_pixel + 0.5);
	   }
	   else
	   {
		   gcode->x_pos = gcode->x_pos + max(gcode->xy_mm_step / gcode->yx_sin, gcode->mm_per_pixel);
		   //gcode->x_pos = (gcode->xy_mm_step / gcode->yx_sin) * (int)((gcode->x_pos + gcode->xy_mm_step / gcode->yx_sin + 0.5) / (gcode->xy_mm_step / gcode->yx_sin));
		   gcode->x_pix = (int32_t)(gcode->x_pos / gcode->mm_per_pixel + 0.5);
	   }
   } 
   else {
	   if (gcode->x_pix < (boundRect.x + boundRect.width - 1))
	   {
		   gcode->x_pos = gcode->x_pos + max(gcode->xy_mm_step / gcode->yx_sin, gcode->mm_per_pixel);
		   //gcode->x_pos = (gcode->xy_mm_step / gcode->yx_sin) * (int)((gcode->x_pos + gcode->xy_mm_step / gcode->yx_sin + 0.5) / (gcode->xy_mm_step / gcode->yx_sin));
		   gcode->x_pix = (int32_t)(gcode->x_pos / gcode->mm_per_pixel + 0.5);
	   }
	   else
	   {
		   gcode->y_pos = gcode->y_pos + max(gcode->xy_mm_step / gcode->yx_cos, gcode->mm_per_pixel);
		   //gcode->y_pos = (gcode->xy_mm_step / gcode->yx_cos) * (int)((gcode->y_pos + gcode->xy_mm_step / gcode->yx_cos + 0.5) / (gcode->xy_mm_step / gcode->yx_cos));
		   gcode->y_pix = (int32_t)(gcode->y_pos / gcode->mm_per_pixel + 0.5);
	   }
   }
}

void set_new_x(struct gcode_struct* gcode, uint8_t add_pix) {
	if (add_pix) {
		//gcode->x_pos = gcode->xy_mm_step * (int)((gcode->x_pos + gcode->xy_mm_step + 0.5) / gcode->xy_mm_step);
		gcode->x_pos = gcode->x_pos + max(gcode->xy_mm_step, gcode->mm_per_pixel);
	}
	else {
		//gcode->x_pos = gcode->xy_mm_step * (int)((gcode->x_pos - gcode->xy_mm_step + 0.5) / gcode->xy_mm_step);
		gcode->x_pos = gcode->x_pos - max(gcode->xy_mm_step, gcode->mm_per_pixel);
	}
	gcode->x_pix = (int32_t)(gcode->x_pos / gcode->mm_per_pixel + 0.5);
}

void set_new_y(struct gcode_struct* gcode, uint8_t add_pix){
   if(add_pix){
      //gcode->y_pos = gcode->xy_mm_step * (int)((gcode->y_pos + gcode->xy_mm_step + 0.5) / gcode->xy_mm_step);
	  gcode->y_pos = gcode->y_pos + max(gcode->xy_mm_step, gcode->mm_per_pixel);
	  
   } else {
      //gcode->y_pos = gcode->xy_mm_step * (int)((gcode->y_pos - gcode->xy_mm_step + 0.5) / gcode->xy_mm_step);
	  gcode->y_pos = gcode->y_pos - max(gcode->xy_mm_step, gcode->mm_per_pixel);
   }
   gcode->y_pix = (int32_t) (gcode->y_pos / gcode->mm_per_pixel + 0.5);
}

