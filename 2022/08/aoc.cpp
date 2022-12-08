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

class Grid
{
public:
    enum Direction : std::size_t {
        NORTH = 0,
        EAST  = 1,
        SOUTH = 2,
        WEST  = 3
    };


    class Tree
    {
    public:
        Tree(int a_height) : height(a_height), visible(false), scenic_score(0) {
            neighbours.fill(nullptr);
        };

        int height;
        bool visible;
        std::size_t scenic_score;

        std::array<Tree*, 4> neighbours;

        operator Tree*() { return this; }
    };

    Grid() : finalized(false) {};

    void addLine(const std::string& line )
    {
        if (finalized)
            throw std::invalid_argument("Can't modifiy grid after finalizing");

        if (! grid.empty())
        {
            if (line.length() != grid[0].size())
            {
                throw std::invalid_argument("Inconsistent grid size : '" + line +"'");
            }
        }

        grid.push_back({});

        for (auto& c : line)
        {
            int height = c - '0';
            grid.rbegin()->push_back(height);
        }
    }

    void print(std::ostream& os)
    {
        for (auto& l :grid)
        {
            for (auto &h: l)
            {
                std::cout << h.height;
            }
            std::cout << std::endl;
        }
    }

    void finalize()
    {
        if (! finalized)
        {
            finalized = true;

            /* Setup pointers */
            for (std::size_t y=0; y<grid.size(); ++y)
            {
                for (std::size_t x=0; x<grid.size(); ++x)
                {
                    Tree& me = grid[y][x];

                    if (x > 0)
                    {
                        me.neighbours[WEST] = grid[y][x-1];
                    }

                    if (x+1 < grid.size())
                    {
                        me.neighbours[EAST] = grid[y][x+1];
                    }

                    if (y > 0)
                    {
                        me.neighbours[NORTH] = grid[y-1][x];
                    }

                    if (y+1 < grid.size())
                    {
                        me.neighbours[SOUTH] = grid[y+1][x];
                    }
                }
            }

            /* Part A, scan for visibility */
            for (std::size_t i = 0; i<grid.size(); ++i)
            {
                scanVisibility(*grid[i].begin(), EAST );
                scanVisibility(*grid[i].rbegin(), WEST );
                scanVisibility((*grid.begin())[i], SOUTH );
                scanVisibility((*grid.rbegin())[i], NORTH );
            }

            /* Part B, scan for viewing distance */
            for (std::size_t y=0; y<grid.size(); ++y)
            {
                for (std::size_t x=0; x<grid.size(); ++x)
                {
                    scanViewingDistance(grid[y][x]);
                }
            }
        }
    }

    std::size_t countVisible() const
    {
        if (! finalized)
            throw std::invalid_argument("Can operate on grid before finalizing");

        std::size_t n_visible = 0;
    
        for (auto& r : grid)
        {
            for (auto& c : r)
            {
                if (c.visible)
                {
                    n_visible++;
                }
            }
        }

        return n_visible;
    }

    std::size_t highestScenicScore() const
    {
        if (! finalized)
            throw std::invalid_argument("Can operate on grid before finalizing");

        std::size_t score = 0;
    
        for (auto& r : grid)
        {
            for (auto& c : r)
            {
                if (c.scenic_score > score)
                {
                    score = c.scenic_score;
                }
            }
        }

        return score;
    }

private:

    bool finalized;

    void scanVisibility(Tree* i, Direction dir )
    {
        int tallest = -999;
        while (i)
        {
            if (i->height > tallest)
            {
                i->visible = true;
                tallest = i->height;
            }

            i = i->neighbours[dir];
        }
    }

    std::size_t findViewingDistance(Tree *i, Direction dir)
    {
        std::size_t distance = 0;

        Tree *next = i->neighbours[dir];
        while (next)
        {
            distance++;
            if (next->height >= i->height)
            {
                next = nullptr;
            }
            else
            {
                next = next->neighbours[dir];
            }
        }

        return distance;
    }

    void scanViewingDistance(Tree *i)
    {
        static const std::array<Direction, 4> dirs{NORTH, SOUTH, EAST, WEST};
        static const std::array<std::string, 4> dirNames{"NORTH", "SOUTH", "EAST ", "WEST " };

        i->scenic_score = 1;
        for (auto& dir : dirs)
        {
            if (i->scenic_score > 0)
            {
                i->scenic_score *= findViewingDistance(i, dir);
            }
        }
    }

    std::vector<std::vector<Tree>> grid;
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
            grid.addLine(line);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    // grid.print(std::cout);
    grid.finalize();

    std::cout << grid.countVisible() << " trees visible" << std::endl;
    std::cout << grid.highestScenicScore() << " highest scenice score" << std::endl;

    return 0;
}
