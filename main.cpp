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

using namespace cv;

//initial min and max HSV filter values.
int H_MIN = 22;
int H_MAX = 33;
int S_MIN = 83;
int S_MAX = 200;
int V_MIN = 142;
int V_MAX = 237;

//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;

//minimum and maximum object area
const int MIN_OBJECT_AREA = 3*3;
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

    createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX);
    createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX);
    createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX);
    createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX);
    createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX);
    createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX);
}

void drawObject(int x, int y, Mat &frame)
{
    circle(frame,Point(x,y),20,Scalar(0,255,0),2);
}

void morphOps(Mat &thresh)
{
    //create structuring element that will be used to "dilate" and "erode" image.
    //the element chosen here is a 3px by 3px rectangle
    Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
    erode(thresh,thresh,erodeElement);
    erode(thresh,thresh,erodeElement);

    //dilate with larger element so make sure object is nicely visible
    Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));
    dilate(thresh,thresh,dilateElement);
    dilate(thresh,thresh,dilateElement);
}

void trackBall(int &x, int &y, Mat threshold, Mat &cameraFeed){

    Mat temp;
    threshold.copyTo(temp);

    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //find contours of filtered image using openCV findContours function
    findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

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
                    ballFound = true;
                    refArea = area;
                } else {
                    ballFound = false;
                }
            }

            if (ballFound == true){
                putText(cameraFeed, "Tracking Ball", Point(0,50), 2, 1, Scalar(0,255,0), 2);
                //draw ball location on screen
                drawObject(x,y,cameraFeed);
            }

        } else {
            putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0,50), 1, 2, Scalar(0,0,255), 2);
        }
    }
}

int main(int argc, char* argv[])
{
    Mat cameraFeed;
    Mat HSV;
    Mat threshold;

    //x and y values for the location of the ball
    int x=0, y=0;

    Table table;

    //create slider bars for HSV filtering
    //createTrackbars();

    VideoCapture capture;

//     capture.open(0);
//     capture.open("/home/niko/Dropbox/VID_20150116_122947.mp4");
//     capture.open("http://192.168.0.27:8080/?action=stream&dummy=param.mjpg");
    capture.open("/dev/stdin");

    capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

    bool paused = false;
    time_t startTime = time(0);

    unsigned frameNum = 0;
    while (1) {

        int runTime = time(0)-startTime;
        if (frameNum % 10 == 0 && runTime) {
            std::cout << "fps " << (frameNum / runTime) << std::endl;
        }
        if (!paused) {
            frameNum++;
            //store image to matrix
            capture.read(cameraFeed);
            table.addFrame(cameraFeed);

            //convert frame from BGR to HSV colorspace
            cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
        }

//         if (frameNum > 30) paused = true;

        //filter HSV image between values and store filtered image to threshold matrix
        inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);

        //perform morphological operations on thresholded image to eliminate noise
        //and emphasize the filtered object(s)
        morphOps(threshold);

        //pass in thresholded frame to our object tracking function
        //this function will return the x and y coordinates of the
        //filtered object
        trackBall(x, y, threshold, cameraFeed);

        if (!paused) {
            table.addPosition(x, y);
            table.paint(cameraFeed);
        }

        //show frames
//         imshow("Thresholded Image", threshold);
        imshow("Wuzl Cam", cameraFeed);
//         imshow("HSV Image", HSV);


        //delay 30ms so that screen can refresh.
        //image will not appear without this waitKey() command
        char e = cvWaitKey(1);

        // toggle pause with 'p' or ' '
        if( e=='p' || e==' ') paused = ! paused;
        // close video with'esc'
        if( e==27 ) break;
    }

    return 0;
}
