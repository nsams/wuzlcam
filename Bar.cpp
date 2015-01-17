/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#include "Bar.h"

#include <cv.h>

using namespace cv;

Bar::Bar()
    : number(-1), team(-1), position(NONE), topX(-1), bottomX(-1)
{
}

Bar::Bar(unsigned int number_, int team_, PositionType position_, unsigned int topX_, unsigned int bottomX_)
    : number(number_), team(team_), position(position_), topX(topX_), bottomX(bottomX_)
{
}

std::string Bar::toString()
{
    std::string ret;
    if (team == 0) ret += "Blauer";
    else if (team == 1) ret += "Roter";
    ret += " ";

    if (position == GOALIE) ret += "Tormann";
    else if (position == DEFENSE) ret += "Verteidiger";
    else if (position == MIDFIELD) ret += "Mitte";
    else if (position == STRIKER) ret += "Sturm";
    return ret;
}

void Bar::paint(Mat& frame)
{
    line(frame, Point(topX, 0), Point(bottomX, 1080), Scalar(255, 0, 0), 2);
}
