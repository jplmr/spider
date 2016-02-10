#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <opencv2/opencv.hpp>

using namespace cv;

const Scalar BLACK = Scalar(0, 0, 0); // b, g, r
const Scalar GRAY = Scalar(100, 100, 100); // b, g, r
const Scalar RED = Scalar(0, 0, 255); // b, g, r
const Scalar GREEN = Scalar(0, 255, 0); // b, g, r

const Scalar START_BLUE = Scalar(170, 190, 50);
const Scalar END_BLUE = Scalar(190, 210, 70);

const int YRANGE = 20;

void processFrame(Mat frame, const std::string window);

int main(int argc, char **argv) {

    const std::string src = "../screencaps/spider_sample_1.mp4";
    const std::string window = "video";
    VideoCapture capt(src);
    namedWindow(window, WINDOW_AUTOSIZE);

    std::cout << "input size: " << capt.get(CAP_PROP_FRAME_WIDTH) << "x"
        << capt.get(CAP_PROP_FRAME_HEIGHT) << std::endl;

    Mat frame;

    while (true) {
        capt.read(frame);
        if(frame.empty()) break;
        processFrame(frame, window);
    }

    capt.release();

    return 0;
}

void processFrame(Mat frame, const std::string window) {
    int spiderCol = frame.cols * 0.242;
    int spiderWidth = frame.cols * 0.04;
    int spiderStartHeight = frame.rows * 0.415;

    rectangle(frame, Point(spiderCol + spiderWidth / 2, -10), Point(spiderCol - spiderWidth / 2, frame.rows + 10), GRAY, 2);
    
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

    Mat green(col.size(), CV_8UC3, RED);
    green.copyTo(frame.colRange(spiderCol - spiderWidth / 2, spiderCol + spiderWidth / 2), col);

    rectangle(frame, Point(spiderCol - spiderWidth / 2, max), Point(spiderCol + spiderWidth / 2, max + frame.rows / YRANGE), BLACK, 3);

    std::ostringstream os;
    os << "y: " << max << " int: " << maxWd;

    putText(frame, os.str(), Point(0, 100), FONT_HERSHEY_SIMPLEX, 2.0, BLACK, 2);

    imshow(window, frame);
    waitKey(40);
}
