#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("usage: disp <image_path>\n");
    }

    Mat image;
    image = imread(argv[1], 1);
    if(!image.data) {
        printf("no data\n");
    }
    namedWindow("Display Image", WINDOW_AUTOSIZE);
    imshow("Display Image", image);

    waitKey(0);

    return 0;
}
