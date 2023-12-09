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

class Sequence
{
public:
    Sequence() : m_Empty(true)
    {
    }
    Sequence(const std::string& strdesc) : m_Empty(true)
    {
        std::istringstream buf(strdesc);
        for (std::string line; std::getline(buf, line, ' ');) {
            m_Points.emplace_back(std::stoi(line));
        }

        if (m_Points.size() > 1) {
            for (auto i = m_Points.begin(); i != std::prev(m_Points.end()); i = std::next(i)) {
                auto j = std::next(i);

                m_Empty &= ((*j - *i) == 0);
            }
        }
    }

    std::ostream& write(std::ostream& os) const
    {
        bool first = true;
        for (auto& p : m_Points) {
            if (!first) {
                first = false;
            } else {
                os << " ";
            };
            os << p;
        }

        return os;
    }

    Sequence derive() const
    {
        Sequence ret = *this;

        ret.m_Empty = derive(ret.m_Points);

        return ret;
    }

    int64_t extraPolateA() const
    {
        if (m_Points.empty())
            return 0;

        std::vector<int64_t> points(m_Points);
        int64_t ret = points.back();

        while (!derive(points)) {
            ret += points.back();
        }

        return ret;
    }

    int64_t extraPolateB() const
    {
        if (m_Points.empty())
            return 0;

        std::vector<int64_t> points(m_Points);

        std::vector<std::pair<int64_t, int64_t>> history{{0, points.front()}};
        while (!derive(points)) {
            history.emplace_back(0, points.front());
        }
        history.emplace_back(0, 0);

        for (auto i = std::next(history.rbegin()); i != history.rend(); i = std::next(i)) {
            auto j = std::prev(i);

            i->first = i->second - j->first;
        }

        return history[0].first;
    }

    bool empty() const
    {
        return m_Empty;
    }

private:
    bool m_Empty;
    std::vector<int64_t> m_Points;

    bool derive(std::vector<int64_t>& points) const
    {
        if (points.size() > 1) {
            for (auto i = points.begin(); i != std::prev(points.end()); i = std::next(i)) {
                auto j = std::next(i);
                *i = *j - *i;
            }
            points.pop_back();
        }

        return !std::any_of(points.begin(), points.end(), [](const int64_t& v) { return v != 0; });
    }
};

namespace std
{
    string to_string(const Sequence& node)
    {
        std::ostringstream buf;
        node.write(buf);
        return buf.str();
    }

    ostream& operator<<(ostream& os, const Sequence& node)
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

    std::vector<Sequence> sequences;

    try {
        std::ifstream infile(argv[1]);

        std::string line;

        while (std::getline(infile, line)) {
            if (!line.empty()) {
                sequences.emplace_back(line);
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    if (with_debug) {
        for (auto& n : sequences) {
            std::cout << "Sequence : " << std::endl;
            std::cout << n << std::endl;

            auto derived = n;
            while (!derived.empty()) {
                derived = derived.derive();
                std::cout << " - " << derived << std::endl;
            }

            std::cout << "A Extrapolated value " << n.extraPolateA() << std::endl;
        }
    }

    int64_t extrapolate_sum = 0;
    for (auto& n : sequences) {
        extrapolate_sum += n.extraPolateA();
    };

    std::cout << "A extrapolated sum " << extrapolate_sum << std::endl;

    extrapolate_sum = 0;
    for (auto& n : sequences) {
        extrapolate_sum += n.extraPolateB();
    };

    std::cout << "B extrapolated sum " << extrapolate_sum << std::endl;

    return 0;
}
