#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
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
#include <list>
#include <future>
#include <functional>
#include <optional>
#include <string_view>

static constexpr bool with_debug = false;

using namespace std::string_view_literals;

struct coord_t {
    using value_type = int64_t;

    value_type x;
    value_type y;

    coord_t() : x(0), y(0){};
    coord_t(value_type a_x, value_type a_y) : x(a_x), y(a_y){};

    bool operator<(const coord_t& rhs) const
    {
        if (y < rhs.y)
            return true;
        if (y > rhs.y)
            return false;
        return x < rhs.x;
    }

    bool operator==(const coord_t& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }

    std::ostream& write(std::ostream& os) const
    {
        os << "(" << x << ", " << y << ")";
        return os;
    };

    coord_t operator-(const coord_t& rhs) const
    {
        return {x - rhs.x, y - rhs.y};
    }

    coord_t abs() const
    {
        coord_t ret{x, y};
        if (x < 0) {
            ret.x = -x;
        }
        if (y < 0) {
            ret.y = -y;
        }

        return ret;
    }

    coord_t max(const coord_t& rhs) const
    {
        return {std::max(x, rhs.x), std::max(y, rhs.y)};
    }

    int64_t length() const
    {
        return x + y;
    }
};

class Galaxy
{
public:
    Galaxy() : m_Id(-1), m_Pos(-1, -1){};
    Galaxy(int id, coord_t::value_type x, coord_t::value_type y) : m_Id(id), m_Pos(x, y){};
    Galaxy(int id, const coord_t& pos) : m_Id(id), m_Pos(pos){};

    const coord_t& getPos() const
    {
        return m_Pos;
    }

    coord_t getExpandedPos(const std::map<coord_t::value_type, coord_t::value_type>& xfrm_x,
                           const std::map<coord_t::value_type, coord_t::value_type>& xfrm_y) const
    {
        coord_t ret(m_Pos);

        auto x_elm = xfrm_x.find(m_Pos.x);
        if (x_elm != xfrm_x.end()) {
            ret.x = x_elm->second;
        }

        auto y_elm = xfrm_y.find(m_Pos.y);
        if (y_elm != xfrm_y.end()) {
            ret.y = y_elm->second;
        }

        return ret;
    }

    int getId() const
    {
        return m_Id;
    }

    std::ostream& write(std::ostream& os) const
    {
        return os;
    };

    coord_t getDelta(const Galaxy& rhs) const
    {
        auto ret = (m_Pos - rhs.m_Pos).abs();

        if (ret.y > ret.x) {
            std::swap(ret.x, ret.y);
        }

        return ret;
    }

    int64_t getDistance(const Galaxy& rhs) const
    {
        return getDelta(rhs).length();
    }

private:
    coord_t m_Pos;
    int m_Id;
};

class Grid
{
public:
    Grid() : m_Y(0), m_X(0), m_Id(1){};

    void addLine(const std::string& line)
    {
        for (std::size_t x = 0; x < line.length(); ++x) {
            if (line[x] == '#') {
                coord_t p(x, m_Y);
                m_Galaxies.emplace(p, Galaxy(m_Id++, p));
                m_used_Y.emplace(m_Y);
                m_used_X.emplace(x);
            }
        }
        m_X = std::max(m_X, (coord_t::value_type)line.length());
        m_Y++;
    }

    std::ostream& write(std::ostream& os) const
    {
        for (int y = 0; y < m_Y; ++y) {
            for (int x = 0; x < m_X; ++x) {
                coord_t p(x, y);
                auto elm = m_Galaxies.find(p);
                if (elm != m_Galaxies.end()) {
                    os << "#";
                } else {
                    os << ".";
                }
            }
            os << std::endl;
        }
        return os;
    };

    Grid expand(const coord_t::value_type& a_expansion_value)
    {
        Grid ret;

        /* Expand columns */
        std::map<coord_t::value_type, coord_t::value_type> xfrm_x;

        int x_offset = 0;
        for (coord_t::value_type x = 0; x < m_X; ++x, ++x_offset) {
            if (m_used_X.count(x) == 0) {
                x_offset += a_expansion_value - 1;
            } else {
                xfrm_x[x] = x_offset;
            }
        }
        /* Expand rows */
        std::map<coord_t::value_type, coord_t::value_type> xfrm_y;

        int y_offset = 0;
        for (coord_t::value_type y = 0; y < m_Y; ++y, ++y_offset) {
            if (m_used_Y.count(y) == 0) {
                y_offset += a_expansion_value - 1;
            } else {
                xfrm_y[y] = y_offset;
            }
        }

        std::map<coord_t, Galaxy> new_Galaxies;
        for (auto& g : m_Galaxies) {
            auto newpos = g.second.getExpandedPos(xfrm_x, xfrm_y);
            ret.m_Galaxies.emplace(newpos, Galaxy(g.second.getId(), newpos));
        }

        ret.m_Y = 0;
        ret.m_X = 0;
        ret.m_Id = m_Id;
        for (auto& g : m_Galaxies) {
            ret.m_Y = std::max(m_Y, g.first.y + 1);
            ret.m_X = std::max(m_X, g.first.x + 1);
            ret.m_used_Y.emplace(g.first.y);
            ret.m_used_X.emplace(g.first.x);
        }

        return ret;
    }

    std::size_t getDistanceSum() const
    {
        std::size_t ret = 0;

        for (auto i = m_Galaxies.begin(); i != m_Galaxies.end(); i = std::next(i)) {
            for (auto j = std::next(i); j != m_Galaxies.end(); j = std::next(j)) {
                ret += i->second.getDistance(j->second);
            }
        }

        return ret;
    }

private:
    std::map<coord_t, Galaxy> m_Galaxies;
    coord_t::value_type m_Y;
    coord_t::value_type m_X;

    std::set<coord_t::value_type> m_used_Y;
    std::set<coord_t::value_type> m_used_X;

    int m_Id; /* Counter for assigning galaxy id's */
};

namespace std
{
    string to_string(const Grid& node)
    {
        std::ostringstream buf;
        node.write(buf);
        return buf.str();
    }

    ostream& operator<<(ostream& os, const Grid& node)
    {
        return node.write(os);
    }

    string to_string(const coord_t& node)
    {
        std::ostringstream buf;
        node.write(buf);
        return buf.str();
    }

    ostream& operator<<(ostream& os, const coord_t& node)
    {
        return node.write(os);
    }

} // namespace std

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    Grid grid;

    try {
        std::ifstream infile(argv[1]);

        std::string line;

        while (std::getline(infile, line)) {
            if (!line.empty()) {
                grid.addLine(line);
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    if (with_debug) {
        std::cout << "Grid :" << std::endl;
        std::cout << grid << std::endl;
    }

    auto grid_a = grid.expand(2);
    auto grid_b = grid.expand(1000000);

    std::cout << "A distance of shortest lengths " << grid_a.getDistanceSum() << std::endl;
    std::cout << "B distance of shortest lengths " << grid_b.getDistanceSum() << std::endl;

    return 0;
}
