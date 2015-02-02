/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef BALLPOSITION_H
#define BALLPOSITION_H

class BallPosition {
public:
    BallPosition(unsigned _x, unsigned _y, unsigned _frameNum, unsigned _speed, double _angle, bool _interpolated)
        : x(_x), y(_y), frameNum(_frameNum), speed(_speed), angle(_angle), interpolated(_interpolated)  {}
    unsigned x;
    unsigned y;
    unsigned frameNum;
    int speed;
    double angle;
    bool interpolated;

    unsigned diffTo(const BallPosition *p) const;
    unsigned diffTo(unsigned x_, unsigned y_) const;
    double angleTo(const BallPosition* p) const;
    double angleTo(unsigned x_, unsigned y_) const;
};

#endif
