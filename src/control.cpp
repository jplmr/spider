#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xmu/WinUtil.h>

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string.h>
#include <sys/time.h>

using namespace cv;
Mat getCapture(Display *disp, Window root);

const Scalar BLACK = Scalar(0, 0, 0); // b, g, r
const Scalar GRAY = Scalar(100, 100, 100); // b, g, r
const Scalar RED = Scalar(0, 0, 255); // b, g, r
const Scalar GREEN = Scalar(0, 255, 0); // b, g, r
const Scalar YELLOW = Scalar(0, 255, 255); // b, g, r

const Scalar START_BLUE = Scalar(150, 170, 50, 0);
const Scalar END_BLUE = Scalar(220, 220, 150, 255);

const int YRANGE = 600 / 10;
const int XRANGE = 20;

int lastY;

Display *display;
Window win;

typedef struct {
    int playerY;
    bool obstacles[YRANGE];
} State;

State curState;
Mat frame;
int spiderCol, spiderWidth, spiderStartHeight;
int fps;
int clicked;

void processFrame() {

    spiderCol = frame.cols * 0.15;
    spiderWidth = frame.cols * 0.04;
    spiderStartHeight = frame.rows * 0.415;

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
    Mat spider(col.size(), frame.type(), RED);
    spider.copyTo(frame.colRange(spiderCol - spiderWidth / 2, spiderCol + spiderWidth / 2), col);

    // draw rectangle around spider
    rectangle(frame, Point(spiderCol - spiderWidth / 2, max - spiderWidth / 2), Point(spiderCol + spiderWidth / 2, max + spiderWidth / 2), BLACK, 3);

    
    // determine stage color
    Vec4b stageCol = frame.at<Vec4b>(1,1);
    int startCol = spiderCol + spiderWidth / 2, endCol = frame.cols;

    Mat stage(frame.colRange(startCol, endCol));
    inRange(stage, stageCol, stageCol, stage);
    Mat stageMask(stage.size(), CV_8UC4, Scalar(0,0,255));
    stageMask.copyTo(frame.colRange(startCol, endCol), stage);

    // find obstacles 
    for(int y = 0, ct = 0; y < stage.rows; y += stage.rows / YRANGE, ct++) {
        Scalar meanW = mean(stage(Rect(0, y, stage.cols/2, stage.rows / YRANGE)));
        double meanWd = sum(meanW)[0];
        Scalar color;
        if(meanWd < 10.0) {
            curState.obstacles[ct] = 0;
            color = GREEN;
        } else {
            curState.obstacles[ct] = 1;
            color = RED;
        }
        rectangle(frame, Point(spiderCol - spiderWidth / 2, y), 
                Point(spiderCol + spiderWidth / 2, y + (frame.rows / YRANGE)), color, 2);
    }

    // show diagnostic info
    std::ostringstream os;
    os << fps << " fps, y: " << max << ", tap: " << ((max < lastY) ? "X" : "_");
    curState.playerY = max;
    putText(frame, os.str(), Point(0, frame.rows - frame.rows / 10), FONT_HERSHEY_SIMPLEX, 1.1, BLACK, 2);

    lastY = max;
}

//https://gist.github.com/pioz/726474
void
clickDown (Display *display, int button)
{
  // Create and setting up the event
  XEvent event;
  memset (&event, 0, sizeof (event));
  event.xbutton.button = button;
  event.xbutton.same_screen = True;
  event.xbutton.subwindow = DefaultRootWindow (display);
  while (event.xbutton.subwindow)
    {
      event.xbutton.window = event.xbutton.subwindow;
      XQueryPointer (display, event.xbutton.window,
		     &event.xbutton.root, &event.xbutton.subwindow,
		     &event.xbutton.x_root, &event.xbutton.y_root,
		     &event.xbutton.x, &event.xbutton.y,
		     &event.xbutton.state);
    }
  // Press
  event.type = ButtonPress;
  if (XSendEvent (display, PointerWindow, True, ButtonPressMask, &event) == 0)
    fprintf (stderr, "Error to send the event!\n");
  XFlush (display);
}

void
clickUp (Display *display, int button)
{
  // Create and setting up the event
  XEvent event;
  memset (&event, 0, sizeof (event));
  event.xbutton.button = button;
  event.xbutton.same_screen = True;
  event.xbutton.subwindow = DefaultRootWindow (display);
  while (event.xbutton.subwindow)
    {
      event.xbutton.window = event.xbutton.subwindow;
      XQueryPointer (display, event.xbutton.window,
		     &event.xbutton.root, &event.xbutton.subwindow,
		     &event.xbutton.x_root, &event.xbutton.y_root,
		     &event.xbutton.x, &event.xbutton.y,
		     &event.xbutton.state);
    }
   // Release
  event.type = ButtonRelease;
  if (XSendEvent (display, PointerWindow, True, ButtonReleaseMask, &event) == 0)
    fprintf (stderr, "Error to send the event!\n");
  XFlush (display);
}

void doMove() {
    std::cout << curState.playerY << std::endl;
    for(int i = 0; i < YRANGE; i++) printf("%d", curState.obstacles[i]);
    std::cout << std::endl;


    // find longest contiguous sequence of zeroes
    int start = 0, max = 1, count = 1;
    for(int i = 1; i < YRANGE; i++) {
        if(curState.obstacles[i] == curState.obstacles[i-1] && curState.obstacles[i] == 0) {
            count++;
        } else {
            if(count > max) {
                max = count;
                start = i - max;
            }
            count = 1;
        }
    }

    int goal = start + max / 2;
    int step = frame.rows / YRANGE;
    line(frame, Point(spiderCol + spiderWidth / 2, goal * step),
            Point(spiderCol + spiderWidth / 2 + 1000, goal * step), YELLOW, 2);


    int playerIndex = curState.playerY / step;

    std::cout << start << " " << max << " " << goal << "?=" << playerIndex << std::endl;


    // need to click this frame
    if(playerIndex > goal) {
        clickDown(display, 1);
        clicked = 1;
    } else {
        clicked = 0;
    }

}


int main() {
    display = XOpenDisplay(NULL); 

    if(display == NULL) {
        std::cerr << "failed to connect to display" << std::endl;
        exit(-1);
    }

    Window root = DefaultRootWindow(display);
    XWindowAttributes attrs;
    XGetWindowAttributes(display, root, &attrs);
    std::cout << "width: " << attrs.width << " height: " << attrs.height << std::endl;

    Window root_win, parent_win;
    Window *child_list;
    Window child;
    unsigned int num_children; 
    unsigned int i;

    if(!XQueryTree(display, root, &root_win, &parent_win, &child_list, &num_children)) {
        std::cerr << "failed to query display windows" << std::endl;
    }
    
    for(i = 0; i < num_children; i++) {
        char *win_name;
        XWindowAttributes cattrs;
        XGetWindowAttributes(display, child_list[i], &cattrs);
        if(XFetchName(display, child_list[i], &win_name)) {
            if(strncmp(win_name, "remix-os", 8) == 0) {
                child = child_list[i];
                printf("%s %dx%d\n", win_name, cattrs.width, cattrs.height);
                break;
            }
        }        
    } 

    win = child;

    int key;

    timeval tv;
    uint64_t millisstart, millisend, diff;
    int lastClicked = clicked;
    while(true) {
        std::cout << "clicked: " << clicked << std::endl;
        gettimeofday(&tv, 0);
        millisstart = (tv.tv_sec * (uint64_t)1000) + (tv.tv_usec / 1000);

        frame = getCapture(display, win);
        if(frame.rows > 0) {
            frame = frame(Rect(240,65,800,600));
            processFrame();
            doMove();
            imshow("vmware", frame);
        }
        key = waitKey(1000 / 30);

        if(lastClicked != clicked && clicked == 0) {
            clickUp(display, 1);
        }

        lastClicked = clicked;

        if(key == 113)
            break;

        gettimeofday(&tv, 0);
        millisend = (tv.tv_sec * (uint64_t)1000) + (tv.tv_usec / 1000);
        diff = millisend - millisstart;
        fps = (int) ((double) 1.0 / ((double) diff / 1000.0));
    }

    if(child_list) XFree((char *)child_list);
    XCloseDisplay(display);
}

/* sourced from Alexandre Pinheiro at http://stackoverflow.com/questions/18620943 */
IplImage *XImage2IplImageAdapter(XImage *ximage)
{
        IplImage *iplImage;
        assert(ximage->format == ZPixmap);
        assert(ximage->depth == 24); 

        iplImage = cvCreateImageHeader(
                    cvSize(ximage->width, ximage->height), 
                IPL_DEPTH_8U,
                ximage->bits_per_pixel/8);

        iplImage->widthStep = ximage->bytes_per_line;
        if(ximage->data != NULL)
                iplImage->imageData = ximage->data;

        return iplImage;
}

/* sourced from Brandon at http://stackoverflow.com/questions/24988164/ */
Mat getCapture(Display *display, Window root)
{
    XWindowAttributes attrs;
    XGetWindowAttributes(display, root, &attrs);

    XImage* img = XGetImage(display, root, 0, 0, attrs.width, attrs.height, AllPlanes, ZPixmap);
    IplImage *cvImageSample = XImage2IplImageAdapter(img);
    XFree(img);

    Mat matImg = cvarrToMat(cvImageSample, true);

    return matImg;
}
