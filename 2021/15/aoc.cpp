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
#include <set>

class Node : public std::enable_shared_from_this<Node>
{
public:
    Node(int a_risk = std::numeric_limits<int>::max()) 
        : risk(a_risk)
        , lowest(std::numeric_limits<int>::max())
        , visited(false)
        , x(0)
        , y(0)
    {
        if (risk != std::numeric_limits<int>::max() )
        {
            if (risk > 9)
            {
                risk = 1 + (risk -1)%9;
            }
        }
    }

    int risk;
    int lowest;
    bool visited;
    int x;
    int y;

    int operator== (const Node& other) const
    {
        return (other.x == x) && (other.y == y);
    }

    std::pair<int, int> co() const 
    {
        return std::make_pair(x, y);
    }
};

class Grid
{
public:
    Grid() {};
    Grid(const Grid& other) : map(other.map) {};

    void addRow(const std::string& line)
    {
        map.push_back({});
        auto &r = *map.rbegin();

        for (auto c : line)
        {
            auto node = std::make_shared<Node>(c - '0');
            node->y = map.size()-1;
            node->x = r.size();

            r.emplace_back(node);
        }

        destinationPoint.first = std::max(destinationPoint.first, (int)(r.size()-1));
        destinationPoint.second = std::max(destinationPoint.second, (int)(map.size()-1));
    }

    std::shared_ptr<Node> getStartPoint()
    {
        return get(std::make_pair(0, 0));
    }

    std::shared_ptr<Node> getDestinationPoint() 
    {
        return get(destinationPoint);
    }

    bool exists(const std::pair<int, int>& p) const
    {
        return exists(p.first, p.second);
    }

    bool exists(int x, int y) const
    {
        if ((y < 0) || (y >= map.size())) return false;
        if ((x < 0) || (x >= map[y].size())) return false;

        return true;
    }

    bool exists(std::shared_ptr<Node> n) const
    {
        return exists(n->co());
    }

    std::shared_ptr<Node> operator[] (const std::pair<int, int>& p)
    {
        return operator() (p.first, p.second);
    }

    std::shared_ptr<Node> operator() (int x, int y)
    {
        if (! exists(x, y)) return offgridNode();      
        return map[y][x];
    }

    std::shared_ptr<Node> get(const std::pair<int, int>& p)
    {
        return operator() (p.first, p.second);
    }

    std::shared_ptr<Node> get(std::shared_ptr<Node>& n, const std::pair<int, int>& delta )
    {
        return operator() (n->x + delta.first, n->y + delta.second);
    }

    void scan()
    {
        constexpr std::array<std::pair<int, int>,4> neighours{{ {0, 1}, {1, 0}, {0, -1}, {-1, 0} }};

        std::list<std::shared_ptr<Node>> unvisited;

        getStartPoint()->lowest = 0;

        unvisited.push_back(getStartPoint());

        while ( ! unvisited.empty() ) 
        {
            auto l = std::min_element(unvisited.begin(), unvisited.end(), [](std::shared_ptr<Node>& a, std::shared_ptr<Node>& b) 
            {
                return a->lowest < b->lowest;
            });

            std::shared_ptr<Node> c = *l;

            unvisited.erase(l);

            for (auto &n : neighours)
            {
                auto p = get(c, n);

                if (p->visited) continue;

                auto val = c->lowest + p->risk;
                if (p->lowest == std::numeric_limits<int>::max())
                {
                    unvisited.push_back(p);
                }
                p->lowest = std::min(p->lowest, val);
            }

            c->visited = true;
        }
    }

    std::vector<std::vector<std::shared_ptr<Node>>> map;

    Grid enlarge() const
    {
        Grid g2;

        /* first multiply in the X direction */
        for (auto& r : map)
        {
            g2.map.push_back({});
            auto nr = g2.map.rbegin();

            for (int scan = 0; scan < 5; ++scan)
            {
                for (auto &c : r)
                {
                    auto node = std::make_shared<Node>( c->risk + scan );
                    node->y = c->y;
                    node->x = nr->size();

                    nr->push_back(node);
                }
            }
        }

        /* Now multiply in the T direction */
        auto rows = g2.map;

        for (int scan = 1; scan < 5; ++scan)
        {
            for (auto& r : rows)
            { 
                g2.map.push_back({});
                auto nr = g2.map.rbegin();

                for (auto &c : r)
                {
                    auto node = std::make_shared<Node>( c->risk + scan );
                    node->x = c->x;
                    node->y = g2.map.size()-1;
                    nr->push_back(node);
                }
            }
        }

        g2.destinationPoint = std::make_pair(
            g2.map.rbegin()->size()-1,
            g2.map.size()-1
        );

        return g2;
    }

private:

    std::shared_ptr<Node> offgridNode() 
    {
        auto ret = std::make_shared<Node>();

        ret->risk = std::numeric_limits<int>::max();
        ret->lowest = std::numeric_limits<int>::max();
        ret->visited = true;

        return ret;
    }

    std::pair<int, int> destinationPoint{0, 0};
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    Grid grid;

    try
    {
        std::ifstream infile(argv[1]);


        std::string line;
        while (std::getline(infile, line))
        {
            if (! line.empty())
            {
                grid.addRow(line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    grid.scan();

    std::cout << "Lowest risk path A " << grid.getDestinationPoint()->lowest << std::endl;

    auto grid2 = grid.enlarge();
    grid2.scan();

    std::cout << "Lowest risk path B " << grid2.getDestinationPoint()->lowest << std::endl;

    return 0;
}
