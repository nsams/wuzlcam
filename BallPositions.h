/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef BALLPOSITIONS_H
#define BALLPOSITIONS_H

#include "BallPosition.h"

#include <vector>

namespace cv {
    class Mat;
}

class BallPositions
{
private:
    std::vector<BallPosition> _positions;
public:
    BallPositions() {};

    void paint(cv::Mat &frame) const;
    void add(int x, int y);

    int size();
    BallPosition at(int index);
};

#endif
