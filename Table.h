/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef TABLE_H
#define TABLE_H

#include "BallPositions.h"
#include "Bar.h"
#include "Event.h"

namespace cv {
    class Mat;
}

class Table {

public:
    Table();
    ~Table();
    void paint(cv::Mat &frame);
    void addPosition(int x, int y);

private:
    void _detectEventForLastPosition();

    Bar* _getNearestBar(BallPosition position);
    Bar* _getNearestBar(int x, int y);

    BallPositions _detectedPositions;
    std::vector<Bar*> _bars;
    std::vector<Event*> _events;
};

#endif
