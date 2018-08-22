#include <iostream>
#include <string>
#include <unistd.h>

#include "config.h"
#include "logger.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"       // 核心组件
#include "opencv2/highgui/highgui.hpp"  // GUI
#include "opencv2/imgproc/imgproc.hpp"  // 图像处理

using namespace ncv;
using namespace cv;

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
    //imshow("result", im_result);
    imwrite("c:\\test.png", im_result);
    waitKey(0);
}

Mat filter(Mat& src) {
    CvMat cvmat = src;
    IplImage* hsv = cvCreateImage(cvGetSize(&cvmat), 8, 3);
    cvGetImage(&cvmat, hsv);

    int width = hsv->width;
    int height = hsv->height;

    for (int i = 0; i < height ; ++i) {
        for (int j = 0; j < width; ++j) {
            CvScalar s_hsv = cvGet2D(hsv, i, j);
            if (s_hsv.val[0] < 235) {
                if (s_hsv.val[1] > 138 || s_hsv.val[1] < 118) {
                    if (s_hsv.val[2] > 137 || s_hsv.val[2] < 117) {
                        CvScalar s;
                        s.val[0] = 0;
                        s.val[1] = 0;
                        s.val[2] = 0;
                        cvSet2D(hsv, i, j, s);
                    }
                }
            }
        }
    }

    CvMat* dst = cvCreateMat(hsv->height, hsv->width, CV_8UC1);
    cvConvert(hsv, dst);
    return Mat(dst->rows, dst->cols, dst->type, dst->data.fl);
}

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
                dst = optarg;
                break;
            case 't':
                tpl = optarg;
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
