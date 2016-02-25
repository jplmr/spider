#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <sys/time.h>

#include "spider.hpp"

using namespace cv;

const Scalar BLACK = Scalar(0, 0, 0); // b, g, r
const Scalar GRAY = Scalar(100, 100, 100); // b, g, r
const Scalar RED = Scalar(0, 0, 255); // b, g, r
const Scalar GREEN = Scalar(0, 255, 0); // b, g, r

const Scalar START_BLUE = Scalar(170, 190, 50);
const Scalar END_BLUE = Scalar(190, 210, 70);

const int YRANGE = 720 / 4;
const int XRANGE = 20;

void processFrame(Mat frame, const std::string window);

int dispvid(int argc, char **argv) {

    const std::string src = "assets/spider_sample_1.mp4";
    const std::string window = "video";
    VideoCapture capt(src);
    namedWindow(window, WINDOW_AUTOSIZE);

    std::cout << "input size: " << capt.get(CAP_PROP_FRAME_WIDTH) << "x"
        << capt.get(CAP_PROP_FRAME_HEIGHT) << std::endl;

    Mat frame;

    timeval tv;
    gettimeofday(&tv, 0);
    uint64_t millisstart = (tv.tv_sec * (uint64_t)1000) + (tv.tv_usec / 1000);
    int frames = 0;

    while (true) {
        capt.read(frame);
        if(frame.empty()) break;
        processFrame(frame, window);
        frames++;
    }

    gettimeofday(&tv, 0);
    uint64_t millisend = (tv.tv_sec * (uint64_t)1000) + (tv.tv_usec / 1000);
    uint64_t diff = millisend - millisstart;

    std::cout << "processed " << frames << " frames in " << diff << " ms" << std::endl;
    double fps = (double) frames / ((double) diff / 1000.0);
    double spf = (double) diff / (double) frames; 
    std::cout << "rate: " << fps << " fps or 1 frame per " << spf << " ms" << std::endl;

    capt.release();

    return 0;
}

void processFrame(Mat frame, const std::string window) {
    int spiderCol = frame.cols * 0.242;
    int spiderWidth = frame.cols * 0.04;
    int spiderStartHeight = frame.rows * 0.415;

    // spider column location indicator
    rectangle(frame, Point(spiderCol + spiderWidth / 2, -10), Point(spiderCol - spiderWidth / 2, frame.rows + 10), GRAY, 2);
    
    // find spider's y coordinate
    Mat col(frame.colRange(spiderCol - spiderWidth / 2, spiderCol + spiderWidth / 2));
    inRange(col, START_BLUE, END_BLUE, col);

    int max = -1;
    double maxWd = -1.0;
    for(int y = 0; y < frame.rows; y += frame.rows / YRANGE) {
        Scalar meanW = mean(col.rowRange(y, y + frame.rows / YRANGE));
        double meanWd = sum(meanW)[0];
        if (meanWd > maxWd) {
            maxWd = meanWd;
            max = y;
        }
    }

    // draw on top of spider 
    Mat spider(col.size(), CV_8UC3, RED);
    spider.copyTo(frame.colRange(spiderCol - spiderWidth / 2, spiderCol + spiderWidth / 2), col);

    // draw rectangle around spider
    rectangle(frame, Point(spiderCol - spiderWidth / 2, max - spiderWidth / 2), Point(spiderCol + spiderWidth / 2, max + spiderWidth / 2), BLACK, 3);

    
    // determine stage color
    Vec3b stageCol = frame.at<Vec3b>(0,0);  // TODO sample and average
    Mat stage(frame.colRange(spiderCol + spiderWidth / 2, frame.cols));
    inRange(stage, stageCol - Vec3b(10,10,10), stageCol + Vec3b(10,10,10), stage);
    Mat stageMask(stage.size(), CV_8UC3, Scalar(0,0,0));
    stageMask.copyTo(frame.colRange(spiderCol + spiderWidth / 2, frame.cols), stage);

    // find obstacles 
    for(int y = 0; y < stage.rows; y += stage.rows / YRANGE) {
        Scalar meanW = mean(stage(Rect(0, y, stage.cols, stage.rows / YRANGE)));
        double meanWd = sum(meanW)[0];
        Scalar color;
        if(meanWd < 10.0) {
            color = GREEN;
        } else {
            color = RED;
        }
        rectangle(frame, Point(spiderCol - spiderWidth / 2, y), 
                Point(spiderCol + spiderWidth / 2, y + (frame.rows / YRANGE)), color, 1);
    }

    // show diagnostic info
    std::ostringstream os;
    os << "y: " << max;
    putText(frame, os.str(), Point(0, frame.rows - frame.rows / 10), FONT_HERSHEY_SIMPLEX, 1.5, BLACK, 2);

    imshow(window, frame);
    waitKey(1);
}
