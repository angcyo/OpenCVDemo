
#include <stdint.h>
#include <stdio.h>
#include "string.h"

//#include "global_settings.h"
#include "bmpTogcode.h"
#include "bmp_io.h"
//#include "pixels.h"
#include "image.h"
#include "gcode.h"
#include "math.h"
#include <string>
#include <algorithm>
#define PI 3.14159

struct image_struct input_image;
struct image_struct output_image;
struct program_struct program;
struct gcode_struct gcode;

struct image_struct tmp_image;

int bmpTogcodeFromPath(const char *inputPath,const char *outputPath,double scale_out, double xy_mm_step, uint16_t angle_mode, double anglec){
   char file_path[200];
   char gcode_path[200];

   printf("Starting BMP Test\n\n");

   
   strcpy(file_path, inputPath);//蔵nput file path
   strcpy(gcode_path, outputPath);//蕂utput file path
//   gcode.scale_out = 50.0;//printing size max 50 mm
//   gcode.xy_mm_step = 0.125;// line space mm
//   gcode.angle_mode = 0;//font direction,0:0 1:90 2:180 3:270
//   double angle = 0.0;// scan angle
    
    gcode.scale_out = scale_out;//printing size max 50 mm
    gcode.xy_mm_step = xy_mm_step;// line space mm
    gcode.angle_mode = angle_mode;//font direction,0:0 1:90 2:180 3:270
    double angle = anglec;// scan angle
   //end

   gcode.yx_ratio = tan(angle * PI / 180);//计算对应角度所需值
   gcode.yx_cos = cos(angle * PI / 180);
   gcode.yx_sin = sin(angle * PI / 180);
   gcode.scan_mode = angle > 0.1 && angle < 89.9 ? 0 : angle < 0.1 ? 1 : 2;//线段扫描方式，垂直和水平所用方式与其余不同
   //printf("%lf\n", gcode.yx_ratio);

   create_gcode_file(&gcode, gcode_path);//创建gcode文件
   Mat tmpImage = imread(file_path);//读取图片
   if (tmpImage.empty())
   {
	   printf("Img is empty!\n");
	   return 0;
   }
   else
   {
	   printf("Successfully get image!\n");
   }

   Mat srcImage;
   if (gcode.scan_mode == 0)
   {
	   int scale_out_rat = (int)(gcode.scale_out / (gcode.xy_mm_step / min(gcode.yx_cos, gcode.yx_sin)));
	   gcode.scale_out = (scale_out_rat % 2 == 0 ? scale_out_rat - 1 : scale_out_rat) * (gcode.xy_mm_step / min(gcode.yx_cos, gcode.yx_sin));//要同时保证scale_out_rat与resize_rat为奇数才不会出现双线打印问题
   }
   int resize_w, resize_h;
   resize_w = (2 * gcode.scale_out / gcode.xy_mm_step) / max(tmpImage.cols, tmpImage.rows) * tmpImage.cols;
   resize_h = (2 * gcode.scale_out / gcode.xy_mm_step) / max(tmpImage.cols, tmpImage.rows) * tmpImage.rows;
   int resize_rat = max(((tmpImage.rows / resize_h) + 1), ((tmpImage.cols / resize_w) + 1));
   if (resize_rat % 2 == 0)//要同时保证scale_out_rat与resize_rat为奇数才不会出现双线打印问题
	   resize_rat += 1;
   resize_w = resize_rat * resize_w;
   resize_h = resize_rat * resize_h;
   resize(tmpImage, srcImage, Size(resize_w, resize_h), INTER_LINEAR);//插值重采样，适应线间距

   struct bound_struct bound;
   boundRect(bound, srcImage);//边缘检测+外接矩形并按x值排序
   if (bound.boundRect.size() == 0)
   {
	   printf("Error getting Rectangle!\n");
	   return 0;
   }
   else
   {
	   printf("successfully get Rectangle!\n");
   }

   mat2image(&srcImage, &input_image);

   //read_bmp(&input_image, file_path);

   greyscale_image(&input_image);
   
   invert_image(&input_image);

   bmp_to_gcode(&input_image, &gcode, &bound);//生成gcode文件

   close_gcode_file(&gcode);
   return 0;
}

bool rect_rank_x(vector<Rect> &vec_rects, vector<vector<int>> &combine_list) {//将外接矩形按x的值排序
	Rect vec_temp;
	vector<int> comb_temp;
	for (int l = 1; l < vec_rects.size(); l++) {
		for (int m = vec_rects.size() - 1; m >= l; m--) {
			if (vec_rects[m].x < vec_rects[m - 1].x) {
				vec_temp = vec_rects[m - 1];
				vec_rects[m - 1] = vec_rects[m];
				vec_rects[m] = vec_temp;
				comb_temp = combine_list[m - 1];//哪些矩形进行了合并的列表也要排序
				combine_list[m - 1] = combine_list[m];
				combine_list[m] = comb_temp;
			}
		}
	}
	return true;
}

bool isOverlap(const Rect &rc1, const Rect &rc2)//判断是否有矩形重叠
{
	if (rc1.x + rc1.width > rc2.x &&
		rc2.x + rc2.width > rc1.x &&
		rc1.y + rc1.height > rc2.y &&
		rc2.y + rc2.height > rc1.y
		)
		return true;
	else
		return false;
}

void boundRect(struct bound_struct &bound, Mat &srcImage)
{
	//Mat srcImage = imread(srcPath);
	//struct bound_struct bound;
	if (srcImage.empty())
	{
		return;
	}
	else
	{
		Mat temp;
		Mat binary_temp;
		Mat flip_temp;
		cvtColor(srcImage, temp, COLOR_BGR2GRAY);
		threshold(temp, binary_temp, 150, 255, CV_THRESH_BINARY_INV);
		vector<Vec4i> hierarchy;
		findContours(binary_temp, bound.contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
		//bound.boundRect.resize(bound.contours.size());
		for (int i = 0; i < bound.contours.size(); i++)
		{
			//bound.boundRect[i] = boundingRect(Mat(bound.contours[i]));
			bool overlap = false;
			Rect tmpRect = boundingRect(Mat(bound.contours[i]));
			for (int j = 0; j < bound.boundRect.size(); j++)
			{
				if (isOverlap(tmpRect, bound.boundRect[j]) || isOverlap(bound.boundRect[j], tmpRect))
				{
					int Ablx = tmpRect.x + tmpRect.width, Ably = tmpRect.y + tmpRect.height, Bblx = bound.boundRect[j].x + bound.boundRect[j].width, Bbly = bound.boundRect[j].y + bound.boundRect[j].height;

					bound.boundRect[j].x = min(tmpRect.x, bound.boundRect[j].x);//有重叠则将重叠矩形合并
					bound.boundRect[j].y = min(tmpRect.y, bound.boundRect[j].y);
					bound.boundRect[j].width = max(Ablx, Bblx) - bound.boundRect[j].x;
					bound.boundRect[j].height = max(Ably, Bbly) - bound.boundRect[j].y;

					bound.combine_list[j].push_back(i);
					overlap = true;
					break;
				}
			}
			if (!overlap)
			{
				bound.boundRect.push_back(tmpRect);
				vector<int> tmp = { i };
				bound.combine_list.push_back(tmp);
			}
		}
		//imshow("srcImage", srcImage);
		//waitKey(0);
		rect_rank_x(bound.boundRect, bound.combine_list);
		return;
	}
}
