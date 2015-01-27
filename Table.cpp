/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#include "Table.h"

#include <cv.h>
#include <highgui.h>

using namespace cv;
Table::Table()
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


