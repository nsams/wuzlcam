/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef TABLE_H
#define TABLE_H

#include "BallPositions.h"
#include "Bar.h"
#include "Event.h"

#include <deque>
#include <cv.h>

namespace cv {
    class Mat;
}

class Table {

public:
    Table();
    ~Table();
    void paint(cv::Mat &frame) const;
    void addPosition(int x, int y, cv::Mat& frame);
    void addFrame(const cv::Mat &frame);
    cv::Mat* popFrame();
    void playbackLastFrames() const;

    bool findTable(cv::Mat &HSV, cv::Mat &cameraFeed); //modified HSV and cameraFeed

private:
    void _detectEventForLastPosition(cv::Mat& frame);

    Bar* _getNearestBar(BallPosition *position);
    Bar* _getNearestBar(int x, int y);

    BallPositions _detectedPositions;
    std::vector<Bar*> _bars;
    std::vector<Event*> _events;
    std::deque<cv::Mat*> _lastFrames;

    int _updateTableIn;
    cv::Point _lastTablePositionTopLeft;
    cv::Point _lastTablePositionTopRight;
    cv::Point _lastTablePositionBottomRight;
    cv::Point _lastTablePositionBottomLeft;

    //apply lastTablePosition points to perspective transform input
    void _perspectiveTransformTable(cv::Mat &HSV, cv::Mat &cameraFeed);
};

#endif
