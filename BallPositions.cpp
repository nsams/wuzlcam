/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#include "BallPositions.h"
#include <cv.h>

using namespace cv;

void BallPositions::add(int x, int y)
{
    unsigned speed = 0;
    if (!_positions.empty()) {
        speed = _positions.back().diffTo(x, y);
    }
    _positions.push_back(
        BallPosition(x, y, _positions.size(), speed)
    );
}

void BallPositions::popLast()
{
    _positions.pop_back();
}

void BallPositions::paint(Mat& frame) const
{
    unsigned sz = _positions.size();
    unsigned lengthPainted = 0;
//         std::cout << "**********" << _positions.back().frameNum << std::endl;
    for (unsigned i = sz; i-- > 1; ) {
        BallPosition p1 = _positions.at(i);
        BallPosition p2 = _positions.at(i-1);
        line(frame, Point(p1.x, p1.y), Point(p2.x, p2.y), Scalar(0,255,0), 2);
        lengthPainted += p1.speed;
        if (lengthPainted > 300) break;
    }
}

BallPosition BallPositions::at(int index)
{
    return _positions.at(index);
}

int BallPositions::size()
{
    return _positions.size();
}

