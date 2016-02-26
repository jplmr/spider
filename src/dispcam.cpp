#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <sys/time.h>

#include "spider.hpp"

using namespace cv;

int dispcam(int argc, char **argv) {

    const std::string window = "video";
    VideoCapture capt(0);
    
    namedWindow(window, WINDOW_AUTOSIZE);

    std::cout << "input size: " << capt.get(CAP_PROP_FRAME_WIDTH) << "x"
        << capt.get(CAP_PROP_FRAME_HEIGHT) << std::endl;

    Mat frame;

    while (true) {
        capt.read(frame);
        if(frame.empty()) break;
        imshow(window, frame);
        waitKey(10);
    }

    capt.release();

    return 0;
}
