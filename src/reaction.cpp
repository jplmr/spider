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
using namespace std;
Mat getCapture(Display *disp, Window root);
Window win;
Display *display;
Mat frame;

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
    Vec4b color = frame.at<Vec4b>(100,100);
    if(color == Vec4b(0, 240, 17, 255)) {
        clickDown(display, 1);
        clickUp(display, 1);
    }
    cout << color << endl; 
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

    while(true) {
        frame = getCapture(display, win);
        if(frame.rows > 0) {
            frame = frame(Rect(240,65,800,600));
            doMove();
            imshow("vmware", frame);
        }
        key = waitKey(1000 / 40);
        if(key == 113)
            break;
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
