/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/

#include <sstream>
#include <string>
#include <iostream>
#include <highgui.h>
#include <cv.h>
#include <time.h>

#include "Table.h"
#include "Fps.h"

using namespace cv;


//initial min and max HSV filter values.
int H_MIN = 22;
int H_MAX = 45;
int S_MIN = 83;
int S_MAX = 200;
int V_MIN = 135;
int V_MAX = 237;

//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;

//minimum and maximum object area
const int MIN_OBJECT_AREA = 4*4;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

void createTrackbars()
{
    const string trackbarWindowName = "Trackbars";

    namedWindow(trackbarWindowName, 0);

    //create memory to store trackbar name on window
    char TrackbarName[50];
    sprintf(TrackbarName, "H_MIN", H_MIN);
    sprintf(TrackbarName, "H_MAX", H_MAX);
    sprintf(TrackbarName, "S_MIN", S_MIN);
    sprintf(TrackbarName, "S_MAX", S_MAX);
    sprintf(TrackbarName, "V_MIN", V_MIN);
    sprintf(TrackbarName, "V_MAX", V_MAX);

    createTrackbar("H_MIN", trackbarWindowName, &H_MIN, 255);
    createTrackbar("H_MAX", trackbarWindowName, &H_MAX, 255);
    createTrackbar("S_MIN", trackbarWindowName, &S_MIN, 255);
    createTrackbar("S_MAX", trackbarWindowName, &S_MAX, 255);
    createTrackbar("V_MIN", trackbarWindowName, &V_MIN, 255);
    createTrackbar("V_MAX", trackbarWindowName, &V_MAX, 255);
}

void morphOps(Mat &thresh)
{
    //create structuring element that will be used to "dilate" and "erode" image.
    //the element chosen here is a 3px by 3px rectangle
    Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
    erode(thresh,thresh,erodeElement);
//     erode(thresh,thresh,erodeElement);

    //dilate with larger element so make sure object is nicely visible
    Mat dilateElement = getStructuringElement(MORPH_RECT, Size(4, 4));
    dilate(thresh,thresh,dilateElement);
//     dilate(thresh,thresh,dilateElement);
}

void trackBall(int &x, int &y, Mat HSV, Mat &cameraFeed)
{
    Mat threshold;

    //filter HSV image between values and store filtered image to threshold matrix
    inRange(HSV, Scalar(H_MIN,S_MIN,V_MIN), Scalar(H_MAX,S_MAX,V_MAX), threshold);

    //perform morphological operations on thresholded image to eliminate noise
    //and emphasize the filtered object(s)
    morphOps(threshold);

    imshow("Thresholded Image", threshold);

    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //find contours of filtered image using openCV findContours function
    findContours(threshold, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    //use moments method to find our filtered object
    double refArea = 0;
    bool ballFound = false;
    if (hierarchy.size() > 0) {
        int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if (numObjects < MAX_NUM_OBJECTS) {
            for (int index = 0; index >= 0; index = hierarchy[index][0]) {

                Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;

                //if the area is less than 20 px by 20px then it is probably just noise
                //if the area is the same as the 3/2 of the image size, probably just a bad filter
                //we only want the object with the largest area so we safe a reference area each
                //iteration and compare it to the area in the next iteration.
                if (area > MIN_OBJECT_AREA && area < MAX_OBJECT_AREA && area > refArea) {
                    x = moment.m10 / area;
                    y = moment.m01 / area;
                    std::cout << "area " << area << std::endl;
                    ballFound = true;
                    refArea = area;
                } else {
                    ballFound = false;
                }
            }

            if (ballFound == true){
                //draw ball location on screen
                circle(cameraFeed, Point(x, y), 20, Scalar(0, 255, 0), 2);
            }

        } else {
            putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0,50), 1, 2, Scalar(0,0,255), 2);
        }
    }
}

double frameDiffTime(Interval fullInterval, unsigned frameNum, int expectedFrameTicks)
{
    int diff = (fullInterval.value() - expectedFrameTicks * frameNum);
    return ((double)diff) * 1000. / cv::getTickFrequency();
}

#define DEBUGPERF(x)

int main(int argc, char* argv[])
{
    Mat cameraFeed;
    Mat capturedFrame;
    Mat HSV;
    Mat threshold;

    //x and y values for the location of the ball
    int x=-1, y=-1;

    Table table;

    //create slider bars for HSV filtering
    createTrackbars();

    VideoCapture capture;

//     capture.open(0);
//     capture.open("/home/niko/Dropbox/VID_20150116_122947.mp4");
// //     capture.open("http://192.168.0.27:8080/?action=stream&dummy=param.mjpg");
    if (argc < 1) {
        std::cerr << "missing file argument" << std::endl;
        return 1;
    }

    bool dropLateFrames = false;
    if (strcmp(argv[1], "-") == 0) {
        dropLateFrames = true;
        capture.open("/dev/stdin");
    } else {
        capture.open(argv[1]);
    }

    double videoWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    double videoHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    double videoFps = capture.get(CV_CAP_PROP_FPS);

    int expectedFrameTicks = 0;
    if (dropLateFrames) {
        Interval initialInterval;
        int slowCalls = 0;
        int initialGrabs = 0;
        for (int i=0; i< 1000; i++) {
            initialGrabs++;
            double v = initialInterval.valueAsMSecAndReset();
            if (v > 8.) {
                slowCalls++;
            }
            capture.grab();
            if (slowCalls > 20) break;
        }
        std::cout << "grabbed " << initialGrabs << " initial buffered frames" << std::endl;

        initialInterval.reset();
        int64 xx = cv::getTickCount();
        double exec_time = (double)getTickCount();
        for (int i=0; i< 500; i++) {
            capture.grab();
        }
        expectedFrameTicks = (initialInterval.value() / 500);
        double expectedFrameTime = (initialInterval.valueAsMSec() / 500.);
        std::cout << "time for frame: " << expectedFrameTicks << " ticks = " << expectedFrameTime << " ms = " << 1000. / expectedFrameTime << "fps" << std::endl;
        expectedFrameTicks = expectedFrameTicks * 1.01; //plus 1% to avoid dropping to many frames
    } else {
        //skip first few frames to wait for correct white balance of camera
        for (int i=0; i< 200; i++) {
            capture.grab();
        }
    }

                                           //fps
    const int showFrameAfterTicks = ((1000 / 25) * cv::getTickFrequency()) / 1000;

    bool paused = false;
    bool slowMotion = false;

    Fps fps;

    int pauseAtFrameNum = -1;
    pauseAtFrameNum = 1;

    Interval lastShownFrameInterval;
    Interval fullInterval;
    unsigned frameNum = 0;
    int64 lastShownFrameTick = -1;
    while (1) {

        Interval frameInterval;
        DEBUGPERF( Interval perfInterval; )

        if (!paused) {
            frameNum++;

            if (dropLateFrames) {
                int fastCalls = 0;
                while (frameDiffTime(fullInterval, frameNum, expectedFrameTicks) > 8.) {
                    double diffTime = frameDiffTime(fullInterval, frameNum, expectedFrameTicks);
                    std::cout << "dropping frame, " << diffTime << "ms too old" << std::endl;
                    //std::cout << "dropping frame, tick " << (fullInterval.value() / frameNum) << " expected " << expectedFrameTicks << " msecs=" << fullInterval.valueAsMSec() / frameNum << std::endl;
                    frameNum++;
                    Interval dropInterval;
                    capture.grab();
                    if (dropInterval.valueAsMSec() < 8.) {
                        fastCalls++;
                        if (fastCalls > 3) {
                            //got three frames in < 8ms
                            frameNum = (fullInterval.value() / expectedFrameTicks) + 10;
                            break;
                        }
                    }
                }
            }

            //store image to matrix
            capture.read(cameraFeed);
            DEBUGPERF( std::cout << "read " << perfInterval.valueAsMSecAndReset() << "ms" << std::endl; )
            fps.update();

            table.addFrame(cameraFeed);
            DEBUGPERF( std::cout << "addFrame " << perfInterval.valueAsMSecAndReset() << "ms" << std::endl; )

            //convert frame from BGR to HSV colorspace
            cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
            DEBUGPERF( std::cout << "2HSV " << perfInterval.valueAsMSecAndReset() << "ms" << std::endl; )

            cameraFeed.copyTo(capturedFrame);

        } else {
            capturedFrame.copyTo(cameraFeed);

            //convert frame from BGR to HSV colorspace
            cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
            DEBUGPERF( std::cout << "2HSV " << perfInterval.valueAsMSecAndReset() << "ms" << std::endl; )
        }


        Rect tableRect =  table.findTable(HSV);
        if (tableRect.width) {
            cameraFeed = cameraFeed(tableRect);
            HSV = HSV(tableRect);

            trackBall(x, y, HSV, cameraFeed);
            DEBUGPERF( std::cout << "trackBall " << perfInterval.valueAsMSecAndReset() << "ms" << std::endl; )


            if (!paused && x != -1 && y != -1) {
                table.addPosition(x, y);
                DEBUGPERF( std::cout << "addPostion " << interval.valueAsMSecAndReset() << "ms" << std::endl; )
            }
        }

        if (frameNum == pauseAtFrameNum) paused = true;

        //show frames at 25fps, don't show if video processed in higher fps
        if (lastShownFrameTick == -1 || cv::getTickCount() > lastShownFrameTick+showFrameAfterTicks) {
            lastShownFrameTick = cv::getTickCount();

            table.paint(cameraFeed);
            DEBUGPERF( std::cout << "table paint " << interval.valueAsMSecAndReset() << "ms" << std::endl; )
            char buffer[33];
            sprintf(buffer, "%d fps, frame %d", fps.get(), frameNum);
            //std::cout << buffer << std::endl;
            putText(cameraFeed, buffer, Point(0, 20), 2, 0.5, Scalar(0,255,0), 2);

            imshow("Wuzl Cam", cameraFeed);
    //         imshow("HSV Image", HSV);
            DEBUGPERF( std::cout << "show image " << interval.valueAsMSecAndReset() << "ms" << std::endl; )


            //delay 30ms so that screen can refresh.
            //image will not appear without this waitKey() command
            char e;
            if (dropLateFrames) {
                e = waitKey(1);
            } else {
                int frameDuration = 1000 / 25/*fps*/;
                if (slowMotion) frameDuration *= 3;
//                 std::cout << frameDuration << " " << lastShownFrameInterval.valueAsMSec();
                frameDuration -= (int)lastShownFrameInterval.valueAsMSec();
                if (frameDuration < 1) frameDuration = 1;
                std::cout << " wait " << frameDuration;
                e = waitKey(frameDuration);
                std::cout << " now at " << lastShownFrameInterval.valueAsMSec() << std::endl;
                lastShownFrameInterval.reset();
            }
            DEBUGPERF( std::cout << "waitKey " << perfInterval.valueAsMSecAndReset() << "ms" << std::endl; )

            // toggle pause with 'p' or ' '
            if( e=='p') paused = ! paused;
            // slow motion with 's'
            if (e=='s') {
                slowMotion = ! slowMotion;
            }
            // reverse with 'r'
            if (e=='r') {
                frameNum--;
                paused = true;
                Mat* f = table.popFrame();
                f->copyTo(capturedFrame);
                delete f;
            }
            // step with ' '
            if (e==' ') {
                paused = false;
                pauseAtFrameNum = frameNum+1;
            }
            // close video with 'esc' or 'q'
            if( e==27 || e=='q' ) break;
        }
    }

    return 0;
}
