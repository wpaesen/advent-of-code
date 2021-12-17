#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <vector>
#include <cstdint>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <iomanip>
#include <list>
#include <chrono>
#include <cmath>
#include <string_view>

class TargetArea
{
public:
    TargetArea(int64_t min_x, int64_t max_x, int64_t min_y, int64_t max_y)
        : x(std::make_pair(min_x, max_x))
        , y(std::make_pair(min_y, max_y))
    {
        if (x.second < x.first)
        {
            std::swap(x.second, x.first);
        }

        if (y.second > y.first)
        {
            std::swap(y.second, y.first);
        }
    }

    bool inTargetY(int64_t ay) const
    {
        if (ay > y.first) return false;
        if (ay < y.second) return false;

        return true;
    }

    bool inTargetX(int64_t ax) const
    {
        if (ax < x.first) return false;
        if (ax > x.second) return false;

        return true;
    }

    bool passedTarget( int64_t ax, int64_t ay ) const
    {
        if (ax > x.second) return true;
        if (ay < y.second) return true;

        return false;
    }

    bool canHitTarget( int64_t vx, int64_t vy) const
    {
        int64_t cx = 0;
        int64_t cy = 0;

        while (! passedTarget(cx, cy))
        {
            cx += vx;
            cy += vy;

            if (inTargetX(cx) && inTargetY(cy)) return true;

            if (vx > 0) vx -= 1;
            vy -= 1;
        }

        return false;
    }


    std::pair<int64_t, int64_t> x;
    std::pair<int64_t, int64_t> y;
};

/*
  y = (a) +(a-1)+(a-2)+(a-3)+(a-4)
  y = 5a -1-2-3-4
*/


/* vx=4              vx=5
 * x = 4, vx = 3     x = 5 , vx = 4 
 * x = 7, vx = 2     x = 9 , vx = 3
 * x = 9, vx = 1     x = 12, vx = 2
 * x = 10, vx = 0    x = 14, vx = 1
 * x = 10, vx = 0    x = 15, vx = 0 
 */

class Probe
{
public:

    Probe(const TargetArea& a_target)
        : target(a_target)
    {
        /* Take any initial positive vy.  After (1+2*vy) steps the 
         * probe will be at height 0 with speed -vy-1.  The highest
         * possible velocity is the one where the next step it will 
         * land at the lowest y target range. 
         */
        max_vy = -a_target.y.second-1;

        /* Take any initial positive vx.  As vx causes drag, there needs
         * to be a minimal speed, otherwise the target area is never 
         * reached. 
         */
        min_vx = 1;
        int64_t x = min_vx;
        while (x < a_target.x.first)
        {
            min_vx++;
            x += min_vx;
        }

        /* Max x velocity is the velcity that would land us at the far right edge
         * of the target area after the first step
         */
        max_vx = a_target.x.second;

        /* Min y velocity is the velicoty that would land us at the bottom edge
         * of the target area after the first step.
         */
        min_vy = a_target.y.second;

        std::cout << "Highest y velocity : " << max_vy << std::endl;
        std::cout << "Lowest  y velocity : " << min_vy << std::endl;

        std::cout << "Highest x velocity : " << max_vx << std::endl;
        std::cout << "Lowest  x velocity : " << min_vx << std::endl;

        std::cout << std::endl;


         /* Find highest y position */
        int64_t max_y = 0;
        for (int64_t vy = max_vy; vy>0; vy--)
        {
            max_y += vy;
        }

        std::cout << "A : Highest possible position   " << max_y << std::endl;


        possible_speeds = 0;
        for (int64_t vy = min_vy; vy <= max_vy; ++vy)
        {
            for (int64_t vx = min_vx; vx <= max_vx; ++vx)
            {
                if ( target.canHitTarget(vx, vy)) 
                {
                    possible_speeds++;
                }
            }
        }

        std::cout << "B : Possible speed combinations " << possible_speeds << std::endl;
    }

    int64_t max_vy;
    int64_t min_vy;

    int64_t min_vx;
    int64_t max_vx;

    int64_t possible_speeds;

    TargetArea target;

    int64_t getXPos(int64_t vx, int steps)
    {
        int64_t x = 0;
        while (steps > 0)
        {
            x += vx;
            steps -= 1;
            if (vx > 0) vx -= 1;
        }
        return x;
    }
};

int
main(int argc, char **argv)
{
    std::cout << "Test probe :" << std::endl;
    Probe testprobe(TargetArea(20, 30, -10, -5));

    std::cout << std::endl << std::endl;
    std::cout << std::endl << "Normal probe :" << std::endl;
    Probe qprobe(TargetArea(81,129,-150,-108));

    return 0;
}
