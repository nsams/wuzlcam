/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef FPS_H
#define FPS_H

// #include <sys/time.h>
#include <cv.h>

int64 GetTickCount()
{
    return cv::getTickCount();
 }

class Interval
{

public:
    // Ctor
    inline Interval() : initial_(GetTickCount())
    {
    }

    // Dtor
    virtual ~Interval()
    {
    }

    inline int64 value() const
    {
        return GetTickCount()-initial_;
    }

    inline double valueAsMSec() const
    {
        return ((double)value()) * 1000. / cv::getTickFrequency();
    }

    inline unsigned int valueAndReset()
    {
        int64 v = GetTickCount();
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
        initial_ = GetTickCount();
    }

private:
    int64 initial_;
};

class Fps
{

public:
    // Constructor
    Fps() : m_fps(0), m_fpscount(0)
    {
    }

    // Update
    void update()
    {
        // increase the counter by one
        m_fpscount++;

        // one second elapsed? (= 1000 milliseconds)
        if (m_fpsinterval.valueAsMSec() > 1000.0)
        {
            // save the current counter value to m_fps
            m_fps         = m_fpscount;

            // reset the counter and the interval
            m_fpscount    = 0;
            m_fpsinterval = Interval();
        }
    }

    // Get fps
    unsigned int get() const
    {
        return m_fps;
    }

private:
    unsigned int m_fps;
    unsigned int m_fpscount;
    Interval m_fpsinterval;
};

#endif
