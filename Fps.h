/**
* Copyright (c) 2015 Niko Sams <niko.sams@gmail.com>
* Distributed under the GNU GPL v3. For full terms see the file COPYING.
*/
#ifndef FPS_H
#define FPS_H

#include "Interval.h"

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
        return (
                m_fps //previsous second
                + m_fpscount * (1000. / m_fpsinterval.valueAsMSec()) //fraction of current second
               ) / (m_fps ? 2 : 1); //devided by 2 (because of the two seconds)
    }

private:
    unsigned int m_fps;
    unsigned int m_fpscount;
    Interval m_fpsinterval;
};

class FpsCap : public Fps
{
public:
    FpsCap(int expectedFps) : m_expectedFps(expectedFps), m_currentWaitMSec(0) {}
    int getWaitMSec()
    {
        int currentFps = get();
        int distance = currentFps - m_expectedFps;
        double adjust = distance * 0.01;
        //std::cout << "adjust " << adjust << std::endl;
        m_currentWaitMSec += adjust;
        if (m_currentWaitMSec < 0) m_currentWaitMSec = 0;
        return m_currentWaitMSec;
    }

private:
    int m_expectedFps;
    double m_currentWaitMSec;
};

#endif
