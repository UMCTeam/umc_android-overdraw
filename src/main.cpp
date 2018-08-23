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
Mat filter(Mat& src);
std::string abs_path(std::string path);

struct MODE_TYPE {
    char* MATCH = "match";
    char* EXR  = "exr";
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
        Mat mat_filter = filter(mat_src);
        imwrite("c:\\a.png", mat_filter);
    }

    logger->warn("exit");
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

Mat filter(Mat& src) {
    int width = src.size().width;
    int height = src.size().height;

    Mat border, sharpen, edge, grayImage;
    copyMakeBorder(src, border, 20, 20, 20, 20, BORDER_CONSTANT, Scalar(255, 255, 255));

    //图片降噪
    blur(border, border,Size(2, 2));

    //锐化处理, 增强边缘
    Mat kernel = (Mat_<char>(3,3) << 0, -1 ,0,
                                     -1, 5, -1,
                                     0, -1, 0);
    filter2D(border,sharpen,border.depth(),kernel, Point(1, 1));

    //原始图片转化为灰度图
    cvtColor(sharpen, grayImage, COLOR_BGR2GRAY);

    //使用canny 算子,进行边缘检测
    Canny(grayImage, edge, 50, 50);

    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;

    //提取轮廓
    findContours(edge, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

  /*  //在黑布上绘制矩形
    Mat polyPic = Mat::zeros(border.size(), CV_8UC3);
    for (int index = 0; index < contours.size(); index++){
        std::vector<int> hull;
        //提取轮廓边角数量
        convexHull(contours[index], hull, false, true);

        drawContours(polyPic, contours, index, Scalar(0,0,255), 2);
    }*/

    for (int index = 0; index < contours.size(); ++index) {
        //获取十分之一大小的控件
        int screen_size = width * height;
        std::vector<Point> polyContour;
        std::vector<int> hull;

        //使用多边形矩形包裹,提取的轮廓
        approxPolyDP(contours[index], polyContour, 2, true);

        //提取轮廓边角数量
        convexHull(polyContour, hull, false, true);

        //移除太小的组件
        double area = contourArea(polyContour);
        if ( area < (screen_size / 3)  &&  area > (screen_size / 100)) {
            std::vector<Point> pos = contours[index];
            Rect rect = boundingRect(Mat(polyContour));
            Mat component = border(rect);

            std::stringstream filename;
            filename << rand() * 10000;
            imwrite("d:\\tmp\\ " + filename.str() + "component.png", component);

           // rectangle(border, polyContour.at(0), polyContour.at(2), Scalar(255, 0, 0));

            for (; rect.width > 10; ) {
                //获取组件颜色的平局值， 方差
                cv::Scalar     mean;
                cv::Scalar     dev;
                meanStdDev(component, mean, dev);

                //红色区域色值趋于稳定, 色值偏于红色
                if (dev.val[2] < 0.35) {
                    int blue = mean.val[0];
                    int green = mean.val[1];
                    int red = mean.val[2];
                    int offset = 100;

                    //色值是否偏于红色
                    if (blue < (red - offset) && green < (red - offset) && red > 250) {
                        std::cout << "overdraw" << std::endl;
                    }
                    break;
                }

                rect.width *= 0.8;
                rect.height *= 0.8;
                component = border(rect);
            }
        }
    }

    imwrite("d:\\test.png", border);

    CvMat cvmat = border;
    IplImage* hsv = cvCreateImage(cvGetSize(&cvmat), 8, 3);
    cvGetImage(&cvmat, hsv);

    for (int i = 0; i < height ; ++i) {
        for (int j = 0; j < width; ++j) {
            CvScalar s_hsv = cvGet2D(hsv, i, j);
            CvScalar s;

            if (s_hsv.val[0] < 235) {
                if (s_hsv.val[1] > 138 || s_hsv.val[1] < 118) {
                    if (s_hsv.val[2] > 137 || s_hsv.val[2] < 117) {
                        s.val[0] = 255;
                        s.val[1] = 255;
                        s.val[2] = 255;
                        cvSet2D(hsv, i, j, s);
                        continue;
                    }
                }
            }

            s.val[0] = 0;
            s.val[1] = 0;
            s.val[2] = 0;
            cvSet2D(hsv, i, j, s);
        }
    }

    CvMat* dst = cvCreateMat(hsv->height, hsv->width, CV_8UC3);
    cvConvert(hsv, dst);
    return Mat(dst->rows, dst->cols, dst->type, dst->data.fl);
}

std::string abs_path(std::string path) {
#define MAX_PATH  40960
    char buffer[MAX_PATH] = {0};

    getcwd(buffer, MAX_PATH);
    std::strcat(buffer, path.c_str());
    #ifdef __CYGWIN__
        //TODO:绝对路径转化
    #endif
    return  std::string(buffer);
}