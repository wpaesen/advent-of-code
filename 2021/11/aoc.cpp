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

class DumboPus
{
public:
    DumboPus() : energy(0)
    {}

    std::vector<DumboPus*> neighbours;

    int energy;

    void increase(DumboPus* source = nullptr)
    {       
        energy++; 
        if (energy == 10)
        {
            for (auto& n : neighbours)
            {
                if (n != source)
                {
                    n->increase(this);
                }
            }
        }
    }

    bool flashed()
    {
        if (energy > 9)
        {
            energy = 0;
            return true;
        }
        return false;
    }
};

class Cavern
{
public:
    Cavern()
    {
        static constexpr std::array<std::pair<int, int>, 8> neighbours{{
             { -1, -1 }, {0, -1}, {1, -1},
             { -1, 0  },          {1, 0 },
             { -1, 1  }, {0,  1}, {1, 1 }
        }};

        /* Setup pointers to neighbours */
        for (int y = 0; y<grid.size(); ++y)
        {            
            for (int x = 0; x<grid[0].size(); ++x)
            {
                auto& me = grid[y][x];

                for (auto& d: neighbours)
                {
                    try
                    {
                        me.neighbours.emplace_back( & grid.at(y+d.second).at(x+d.first) );
                    }
                    catch( std::out_of_range const& e)
                    {}
                }
            }
        }
    }

    std::array<std::array<DumboPus,10>,10> grid;
   
    int iterate()
    {
        int ret = 0;
        for (auto &y : grid)
        {
            for(auto &x : y)
            {
                x.increase();
            }
        }        

        for (auto &y : grid)
        {
            for(auto &x : y)
            {
                ret += x.flashed() ? 1 : 0;
            }
        }

        return ret;    
    }

    static constexpr const std::string_view yellow_on = "\033[1m\033[33m";
    static constexpr const std::string_view yellow_off = "\033[0m";

    void print()
    {
        for (auto &y : grid)
        {
            for(auto &x : y)
            {
                if (x.energy == 0)
                {
                    std::cout << yellow_on << 0 << yellow_off;
                }
                else
                {
                    std::cout << x.energy;
                }
            }
            std::cout << std::endl;
        }
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

    Cavern cavern;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        int y = 0;
        while (std::getline(infile, line))
        {
            if (! line.empty())
            {
                int x = 0;
                for (auto& c : line)
                {
                    int energy = c-'0';
                    if ( (energy < 0) || (energy > 9) )
                    {
                        throw std::invalid_argument("Out of range");
                    }
                    cavern.grid.at(y).at(x++).energy = energy;
                }

                y++;
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    int i =0;
 
    int flashes = 0;
    for (; i<100; ++i)
    {
        flashes += cavern.iterate();
    }
    std::cout << "step " << i << " : " << flashes << " flashes seen" << std::endl;

    for (; i<999; ++i)
    {
        int df = cavern.iterate();
        if (df == 100)
        {
            std::cout << "step " << i+1 << " : First sync flash observed" << std::endl;
            break;
        }
    }

    if ( i == 999 )
    {
        std::cout << "No sync flash before step 1000.";
    }

    return 0;
}
