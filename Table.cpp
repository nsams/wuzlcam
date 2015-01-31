/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#include "Table.h"

#include <cv.h>
#include <highgui.h>

using namespace cv;
Table::Table(): _updateTableIn(0)
{
    _bars.push_back(new Bar(1, 0, Bar::GOALIE, 40, 40));
    _bars.push_back(new Bar(2, 0, Bar::DEFENSE, 106, 106));
    _bars.push_back(new Bar(3, 1, Bar::STRIKER, 174, 174));
    _bars.push_back(new Bar(4, 0, Bar::MIDFIELD, 244, 244));
    _bars.push_back(new Bar(5, 1, Bar::MIDFIELD, 315, 315));
    _bars.push_back(new Bar(6, 0, Bar::STRIKER, 380, 380));
    _bars.push_back(new Bar(7, 1, Bar::DEFENSE, 450, 450));
    _bars.push_back(new Bar(8, 1, Bar::GOALIE, 518, 518));
}

Table::~Table()
{
    while(!_bars.empty()) {
        delete _bars.back();
        _bars.pop_back();
    }

    while(!_events.empty()) {
        delete _events.back();
        _events.pop_back();
    }

    while(!_lastFrames.empty()) {
        delete _lastFrames.front();
        _lastFrames.pop_front();
    }
}

void Table::addPosition(int x, int y)
{
    _detectedPositions.add(x, y);
    _detectEventForLastPosition();
}

void Table::addFrame(const cv::Mat &frame)
{
    Mat *temp = new Mat();
    frame.copyTo(*temp);
    if (_lastFrames.size() > 90*1) {
        delete _lastFrames.front();
        _lastFrames.pop_front();
    }
    _lastFrames.push_back(temp);
}

Mat *Table::popFrame()
{
    if (!_lastFrames.empty()) {
        Mat *ret = _lastFrames.back();
        _lastFrames.pop_back();
        _detectedPositions.popLast();
        return ret;
    }
    return 0;
}



void Table::_detectEventForLastPosition()
{
/*
what we have to detect:
- shot                     [done]
  - by bar
- touch/slow down
  - by bar
- touch/change direction
  - by bar
- touch edge
- goal
  - by bar
  - which goal
- out
  - by bar
 */

    unsigned sz = _detectedPositions.size();
    if (sz > 1) {
        BallPosition curPos = _detectedPositions.at(sz-1);
        BallPosition prevPos = _detectedPositions.at(sz-2);
        unsigned speedChange = abs(curPos.speed - prevPos.speed);
    //             std::cout << "speed=" << diff << " prevSpeed" << prevSpeed << " speedChange=" << speedChange << std::endl;
        Event* lastEvent = 0;
        if (!_events.empty()) lastEvent = _events.back();
        if (speedChange > 50) {
            //std::cout << "SHOT!!! at frame " << curPos.frameNum << " speedChange=" << speedChange << std::endl;
//             std::cout << "bar number " << _getNearestBar(prevPos)->toString() << std::endl;
            Bar* bar = _getNearestBar(prevPos);
            if (bar && (!lastEvent || (lastEvent->type != Event::SHOT || lastEvent->byBar != bar))) {
                Event* ev = new Event(Event::SHOT, bar);
                _events.push_back(ev);
                std::cout << "EVENT: " << ev->toString() << std::endl;
                //playbackLastFrames();
            }
        }
    }
}


void Table::paint(Mat& frame) const
{

    for(std::vector<Bar*>::const_iterator it = _bars.begin(); it != _bars.end(); ++it) {
        (*it)->paint(frame);
    }

    _detectedPositions.paint(frame);
}

Bar* Table::_getNearestBar(BallPosition position)
{
    return _getNearestBar(position.x, position.y);
}

Bar* Table::_getNearestBar(int x, int y)
{
    Bar* ret = 0;
    int minDistance = -1;
    for (std::vector<Bar*>::const_iterator it = _bars.begin(); it != _bars.end(); ++it) {
        unsigned barX = (*it)->topX;
        if ((*it)->bottomX != (*it)->topX) {
            int barK = (480 - 0) / ((*it)->bottomX - (*it)->topX);
            int barD = 0 - barK * (*it)->topX;
            barX = (y - barD) / barK;
        }
        unsigned distance = abs(barX - x);
//         std::cout << "bar " << (*it).number << " distance=" << distance << std::endl;
        if (minDistance == -1 || distance < minDistance) {
            ret = *it;
            minDistance = distance;
        }
    }
    return ret;
}

void Table::playbackLastFrames() const
{
    for (std::deque<cv::Mat*>::const_iterator it = _lastFrames.begin(); it != _lastFrames.end(); ++it) {
        cv::Mat temp;
        (**it).copyTo(temp);
        putText(temp, "Slow Motion", Point(0,50), 2, 1, Scalar(0,255,0), 2);
        cv::imshow("Wuzl Cam", temp);
        char e = cvWaitKey(99);
        if (e == 27) break;
    }
}

cv::Rect Table::findTable(Mat HSV)
{
    if (_updateTableIn > 0) {
        return _lastTablePosition;
    }

    Mat threshold;
    inRange(HSV, Scalar(22, 126, 134), Scalar(66, 213, 238), threshold);

    //create structuring element that will be used to "dilate" and "erode" image.
    //the element chosen here is a 3px by 3px rectangle
    Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
    erode(threshold,threshold,erodeElement);
    erode(threshold,threshold,erodeElement);

    //dilate with larger element so make sure object is nicely visible
    Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));
    dilate(threshold,threshold,dilateElement);
    dilate(threshold,threshold,dilateElement);


    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //find contours of filtered image using openCV findContours function
    findContours(threshold, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    //use moments method to find our filtered object
    int cornersFound = 0;
    int numObjects = hierarchy.size();

    if (numObjects >= 4) {
        Point topLeft;
        Point bottomLeft;
        Point topRight;
        Point bottomRight;
        for (int index = 0; index >= 0; index = hierarchy[index][0]) {

            Moments moment = moments((cv::Mat)contours[index]);
            double area = moment.m00;

            //if the area is less than 20 px by 20px then it is probably just noise
            //if the area is the same as the 3/2 of the image size, probably just a bad filter
            //we only want the object with the largest area so we safe a reference area each
            //iteration and compare it to the area in the next iteration.
            const int minCornerMarkerArea = 29*29; //TODO these are only for 640 with
            const int maxCornerMarkerArea = 45*45;
            if (area > minCornerMarkerArea && area < maxCornerMarkerArea) {
                int x = moment.m10 / area;
                int y = moment.m01 / area;
                int halfRectSize = sqrt(area) / 2;
                bool isValid = true;
                if (x < 100 && y < 100) {
                    //top left
                    topLeft.x = x + halfRectSize;
                    topLeft.y = y - halfRectSize;
                } else if (x < 100 && y > HSV.size().height-200) {
                    //bottom left
                    bottomLeft.x = x + halfRectSize;
                    bottomLeft.y = y + halfRectSize;
                } else if (x > HSV.size().width-100 && y < 100) {
                    //top right
                    topRight.x = x - halfRectSize;
                    topRight.y = y - halfRectSize;
                } else if (x > HSV.size().width-100 && y > HSV.size().height-200) {
                    //bottom right
                    bottomRight.x = x - halfRectSize;
                    bottomRight.y = y + halfRectSize;
                } else {
                    isValid = false;
                }
                if (isValid) {
                    y -= sqrt(area) / 2;
                    cornersFound++;
                    //rectangle(cameraFeed, Point(x,y), Point(x+35,y+35), Scalar(0,255,255), 2);
                }
            }
        }
//         imshow("Thresholded Image Corners", threshold);
        if (cornersFound == 4) {
            _lastTablePosition = Rect(topLeft.x, topLeft.y, bottomRight.x-topLeft.x, bottomRight.y-topLeft.y);
            _updateTableIn = 90; //update every second
            return _lastTablePosition;
        }
    } else {
        std::cout << "TOO MUCH NOISE TO FIND CORNERS! ADJUST FILTER" << std::endl;
    }
    return Rect();
}


