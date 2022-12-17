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
#include <list>

constexpr bool do_debug{false};

class Valve
{
public:

    Valve() : name(""), rate(0), opened(false) {}

    Valve(const std::string& a_name, int a_rate)
        : name(a_name), rate(a_rate), opened(false)
    {}

    std::string name;
    int rate;

    void setNextNames(const std::string& names)
    {
        std::string accu;

        for (auto& c : names)
        {
            if ((c >='A') && (c<='Z'))
            {
                accu.push_back(c);
            }
            else if (! accu.empty())
            {
                next_names.push_back(accu);
                accu.clear();
            }
        }

        if (! accu.empty())
        {
            next_names.push_back(accu);            
        }
    };

   
    std::vector<std::string> next_names;
    std::list<std::pair<int, Valve*>> next;

    bool opened;

    void print(std::ostream& os) const
    {
        os << "Valve " << name << " has flow rate=" << rate << "; ";
        if (next_names.size() == 1)
        {
            os << "tunnel leads to valve " << next_names[0];
        }
        else
        {
            os << "tunnels lead to valves ";
            std::string sep = "";

            for (auto& n : next_names)
            {
                os << sep << n;
                if (sep.empty())
                {
                    sep = ", ";
                }
            }
        }
    }

    void print2(std::ostream& os) const
    {
        os << "Valve " << name << " has flow rate=" << rate << "; next=";

        for (auto &n : next)
        {
            os << " (" << n.first << ":" << n.second->name << ":" << n.second->rate << ")"; 
        }
    }

    operator Valve*() { return this; }
    operator const Valve*() const { return this; }

    class Opener
    {
    public:
        Opener(Valve *v) : valve(v)
        {
            if (valve->opened)
            {
                opened = false;
            }
            else
            {
                opened = true;
                valve->opened = true;
            }
        }

        ~Opener()
        {
            if (opened)
                valve->opened = false;
        }

        operator bool() const 
        {
            return opened;
        }

    private:
        bool opened;
        Valve* valve;
    };
};

class Grid
{
public:
    Grid() {};

    struct Distance
    {
        Distance() : value(std::numeric_limits<int>::max()) {}
        Distance(int a_value) : value(a_value) {};

        int value;

        Distance operator+(const Distance& rhs) const
        {
            if (value == std::numeric_limits<int>::max()) return Distance();;
            if (rhs.value == std::numeric_limits<int>::max()) return Distance();
            return Distance(value + rhs.value);
        }

        void updateIfLower(const Distance& rhs)
        {
            if (rhs.value < value)
                value = rhs.value;
        }
    };

    void addValve(const std::string& name, const std::string& rate, const std::string& tunnels)
    {
        auto ret = valves.emplace(name, Valve(name, std::stoi(rate)));
        ret.first->second.setNextNames(tunnels);
    }

    
    std::map<std::string, Valve> valves;

    void finalize()
    {
        std::map<std::string, std::map<std::string, Distance>> m_Distances;

        /* Init the matrix */
        std::map<Valve*, int> distancesInit;
        for (auto& v1 : valves)
        {
            auto r = m_Distances.emplace(v1.first, std::map<std::string, Distance>());
            for (auto& v1 : valves)
            {
                r.first->second.emplace(v1.first, Distance());
            }
        }

        for (auto& v1 : valves)
        {
            auto r = m_Distances.find(v1.first);
            if (r == m_Distances.end()) 
                break;

            for (auto& v2n : v1.second.next_names)
            {
                auto c = r->second.find(v2n);
                if (c != r->second.end()) 
                {
                    c->second.value = 1;
                }
            }

        }

        for (auto& v1 : valves)
        {
            for (auto& v2 : valves)
            {
                for (auto& v3 : valves)
                {
                    m_Distances[v2.first][v3.first].updateIfLower(m_Distances[v2.first][v1.first] + m_Distances[v1.first][v3.first]);
                }
            }
        }

        for (auto& v1 : m_Distances)
        {
            auto& valve1 = valves[v1.first];
            for (auto& v2 : v1.second)
            {
                auto& valve2 = valves[v2.first];
                if ((v2.second.value < Distance().value) && (valve2.rate > 0))
                {
                    valve1.next.push_back({v2.second.value, valve2});
                }
            }
        }
    }

    std::size_t findHighestYield()
    {
        auto start = valves.find("AA");
        if (start == valves.end())
            throw std::runtime_error("Valve AA not present");

        return findHighestYield(30, start->second);
    }

    std::size_t findHighestYield2()
    {
        auto start = valves.find("AA");
        if (start == valves.end())
            throw std::runtime_error("Valve AA not present");

        std::array<std::pair<int, Valve*>, 2> situation;
        for (auto& s : situation)
        {
            s.first = 26;
            s.second = start->second;
        }
    
        return findHighestYield2(situation);
    }

    std::size_t findHighestYield(int time_left, Valve* current)
    {
        if (time_left <= 0)
            return 0;

        std::size_t ret = 0;
        for (auto& v : current->next)
        {
            Valve::Opener canOpen(v.second);

            if (canOpen)
            {
                int new_time_left = time_left - v.first - 1;
                if (new_time_left <= 0)
                    continue;

                auto newret = (v.second->rate * new_time_left) + findHighestYield(new_time_left, v.second);
                if (newret > ret)
                    ret = newret;
            }
        }

        return ret;
    }

    std::size_t findHighestYield2(const std::array<std::pair<int, Valve*>, 2>& iter)
    {
        for (auto& i : iter)
        {
            if (i.first <= 0)
                return 0;
        }

        std::array<std::pair<int, Valve*>, 2> step;

        std::size_t ret = 0;
        for (auto& v1 : iter[0].second->next)
        {
            Valve::Opener canOpen1(v1.second);

            if (canOpen1)
            {
                step[0].first = iter[0].first - v1.first - 1;
                if (step[0].first <= 0)
                    continue;
                step[0].second = v1.second;

                for (auto& v2 : iter[1].second->next)
                {
                    Valve::Opener canOpen2(v2.second);

                    if (canOpen2)
                    {
                        step[1].first = iter[1].first - v2.first - 1;
                        if (step[1].first <= 0)
                            continue;

                        step[1].second = v2.second;

                        auto newret = (v1.second->rate * step[0].first) + (v2.second->rate * step[1].first) ;
                        newret += findHighestYield2(step);

                        if (newret > ret)
                        {
                            ret = newret;
                        }
                    }
                }
            }
        }

        /* Also check if any of the parties goes alone on it's own */
        for (auto& i : iter)
        {
            for (auto& v1 : i.second->next)
            {
                Valve::Opener canOpen1(v1.second);

                if (canOpen1)
                {
                    int new_time_left = i.first - v1.first - 1;
                    if (new_time_left <= 0)
                        continue;

                    auto newret = (v1.second->rate * new_time_left) + findHighestYield(new_time_left, v1.second);
                    if (newret > ret)
                    {
                        ret = newret;
                    }
                }
            }
        }

        return ret;
    }

    void print(std::ostream& os) const
    {
        for (auto &v : valves)
        {
            v.second.print2(os);
            os << std::endl;
        }
    }
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    Grid grid;
    try
    {
        std::ifstream infile(argv[1]);

        std::regex matchrule{"^\\s*Valve ([A-Z]{2}) has flow rate=([0-9]+); tunnels{0,1} leads{0,1} to valves{0,1} (.*)\\s*$"};

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule))
            {
                grid.addValve(match[1].str(), match[2].str(), match[3].str());
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

    grid.finalize();

    std::cout << "Best pressure release : " << grid.findHighestYield() << std::endl;
    std::cout << "Elephant helped Best pressure release : " << grid.findHighestYield2() << std::endl;

    return 0;
}
