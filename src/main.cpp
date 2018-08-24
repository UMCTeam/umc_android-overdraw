#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#ifdef __CYGWIN__
#include <dirent.h>
#endif

#include "config.h"
#include "logger.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"       // 核心组件
#include "opencv2/highgui/highgui.hpp"  // GUI
#include "opencv2/imgproc/imgproc.hpp"  // 图像处理

using namespace ncv;
using namespace cv;

void match(char* tpl, char* dst);
float overdrawAnalyze (Mat& src);

struct MODE_TYPE {
    const char* MATCH = "match";
    const char* EXR  = "exr";
} mode_type;

int main(int argc, char** argv) {
    Logger* logger = Logger::GetInstance();

    int args;
    char* mode = nullptr;
    char* tpl  = nullptr;
    char* dst  = nullptr;
    while (( args = getopt(argc, argv, "v:h:m:t:d:")) != -1) {
        switch (args) {
            case 'h':
                std::cout << "node-addon version 1.0.0: \n" \
                            "-v \t 帮助: 安卓过度渲染处理工具" << std::endl;
                break;
            case 'v':
            {
                std::cout << "version: "  << NODE_OPENCV_VERSION <<std::endl;
            }
                break;
            case 'm':
                mode = optarg;
                break;
            case 'd':
            {
                dst = optarg;
               // dst = const_cast<char*> (abs_path(std::string(optarg)).c_str());
            }
                break;
            case 't':
            {
                tpl = optarg;
                //tpl = const_cast<char*> (abs_path(std::string(optarg)).c_str());
            }
                break;
        }

    }

    if (!strcasecmp(mode, mode_type.MATCH)) {
        match(tpl, dst);
    } else if (!strcasecmp(mode, mode_type.EXR)) {
        //过滤无用色值
        Mat mat_src = imread(dst);
        float percent = overdrawAnalyze(mat_src);
        std::cout << percent << std::endl;
    }

    //logger->warn("exit");
    return  0;
}


void match(char* tpl, char* dst) {
    Mat m_tpl = imread(tpl);
    Mat m_dst = imread(dst);
    Mat im_result;
    m_dst.copyTo(im_result);

    Mat result;
    int result_cols = m_dst.cols - m_tpl.cols + 1;
    int result_rows = m_dst.rows - m_tpl.rows + 1;

    result.create(result_cols, result_rows, CV_32FC1);

    //模板匹配
    matchTemplate(m_dst, m_tpl, result, 4);
    normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

    //获取匹配度最高位置
    Point matchLoc;
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
    matchLoc = maxLoc;

    //显示匹配位置
    rectangle(im_result, matchLoc, Point(matchLoc.x + m_tpl.cols, matchLoc.y + m_tpl.rows), Scalar(0, 0, 255), 4, 4);
    imwrite("c:\\test.png", im_result);
    waitKey(0);
}

float overdrawAnalyze(Mat& src) {
    CvMat cvmat = src;
    IplImage* hsv = cvCreateImage(cvGetSize(&cvmat), 8, 3);
   // IplImage* dst_hsv = cvCreateImage(cvGetSize(&cvmat), 8, 3);
    cvGetImage(&cvmat, hsv);

    int width  = hsv->width;
    int height = hsv->height;
    long double  red_number = 0;

    //阈值 0 - 255；
    int limit  = 100;

    for (int i = 0; i < height ; ++i) {
        for (int j = 0; j < width; ++j) {
            CvScalar color = cvGet2D(hsv, i, j);
            double b = color.val[0];
            double g = color.val[1];
            double r = color.val[2];

            double dst_r = (1 - (abs(g - b)/ 255.0)) * (1 - (g + b)/(500.0)) * r;

            if (dst_r > limit) {
                ++ red_number;
            }

            //dst_r = dst_r > limit ? 255 : 0;
            //CvScalar dst_color(0, 0, dst_r);
            //cvSet2D(dst_hsv, i, j, dst_color);
        }
    }

    //CvMat* dst = cvCreateMat(dst_hsv->height, dst_hsv->width, CV_8UC3);
    //cvConvert(dst_hsv, dst);

    //imwrite("c:\\test.png", Mat(dst->rows, dst->cols, dst->type, dst->data.fl));

    return red_number / (float)(hsv->width * hsv->height);
}