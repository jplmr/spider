#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const Scalar BLACK = Scalar(0, 0, 0); // b, g, r
const Scalar WHITE = Scalar(255, 255, 255); // b, g, r
const Scalar GRAY = Scalar(100, 100, 100); // b, g, r
const Scalar RED = Scalar(0, 0, 255); // b, g, r
const Scalar GREEN = Scalar(0, 255, 0); // b, g, r

int main(int argc, char **argv) {

    const string source = argv[1];
    VideoCapture input(source);
    if(!input.isOpened()) {
        cout << "could not open video file " << source << endl;
        return -1;
    }

    string::size_type pAt = source.find_last_of('.');
    const string NAME = source.substr(0, pAt) + "_trainer.avi";
    int ex = static_cast<int>(input.get(CV_CAP_PROP_FOURCC));

    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};

    Size S = Size((int) input.get(CV_CAP_PROP_FRAME_WIDTH),
                  (int) input.get(CV_CAP_PROP_FRAME_HEIGHT));

    VideoWriter output;
    const string window = "trainer";
    namedWindow(window, WINDOW_AUTOSIZE);
    output.open(NAME, ex, input.get(CV_CAP_PROP_FPS), S, true);

    if (!output.isOpened())
    {
        cout  << "could not open the output video for write " << NAME << endl;
        return -1;
    }

    cout << "Input frame resolution: Width=" << S.width << "  Height=" << S.height
         << " of nr#: " << input.get(CV_CAP_PROP_FRAME_COUNT) << endl;
    cout << "Input codec type: " << EXT << endl;

    Mat src, res, disp;
    int frames = input.get(CV_CAP_PROP_FRAME_COUNT);
    int cur = 0;

    for(;;) {
        input >> src; // next frame
        if(src.empty()) break;
   
        disp = Mat(src); 
        ostringstream os;
        os << cur << " / " << frames;
        putText(disp, os.str(), Point(0, disp.rows - disp.rows / 10), FONT_HERSHEY_SIMPLEX, 1.5, BLACK, 2);

        imshow(window, src);
        int key = waitKey(-1);
        Scalar color = RED;
        switch(key) {
            case 'j':
                color = BLACK;
                break;
            case 'k':
                color = WHITE;
                break;
        }

        res = Mat(src);
        rectangle(res, Point(0, 0), Point(10, 10), color, CV_FILLED);

        cur++;
        output << res;
    }

    cout << "Done" << endl;
    return 0;

}
