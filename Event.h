/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef EVENT_H
#define EVENT_H

#include "Bar.h"
#include <string>

class BallPosition;

class Event {
public:
    enum EventType {
        SHOT,
        TOUCH,
        GOAL,
        GOAL_BACK,
        OUT,
        OPENING
    };

    Event(EventType _type, Bar* _byBar, BallPosition * _atPos)
        : type(_type), byBar(_byBar), atPos(_atPos) {}

    EventType type;
    Bar* byBar;
    BallPosition* atPos;

    void paint(cv::Mat &frame) const;

    std::string toString();
};

#endif
