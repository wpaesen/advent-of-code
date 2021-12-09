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

class Grid
{
public:

    struct Coord
    {
        Coord() : x(0), y(0) {};
        Coord(int ax, int ay) : x(ax), y(ay) {};

        int x;
        int y;
    };

    Grid() : max_x(0)
    {
    }

    Coord getExtents()
    {
        return Coord( max_x, data.size() );
    }

    void pushRow(const std::string& row)
    {
        if (data.empty())
        {
            max_x = row.length();
        }
        else 
        {
            if (row.length() != max_x)
            {
                throw std::invalid_argument("Mismatching row length");
            }
        }

        data.emplace_back(row);
    }
    
    unsigned char operator[](const Coord& coord ) const
    {
        if ( coord.x < 0 ) return 9;
        if ( coord.y < 0 ) return 9;
        if ( coord.y >= data.size() ) return 9;
        if ( coord.x >= max_x) return 9;

        return data[coord.y][coord.x] - '0';
    }

    void clear(const Coord& coord)
    {
        if ( coord.x < 0 ) return;
        if ( coord.y < 0 ) return;
        if ( coord.y >= data.size() ) return;
        if ( coord.x >= max_x) return;

        data[coord.y][coord.x] = '9';
    }

    std::vector<Coord> getNeighours(const Coord& coord) const
    {
        std::vector<Coord> ret;

        if (coord.y > 0) ret.emplace_back( coord.x, coord.y-1 );
        if (coord.x > 0) ret.emplace_back( coord.x-1, coord.y );
        if (coord.y+1 < data.size()) ret.emplace_back( coord.x, coord.y+1 );
        if (coord.x+1 < max_x) ret.emplace_back( coord.x+1, coord.y );

        return ret;
    }

    std::vector<unsigned char> getNeighourValues(const Coord& coord)
    {
        std::vector<unsigned char> ret;

        for (auto &p : getNeighours(coord))
        {
            ret.emplace_back( (*this)[p] );
        }

        return ret;
    }


private:

    std::vector<std::string> data;
    int max_x;
};

class Explorer
{
public:
    Explorer() : risk(0), basinsurface(0) {};

    std::vector<Grid::Coord> minima;
    int risk;
    int basinsurface;

    void Run(const Grid& a_grid)
    {
        Grid grid(a_grid);

        /* Reset state */
        minima.clear();
        risk = 0;
        basinsurface = 1;

        /* Scan for the minima */
        {
            Grid::Coord iter;
            for (iter.y = 0; iter.y < grid.getExtents().y; ++iter.y)
            {
                for (iter.x = 0; iter.x < grid.getExtents().x; ++iter.x)
                {
                    auto neigbourValues = grid.getNeighourValues(iter);
                    auto myValue = grid[iter];

                    auto isLower = [myValue](unsigned char v) {
                        return myValue < v;
                    };

                    if (std::all_of( neigbourValues.cbegin(), neigbourValues.cend(), isLower ))
                    {
                        minima.emplace_back(iter);
                    }
                }
            }
        }

        /* Calculate the risk value (sum of all minimum+1) */
        for (auto &p : minima)
        {
            risk += grid[p]+1;
        }

        /* Calculate basin surfaces bleeding out from each minimum */
        std::vector<int> basinSurfaces;
        for (auto &p : minima)
        {
            basinSurfaces.emplace_back( getBasinSurface(grid, p) );
        }

        std::sort( basinSurfaces.begin(), basinSurfaces.end() );

        int i = 0;
        for ( auto k = basinSurfaces.rbegin(); (i < 3) && (k != basinSurfaces.rend()); ++k, ++i )
        {
            basinsurface *= *k;
        }
    }

private:

    int getBasinSurface(Grid& grid, const Grid::Coord &p)
    {
        if (grid[p] == 9)
            return 0;

        grid.clear(p);

        int ret = 1;
        for ( auto &p : grid.getNeighours(p) )
        {
            ret += getBasinSurface(grid, p);
        }

        return ret;
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

    Grid grid;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^\\s*([0-9]+)\\s*$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                grid.pushRow(match[1].str());
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    Explorer explorer;

    explorer.Run(grid);

    std::cout << "Risk level    " << explorer.risk << std::endl;
    std::cout << "Basin surface " << explorer.basinsurface << std::endl;
   
    return 0;
}
