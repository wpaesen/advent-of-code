#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <vector>
#include <cstdint>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <iomanip>
#include <list>
#include <chrono>
#include <cmath>

class Cave
{
public:

    Cave(const std::string& a_name)
        : name( a_name )
        , m_visits(0)
    {
        m_endcave = (name == "end");
        m_begincave = (name == "start");
        m_bigcave = std::any_of( name.begin(), name.end(), []( char c ) { return std::isupper(c); });
    }

    void Connect(std::unique_ptr<Cave>& other)
    {
        Connect(*other);
    }

    void Connect(Cave& other)
    {
        Cave* po = other.p();

        for (auto& pc : con)
        {
            if (pc == po) return;
        }

        con.emplace_back(po);
    }

    bool isEnd() const
    {
        return m_endcave;
    }

    int64_t traverse(std::vector<Cave*> path = std::vector<Cave*>())
    {
        int64_t ret = 0;

        if ( ! m_bigcave )
        {
            for (auto& c: path)
            {
                if (c == this) return ret;
            }
        }

        path.emplace_back(this);
        
        if ( isEnd() )
        {
            ret = 1;
        }
        else
        {
            for (auto& c : con)
            {
                ret += c->traverse(path);
            }
        }

        return ret;
    }

    int64_t traverse2(std::vector<Cave*> path = std::vector<Cave*>(), bool looped = false)
    {
        int64_t ret = 0;

        if ( ( ! path.empty() ) && m_begincave )
        {
            return 0;
        }

        if ( ! m_bigcave )
        {
            bool visited = false;
            for (auto& c: path)
            {
                if (c == this) 
                {
                    if (looped) return 0;
                    visited += 1;
                    looped = true;
                    if (visited > 1) return 0;
                }
            }
        }

        path.emplace_back(this);
        if ( isEnd() )
        {
            ret = 1;
        }
        else
        {
            for (auto& c : con)
            {
                ret += c->traverse2(path, looped);
            }
        }

        return ret;
    }
private:

    std::string name;
    std::vector<Cave*> con;
    
    bool m_bigcave;
    bool m_endcave;
    bool m_begincave;

    int  m_visits;

    Cave* p() { return this; };
};

class Graph
{
public:
    Graph() {};

    std::map<std::string,std::unique_ptr<Cave>> caves;

    void addLink(const std::string& a, const std::string& b)
    {
        auto pa = caves.find(a);
        if (pa == caves.end())
        {
            auto ret = caves.emplace(a, std::make_unique<Cave>(a));
            pa = ret.first;
        }

        auto pb = caves.find(b);
        if (pb == caves.end())
        {
            auto ret = caves.emplace(b, std::make_unique<Cave>(b));
            pb = ret.first;
        }

        pa->second->Connect(pb->second);
        pb->second->Connect(pa->second);
    }

    int64_t countTraverses()
    {
        return caves["start"]->traverse();
    }

    int64_t countTraverses2()
    {
        return caves["start"]->traverse2();
    }
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    Graph graph;
    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^\\s*(.+)-(.+)\\s*$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                graph.addLink( match[1].str(), match[2].str() );
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    std::cout << "part 1 : " << graph.countTraverses() << " possible traverses" << std::endl;
    std::cout << "part 2 : " << graph.countTraverses2() << " possible traverses" << std::endl;

    return 0;
}
