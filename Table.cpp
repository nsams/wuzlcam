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

void Table::addPosition(int x, int y, cv::Mat &frame)
{
    _detectedPositions.add(x, y);
    _detectEventForLastPosition(frame);
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

void Table::_detectEventForLastPosition(cv::Mat &frame)
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

const int GOAL_LINE_BLUE_X = 20;
const int GOAL_LINE_RED_X = 560-20;

    unsigned sz = _detectedPositions.size();
    if (sz > 1) {
        BallPosition *curPos = _detectedPositions.at(sz-1);
        BallPosition *prevPos = _detectedPositions.at(sz-2);
        Event* lastEvent = 0;
        if (!_events.empty()) lastEvent = _events.back();

        int speedChange = curPos->speed - prevPos->speed;
        int speedChangePercent = 0;
        if (curPos->speed) speedChangePercent = speedChange * 100 / curPos->speed;

        if (lastEvent && lastEvent->type == Event::POSITION_BALL_TO_KICKOFF) {
            Bar* bar = _getNearestBar(curPos);
            if (bar && bar->position == Bar::MIDFIELD) {
                if (curPos->speed == 0) {
                    Event* ev = new Event(Event::KICKOFF, bar, curPos);
                    _events.push_back(ev);
                    std::cout << "EVENT: " << ev->toString() << std::endl;
                }
            }
        }

        if (curPos->interpolated && lastEvent && lastEvent->type != Event::GOAL && lastEvent->type != Event::POSITION_BALL_TO_KICKOFF) {
            //ball not visible, probably goal?
            //std::cout << "x: " << curPos->x << " curPos->angle" << curPos->angle << std::endl;
            if (curPos->x < GOAL_LINE_BLUE_X+20 && curPos->angle < 180) {
                Event* ev = new Event(Event::GOAL, 0, curPos);
                _events.push_back(ev);
                std::cout << "EVENT: " << ev->toString() << std::endl;
            } else if (curPos->x > GOAL_LINE_RED_X-20 && curPos->angle > 180) {
                Event* ev = new Event(Event::GOAL, 0, curPos);
                _events.push_back(ev);
                std::cout << "EVENT: " << ev->toString() << std::endl;
            }
        }

        if (lastEvent && lastEvent->type == Event::GOAL) {
            if (curPos->frameNum - lastEvent->atPos->frameNum > 90*2) { //2 seconds passed
                //we had a goal, now kickoff
                Event* ev = new Event(Event::POSITION_BALL_TO_KICKOFF, 0, curPos);
                _events.push_back(ev);
                std::cout << "EVENT: " << ev->toString() << std::endl;
            }
        }

        if (!curPos->interpolated && lastEvent && lastEvent->type == Event::GOAL) {
            //we had a goal, ball came back out
            //std::cout << "x: " << curPos->x << " curPos->angle" << curPos->angle << std::endl;
            if (curPos->x < GOAL_LINE_BLUE_X+30 && curPos->x >= GOAL_LINE_BLUE_X) {
                Event* ev = new Event(Event::GOAL_BACK, 0, curPos);
                _events.push_back(ev);
                std::cout << "EVENT: " << ev->toString() << std::endl;
            } else if (curPos->x > GOAL_LINE_RED_X-30 && curPos->x <= GOAL_LINE_RED_X) {
                Event* ev = new Event(Event::GOAL_BACK, 0, curPos);
                _events.push_back(ev);
                std::cout << "EVENT: " << ev->toString() << std::endl;
            }
        }


        if (!lastEvent || lastEvent->type != Event::POSITION_BALL_TO_KICKOFF) {
            //std::cout << "SPEED: " << "curPos=" << curPos->speed << " prevPos" << prevPos ->speed << " speedChange=" << speedChange << "=" << speedChangePercent << "%" <<  std::endl;
            if (speedChange > 5 && speedChangePercent > 50 && curPos->x > GOAL_LINE_BLUE_X && curPos->x < GOAL_LINE_RED_X) {
    //         std::cout << "SHOT!!! at frame " << curPos->frameNum << " speedChange=" << speedChange << std::endl;
    //             std::cout << "bar number " << _getNearestBar(prevPos)->toString() << std::endl;
                Bar* bar = _getNearestBar(prevPos);
                if (bar && (!lastEvent || (lastEvent->type != Event::SHOT || lastEvent->byBar != bar))) {
                    Event* ev = new Event(Event::SHOT, bar, prevPos);
                    _events.push_back(ev);
                    std::cout << "EVENT: " << ev->toString() << std::endl;
                    //playbackLastFrames();
                }
            }

            double directionChange = abs(curPos->angle - prevPos->angle);
            //std::cout << "DIRECTION: " << curPos->angle <<  " change" << directionChange << std::endl;
            if (directionChange > 20 && curPos->x > GOAL_LINE_BLUE_X && curPos->x < GOAL_LINE_RED_X) {
                Bar* bar = _getNearestBar(prevPos);
                //std::cout << "direction at frame " << curPos->frameNum << " directionChange=" << directionChange << "°" << std::endl;
                if (bar && !lastEvent || !(lastEvent->type == Event::TOUCH && lastEvent->byBar ==bar)) {
                    Event* ev = new Event(Event::TOUCH, bar, prevPos);
                    _events.push_back(ev);
                    std::cout << "EVENT: " << ev->toString() << std::endl;
                }
            }
        }

    }
}


void Table::paint(Mat& frame) const
{

    for(std::vector<Bar*>::const_iterator it = _bars.begin(); it != _bars.end(); ++it) {
        (*it)->paint(frame);
    }


    for(std::vector<Event*>::const_reverse_iterator it = _events.rbegin(); it != _events.rend(); ++it) {
        if (_detectedPositions.back()->frameNum - (*it)->atPos->frameNum > 100) {
            break;
        }
        (*it)->paint(frame);
    }

    _detectedPositions.paint(frame);

    if (_detectedPositions.size()) {
        char buffer[33];
        const float TALBE_WIDTH = 1.500; //in meters
        float speed = ((float)_detectedPositions.back()->speed) * TALBE_WIDTH / 560 * 90 * 3.6;
        sprintf(buffer, "%.1f km/h", speed);
        putText(frame, buffer, Point(0, 40), 2, 0.5, Scalar(255,0,0), 2);
    }

    line(frame, Point(20, 0), Point(20, 1080), Scalar(255, 0, 0), 2);
    line(frame, Point(frame.cols-20, 0), Point(frame.cols-20, 1080), Scalar(255, 0, 0), 2);
}

Bar* Table::_getNearestBar(BallPosition* position)
{
    return _getNearestBar(position->x, position->y);
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

// based on http://www.pyimagesearch.com/2014/08/25/4-point-opencv-getperspective-transform-example/
void Table::_perspectiveTransformTable(Mat& HSV, Mat& cameraFeed)
{
    Point tl = _lastTablePositionTopLeft;
    Point tr = _lastTablePositionTopRight;
    Point br = _lastTablePositionBottomRight;
    Point bl = _lastTablePositionBottomLeft;

    Point2f rect[4];
    rect[0] = tl;
    rect[1] = tr;
    rect[2] = br;
    rect[3] = bl;
/*
    //compute the width of the new image, which will be the
    //maximum distance between bottom-right and bottom-left
    //x-coordiates or the top-right and top-left x-coordinates
    int widthA = sqrt(pow(br.x - bl.x, 2) + pow(br.x - bl.x, 2));
    int widthB = sqrt(pow(tr.x - tl.x, 2) + pow(tr.x - tl.x, 2));
    int maxWidth = max(widthA, widthB);

    // compute the height of the new image, which will be the
    // maximum distance between the top-right and bottom-right
    // y-coordinates or the top-left and bottom-left y-coordinates
    int heightA = sqrt(pow(tr.y - br.y, 2) + pow(tr.y - br.y, 2));
    int heightB = sqrt(pow(tl.y - bl.y, 2) + pow(tl.y - bl.y, 2));
    int maxHeight = max(heightA, heightB);
*/
    int maxWidth = 560;
    int maxHeight = 320;


    // now that we have the dimensions of the new image, construct
    // the set of destination points to obtain a "birds eye view",
    // (i.e. top-down view) of the image, again specifying points
    // in the top-left, top-right, bottom-right, and bottom-left
    // order
    Point2f dst[4];
    dst[0] = Point2f(0, 0);
    dst[1] = Point2f(maxWidth - 1, 0);
    dst[2] = Point2f(maxWidth - 1, maxHeight - 1);
    dst[3] = Point2f(0, maxHeight - 1);

    // compute the perspective transform matrix and then apply it
    Mat matrix = cv::getPerspectiveTransform(rect, dst);
    cv::warpPerspective(HSV, HSV, matrix, Size(maxWidth, maxHeight));
    cv::warpPerspective(cameraFeed, cameraFeed, matrix, Size(maxWidth, maxHeight));
}


bool Table::findTable(cv::Mat &HSV, cv::Mat &cameraFeed)
{
    if (_updateTableIn > 0) {
        _perspectiveTransformTable(HSV, cameraFeed);
        return true;
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
            _lastTablePositionTopLeft = topLeft;
            _lastTablePositionTopRight = topRight;
            _lastTablePositionBottomRight = bottomRight;
            _lastTablePositionBottomLeft = bottomLeft;
            _updateTableIn = 90; //update every second
            _perspectiveTransformTable(HSV, cameraFeed);
            return true;
        }
    } else {
        std::cout << "TOO MUCH NOISE TO FIND CORNERS! ADJUST FILTER" << std::endl;
    }
    return false;
}


