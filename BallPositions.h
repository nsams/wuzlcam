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
    std::vector<BallPosition*> _positions;
public:
    BallPositions() {};
    ~BallPositions();

    void paint(cv::Mat &frame) const;
    void add(int x, int y);
    void popLast();

    std::vector<BallPosition*>::iterator begin() { return _positions.begin(); }
    std::vector<BallPosition*>::iterator end() { return _positions.end(); }
    std::vector<BallPosition*>::reverse_iterator rbegin() { return _positions.rbegin(); }
    std::vector<BallPosition*>::reverse_iterator rend() { return _positions.rend(); }
    BallPosition* back() const { return _positions.back(); }

    int size() const;
    BallPosition* at(int index);
};

#endif
