#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <sys/time.h>

#include "spider.hpp"

using namespace std;
using namespace cv;

const Scalar BLACK = Scalar(0, 0, 0); // b, g, r
const Scalar GRAY = Scalar(100, 100, 100); // b, g, r
const Scalar RED = Scalar(0, 0, 255); // b, g, r
const Scalar GREEN = Scalar(0, 255, 0); // b, g, r

Point screen_tl(0,0);
Point screen_br(0,0);

const int device_res_x = 2560;
const int device_res_y = 1440;

void GUICallback(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN ) {
        screen_tl = Point(x, y);
        cout << "LMB (" << x << ", " << y << ")" << endl;
    } else if  ( event == EVENT_RBUTTONDOWN ) {
        screen_br = Point(x, y);
        cout << "RMB (" << x << ", " << y << ")" << endl;
    } else if  ( event == EVENT_MBUTTONDOWN ) {
        int xClick = device_res_x - (int) ((float) (x - screen_tl.x) / (float) (screen_br.x - screen_tl.x) * device_res_x);
        int yClick = (int) ((float) (y - screen_tl.y) / (float) (screen_br.y - screen_tl.y) * device_res_y);
        ostringstream command;
        command << "adb shell input tap " << yClick << " " << xClick;
        system(command.str().c_str());

        cout << "MMB (" << xClick << ", " << yClick << ")" << endl;
    } else if ( event == EVENT_MOUSEMOVE ) { }
}

int dispcam(int argc, char **argv) {

    const std::string window = "video";

    VideoCapture capt;
    for (int i = 0; i < 5; i++) {
        capt = VideoCapture(1);
        if(capt.isOpened()) { break; }
    }

    if(!capt.isOpened()) {
        cout << "could not find a camera" << endl;
        return -1;
    } 

    namedWindow(window, WINDOW_AUTOSIZE);
    setMouseCallback(window, GUICallback, NULL);

    std::cout << "input size: " << capt.get(CAP_PROP_FRAME_WIDTH) << "x"
        << capt.get(CAP_PROP_FRAME_HEIGHT) << std::endl;

    Mat frame;

    while (true) {
        capt.read(frame);
        if(frame.empty()) break;

        flip(frame, frame, -1);

        rectangle(frame, screen_tl, screen_br, GREEN, 1);

        imshow(window, frame);
        waitKey(10);
    }

    capt.release();

    return 0;
}
