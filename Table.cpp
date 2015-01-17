/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#include "Table.h"

#include <cv.h>

using namespace cv;
Table::Table()
{
    _bars.push_back(new Bar(1, 0, Bar::GOALIE, 1620, 1580));
    _bars.push_back(new Bar(2, 0, Bar::DEFENSE, 1330, 1250));
    _bars.push_back(new Bar(3, 1, Bar::STRIKER, 1100, 930));
    _bars.push_back(new Bar(4, 0, Bar::MIDFIELD, 885, 664));
    _bars.push_back(new Bar(5, 1, Bar::MIDFIELD, 703, 432));
    _bars.push_back(new Bar(6, 0, Bar::STRIKER, 546+30, 232+30));
    _bars.push_back(new Bar(7, 1, Bar::DEFENSE, 403+30, 55+30));
    _bars.push_back(new Bar(8, 1, Bar::GOALIE, 274+30, -100+30));
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
}


void Table::addPosition(int x, int y)
{
    _detectedPositions.add(x, y);
    _detectEventForLastPosition();
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
            if (!lastEvent || (lastEvent->type != Event::SHOT || lastEvent->byBar != bar)) {
                Event* ev = new Event(Event::SHOT, bar);
                _events.push_back(ev);
                std::cout << "EVENT: " << ev->toString() << std::endl;
            }
        }
    }
}


void Table::paint(Mat& frame)
{

    for(std::vector<Bar*>::iterator it = _bars.begin(); it != _bars.end(); ++it) {
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
    for(std::vector<Bar*>::iterator it = _bars.begin(); it != _bars.end(); ++it) {
        int barK = (1080 - 0) / ((*it)->bottomX - (*it)->topX);
        int barD = 0 - barK * (*it)->topX;
        unsigned barX = (y - barD) / barK;
        unsigned distance = abs(barX - x);
//         std::cout << "bar " << (*it).number << " distance=" << distance << std::endl;
        if (minDistance == -1 || distance < minDistance) {
            ret = *it;
            minDistance = distance;
        }
    }
    return ret;
}

