/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#include "Event.h"

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
    }
    ret += " von " + byBar->toString();
    return ret;
}
