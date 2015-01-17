/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#include "BallPosition.h"
#include <math.h>
#include <cstdlib>

unsigned int BallPosition::diffTo(unsigned int x_, unsigned int y_)
{
    unsigned diffX = abs(x - x_);
    unsigned diffY = abs(y - y_);
    return sqrt(diffX*diffX + diffY*diffY);
}


unsigned int BallPosition::diffTo(BallPosition p)
{
    return diffTo(p.x, p.y);
}
