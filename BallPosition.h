/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef BALLPOSITION_H
#define BALLPOSITION_H

class BallPosition {
public:
    BallPosition(unsigned _x, unsigned _y, unsigned _frameNum, unsigned _speed)
        : x(_x), y(_y), frameNum(_frameNum), speed(_speed)  {}
    unsigned x;
    unsigned y;
    unsigned frameNum;
    unsigned speed;

    unsigned diffTo(BallPosition p);
    unsigned diffTo(unsigned x_, unsigned y_);
};

#endif
