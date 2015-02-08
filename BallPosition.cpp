/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#include "BallPosition.h"
#include <math.h>
#include <cstdlib>

unsigned int BallPosition::diffTo(unsigned int x_, unsigned int y_) const
{
    unsigned diffX = abs(x - x_);
    unsigned diffY = abs(y - y_);
    return sqrt(diffX*diffX + diffY*diffY);
}


unsigned int BallPosition::diffTo(const BallPosition *p) const
{
    return diffTo(p->x, p->y);
}

double BallPosition::angleTo(const BallPosition* p) const
{
    return angleTo(p->x, p->y);
}

double BallPosition::angleTo(unsigned int x_, unsigned int y_) const
{
    if ((x - x_) == 0) {
        if (y > y_) return 180;
        return 0;
    }
    double ret =  atan(((double)y - (double)y_) / ((double)x - (double)x_));
    ret *= 180 / M_PI;
    ret += 90;
    if (y > y_) ret += 180;
    return ret;
}

