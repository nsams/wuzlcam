/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#include "Event.h"

#include "BallPosition.h"
#include <cv.h>



using namespace cv;

std::string Event::toString()
{
    std::string ret;
    switch (type) {
        case SHOT:
            ret = "Schuß";
            break;
        case TOUCH:
            ret ="Beühruhrung";
            break;
        case GOAL:
            ret = "Tor";
            break;
        case GOAL_BACK:
            ret = "Tor heraus";
            break;
        case KICKOFF:
            ret = "Anstoß";
            break;
        case POSITION_BALL_TO_KICKOFF:
            ret = "Ball zum Anstoß positionieren";
            break;
    }
    if (byBar) ret += " von " + byBar->toString();
    return ret;
}

void Event::paint(cv::Mat& frame) const
{
    Scalar color;
    switch (type) {
        case SHOT:
            color =  Scalar(0, 0, 255);
            break;
        case TOUCH:
            color = Scalar(0, 255, 0);
            break;
    }
    circle(frame, Point(atPos->x, atPos->y), 10, color, 2);
}
