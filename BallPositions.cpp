/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#include "BallPositions.h"
#include <cv.h>

using namespace cv;

BallPositions::~BallPositions()
{
    while(!_positions.empty()) {
        delete _positions.back();
        _positions.pop_back();
    }
}
void BallPositions::add(int x, int y)
{
    if (x == -1 && _positions.empty()) return;

    bool interpolated = false;
    unsigned speed = 0;
    double angle = 0;
    if (x == -1) {
        x = _positions.back()->x;
        y = _positions.back()->y;
        speed = _positions.back()->speed;
        angle = _positions.back()->angle;
        interpolated = true;
    } else {
        int interpolatedSteps = 0;
        for (std::vector<BallPosition*>::reverse_iterator it = _positions.rbegin(); it != _positions.rend(); ++it) {
            interpolatedSteps++;
            if (!(*it)->interpolated) {
                speed = (*it)->diffTo(x, y) / interpolatedSteps;
                break;
            }
        }

        unsigned sz = _positions.size();
        BallPosition *pos1 = 0;
        for (unsigned i = sz; i-- > 1; ) {
            BallPosition *p = _positions.at(i);
            if (p->diffTo(_positions.back()) > 10) {
                pos1 = p;
                break;
            }
        }
        if (pos1) {
            angle = pos1->angleTo(x, y);
        }
    }
    _positions.push_back(
        new BallPosition(x, y, _positions.size(), speed, angle, interpolated)
    );
}

void BallPositions::popLast()
{
    delete _positions.back();
    _positions.pop_back();
}

void BallPositions::paint(Mat& frame) const
{
    unsigned sz = _positions.size();
    if (!sz) return;
    unsigned lengthPainted = 0;
//         std::cout << "**********" << _positions.back().frameNum << std::endl;
    for (unsigned i = sz; i-- > 1; ) {
        BallPosition *p1 = _positions.at(i);
        BallPosition *p2 = _positions.at(i-1);
        line(frame, Point(p1->x, p1->y), Point(p2->x, p2->y), Scalar(0,255,0), 2);
        lengthPainted += p1->speed;
        if (lengthPainted > 300) break;
    }

    //***paint direction
    BallPosition *pos1 = 0;
    for (unsigned i = sz-1; i-- > 1; ) {
        BallPosition *p = _positions.at(i);
        if (p->diffTo(_positions.back()) > 10) {
            pos1 = p;
            break;
        }
    }
    if (pos1) {
        line(frame, Point(_positions.back()->x, _positions.back()->y), Point(pos1->x, pos1->y), Scalar(0, 0, 0), 2);
    }

}

BallPosition *BallPositions::at(int index)
{
    return _positions.at(index);
}

int BallPositions::size() const
{
    return _positions.size();
}

