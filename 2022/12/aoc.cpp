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

constexpr bool do_debug{false};

class Map {

public:
    enum Dir : std::size_t {
        NORTH = 0,
        EAST = 1,
        SOUTH = 2,
        WEST = 3,
    };

    struct Node {

        Node() : height(-1) { 
            reset(); 
        }
        Node(char a_height) : height(a_height - 'a') { 
            reset(); 
        };
        int height;
        std::size_t distance;
        bool visited;
    
        std::vector<Node*> neighbours;

        void reset()
        {
            distance = std::numeric_limits<std::size_t>::max();
            visited = false;
        }

        void calc()
        {
            std::size_t new_distance = distance + 1;
            for (auto &n : neighbours)
            {
                if (! n->visited)
                {
                    if (n->distance > new_distance)
                    {
                        n->distance = new_distance;
                    }
                }
            }
            visited = true;
        }
    };

    Map() : start(nullptr), end(nullptr) {};

    void addLine(const std::string& line)
    {
        grid.emplace_back();
        auto& row = grid.back();

        for (auto& c : line)
        {
            if (c == 'S')
            {
                row.emplace_back(std::make_unique<Node>('a'));
                start = row.back().get();
            }
            else if (c == 'E')
            {
                row.emplace_back(std::make_unique<Node>('z'));
                end = row.back().get();                
            }
            else
            {
                row.emplace_back(std::make_unique<Node>(c));
            }
        }
    }

    void finalize()
    {
        if (start == nullptr)
            throw std::invalid_argument("No start point in data");

        if (end == nullptr)
            throw std::invalid_argument("No end point in data");

        std::size_t row_size = 0;
        for (auto& r : grid)
        {
            if (row_size == 0)
            {
                row_size = r.size();
            }
            else if (row_size != r.size())
            {
                throw std::invalid_argument("Irregular grid sizing");
            }
        }

        for (std::size_t y=0; y<grid.size(); ++y)
        {
            for (std::size_t x=0; x<row_size; ++x)
            {
                auto& node = grid[y][x];

                auto isReachable = [](const std::unique_ptr<Node>& a, std::unique_ptr<Node>& b) {
                    int diff = b->height - a->height;
                    return (diff <= 1);
                };

                if (y > 0)
                {
                    if (isReachable(node, grid[y-1][x]))
                    {
                        node->neighbours.emplace_back(grid[y-1][x].get());
                    }
                }

                if (x+1 < row_size)
                {
                    if (isReachable(node,grid[y][x+1]))
                    {
                        node->neighbours.emplace_back(grid[y][x+1].get());
                    }
                }

                if (y+1 < grid.size())
                {
                    if (isReachable(node,grid[y+1][x]))
                    {
                        node->neighbours.emplace_back(grid[y+1][x].get());
                    }
                }

                if (x > 0)
                {
                    if (isReachable(node,grid[y][x-1]))
                    {
                        node->neighbours.emplace_back(grid[y][x-1].get());
                    }
                }
            }
        }

        end->neighbours.clear();
    }

    std::size_t findShortestPath()
    {
        return findShortestPath(start);
    }

    std::size_t findShortestHikingTrail()
    {
        /* Find all nodes with height 0 */
        std::vector<Node*> startnodes;

        for (auto& r : grid)
        {
            for (auto& c : r)
            {
                if (c->height == 0)
                {
                    startnodes.push_back(c.get());
                }
            }
        }

        std::size_t shortest_distance = std::numeric_limits<std::size_t>::max();

        for (auto& n : startnodes)
        {
            auto distance = findShortestPath(n);
            if (distance < shortest_distance)
            {
                shortest_distance = distance;
            }
        }

        return shortest_distance;
    }

private:

   std::size_t findShortestPath(Node *a_start)
    {
        std::vector<Node*> unvisited;

        for (auto& r : grid)
        {
            for (auto& c : r)
            {
                c->reset();
                if (c.get() != a_start)
                {
                    unvisited.push_back(c.get());
                }
            }
        }

        a_start->distance = 0;
        a_start->calc();
        while ((! unvisited.empty()) && (! end->visited))
        {
            auto node = std::min_element(unvisited.begin(), unvisited.end(), [](const Node* a, const Node *b) {
                return a->distance < b->distance;
            });

            (*node)->calc();
            unvisited.erase(node);
        }

        return end->distance;
    }

    std::vector<std::vector<std::unique_ptr<Node>>> grid;
    Node *start;
    Node *end;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    Map map;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            map.addLine(line);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    map.finalize();

    std::cout << "Shortest path to top " << map.findShortestPath() << std::endl;
    std::cout << "Shortest hiking trail " << map.findShortestHikingTrail() << std::endl;

    return 0;
}
