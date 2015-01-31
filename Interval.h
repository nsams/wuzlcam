/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef INTERVAL_H
#define INTERVAL_H

#include <cv.h>


class Interval
{

public:
    // Ctor
    inline Interval() : initial_(cv::getTickCount())
    {
    }

    // Dtor
    virtual ~Interval()
    {
    }

    inline int64 value() const
    {
        return cv::getTickCount()-initial_;
    }

    inline double valueAsMSec() const
    {
        return ((double)value()) * 1000. / cv::getTickFrequency();
    }

    inline unsigned int valueAndReset()
    {
        int64 v = cv::getTickCount();
        unsigned int ret = v-initial_;
        initial_ = v;
        return ret;
    }

    inline double valueAsMSecAndReset()
    {
        double ret = valueAsMSec();
        reset();
        return ret;
    }

    inline void reset()
    {
        initial_ = cv::getTickCount();
    }

private:
    int64 initial_;
};


#endif
