/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef EVENT_H
#define EVENT_H

#include "Bar.h"
#include <string>

class Event {
public:
    enum EventType {
        SHOT,
        TOUCH,
        GOAL,
        OUT
    };

    Event(EventType _type, Bar* _byBar)
        : type(_type), byBar(_byBar) {}

    EventType type;
    Bar* byBar;

    std::string toString();
};

#endif
