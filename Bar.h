/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef BAR_H
#define BAR_H

#include <string>

namespace cv {
    class Mat;
}

class Bar
{
public:
    enum PositionType {
        NONE,
        GOALIE,
        DEFENSE,
        MIDFIELD,
        STRIKER
    };

    Bar();
    Bar(unsigned number, int team, PositionType position, unsigned topX, unsigned bottomX);
    void paint(cv::Mat& frame);

    std::string toString();


    int number;
    int topX;
    int bottomX;

    int team;
    PositionType position;
};

#endif
