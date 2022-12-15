#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <vector>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <bitset>
#include <regex>
#include <set>
#include <exception>
#include <deque>
#include <iomanip>
#include <thread>
#include <atomic>

constexpr bool do_debug{false};

class Sensor : public std::enable_shared_from_this<Sensor>
{
public:
    Sensor(int a_x, int a_y, int a_cb_x, int a_cb_y) : x(a_x), y(a_y), cb_x(a_cb_x - a_x), cb_y(a_cb_y - a_y) {
        distance = std::abs(cb_x) + std::abs(cb_y);
    }

    void print(std::ostream& os) const
    {
        os << "Sensor at x=" << x <<", y=" << y << ": closest beacon is at x=" << cb_x << ", y=" << cb_y << ", distance=" << distance;
    }

    std::pair<bool, std::pair<int, int>> getNoSensorSpots(int s_y) const
    {
        std::pair<bool, std::pair<int, int>> ret{false, {0,0}};

        int y_distance = std::abs(s_y - y);
        if (std::abs(y_distance) > distance)
        {
            /* Line is too far way from the sensor */
            return ret; 
        }

        ret.first = true;
        ret.second.first = x - (distance - y_distance);
        ret.second.second = x + (distance - y_distance);

        return ret;
    }

    int getBeaconX() const { return x + cb_x; }
    int getBeaconY() const { return y + cb_y; }

    bool beaconPossible(int b_x, int b_y) const
    {
        return (std::abs(x - b_x) + std::abs(y - b_y)) > distance;
    }

private:

    int x;
    int y;

    int cb_x;
    int cb_y;

    int distance;
};

std::ostream& operator<<(std::ostream& os, const Sensor& s)
{
    s.print(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Sensor>& s)
{
    if (s)
    {
        os << *s;
    }
    else
    {
        os << "Invalid sensor";
    }

    return os;
}

void mergelist(std::vector<std::pair<int, int>>& list)
{
    /* Try to merge segments, first all sort them on lowest x */
    std::sort(list.begin(), list.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return a.first < b.first;
    });

    auto left = list.begin();
    while (left != list.end())
    {
        auto right = std::next(left);
        if (right != list.end())
        {
            /* Check if these 2 segments overlap, if so adjust left 
             * so incorporates both
             */
            if (left->second >= right->first-1)
            {
                left->second = std::max(right->second, left->second);
                list.erase(right);
            }
            else
            {
                left = right;
            }
        }
        else
        {
            break;
        }
    }
}

int
main(int argc, char **argv)
{
    if (argc < 4)
    {
        std::cerr << "Usage : " << argv[0] << " datafilename testrow searchspace" << std::endl << std::endl;
        std::cerr << "For test  : " << argv[0] << " test.dat 10 20" << std::endl;
        std::cerr << "For input : " << argv[0] << " input.dat 2000000 4000000" << std::endl;

        exit(-1);
    }

    std::vector<std::shared_ptr<Sensor>> sensors;
    try
    {
        std::ifstream infile(argv[1]);

        std::regex matchrule{"^\\s*Sensor at x=([-]{0,1}[0-9]+)\\s*, y=([-]{0,1}[0-9]+)\\s*: closest beacon is at x=([-]{0,1}[0-9]+)\\s*, y=([-]{0,1}[0-9]+)\\s*$"};

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule))
            {
                sensors.emplace_back(std::make_shared<Sensor>(
                    std::stoi(match[1].str()),
                    std::stoi(match[2].str()),
                    std::stoi(match[3].str()),
                    std::stoi(match[4].str())
                ));
            }
            else
            {
                throw std::invalid_argument("Syntax error");
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    int testrow = std::stoi(argv[2]);
    std::cout << "Checking for sensor positions in row " << testrow << std::endl;

    std::vector<std::pair<int, int>> filled_segments;
    for (auto& s : sensors)
    {
        if (do_debug)
        {
            std::cout << s << std::endl;
        }
        auto ret = s->getNoSensorSpots(testrow);
        if (ret.first)
        {
            filled_segments.emplace_back(ret.second);
        }
    }

    mergelist(filled_segments);

    std::size_t length = 0;
    for (auto &s : filled_segments)
    {
        length += 1 + (s.second - s.first);
    }

    /* Check if any known beacons are present in our segment.  For every beacon
     * that is present, reduce the length with 1.  First make sure that we don't 
     * count the same beacon multiple times.
     */
    std::set<int> beacon_xs;
    for (auto& s : sensors)
    {
        if (s->getBeaconY() == testrow)
        {
            beacon_xs.insert(s->getBeaconX());
        }
    }

    for (auto& x : beacon_xs)
    {
        for (auto &s : filled_segments)
        {
            if ((s.first <= x) && (s.second >= x))
            {
                length--;
            }
        }
    }

    std::cout << "     -> " << length << " positions which can not contain a beacon" << std::endl;

    int min_x = 0;
    int max_x = std::stoi(argv[3]);

    std::cout << "Searching for beacon spots in area [" << min_x << "," << max_x << "]" << std::endl;

    for (int y = min_x; y <= max_x; ++y)
    {
        std::vector<std::pair<int, int>> blocked_segments;

        for (auto& s: sensors)
        {
            auto r = s->getNoSensorSpots(y);

            if (r.first)
            {
                blocked_segments.emplace_back(std::max(min_x, r.second.first), std::min(max_x, r.second.second));
            }
        }

        /* Try to merge segments, first all sort them on lowest x */
        std::sort(blocked_segments.begin(), blocked_segments.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first < b.first;
        });

        auto left = blocked_segments.begin();
        while (left != blocked_segments.end())
        {
            auto right = std::next(left);
            if (right != blocked_segments.end())
            {
                /* Check if these 2 segments overlap, if so adjust left 
                 * so incorporates both
                 */
                if (left->second >= right->first-1)
                {
                    left->second = std::max(right->second, left->second);
                    blocked_segments.erase(right);
                }
                else
                {
                    left = right;
                }
            }
            else
            {
                break;
            }
        }

        if (blocked_segments.empty())
        {
            std::cerr << "Empty line ?" << std::endl;
            std::exit(-1);
        }

        if (blocked_segments.size() > 1)
        {
            /* find the first x that is not part of a segment */
            int res_x = blocked_segments.begin()->second + 1;
            int64_t freq = (((int64_t)res_x)*4000000) + (int64_t)y;

            std::cout << "     -> " << "Sensor at (" << res_x << ", " << y << ") : freq " << freq << std::endl;
            break;
        }
    }

    return 0;
}
