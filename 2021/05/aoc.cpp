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

class Line
{
public:

    Line(int x1, int y1, int x2, int y2)
        : coords( { { {x1, y1}, {x2, y2} } } )
    {};

    Line()
        : coords( { { {0, 0}, {0, 0} } } )
    {};

    std::array<std::pair<int, int>,2> coords;

    bool isHorizontal() const
    {
        return ( coords[0].second == coords[1].second );
    }

    bool isVertical() const
    {
        return ( coords[0].first == coords[1].first );
    }

    bool isDiagonal() const
    {
        return abs( coords[0].first - coords[1].first ) == abs( coords[0].second - coords[1].second );
    }

    void print() const
    {
        std::cout << coords[0].first << "," << coords[0].second << " -> " << coords[1].first << "," << coords[1].second << std::endl;
    }

    std::vector<std::pair<int,int>> getPoints() const
    {
        std::vector<std::pair<int,int>> ret;

        if ( isVertical() )
        {
            /* vertical line */
            int y_min = std::min( coords[0].second, coords[1].second );
            int y_max = std::max( coords[0].second, coords[1].second );
            for ( int y = y_min; y <= y_max; ++y )
            {
                ret.emplace_back( coords[0].first, y );
            }
        }
        else if ( isHorizontal() )
        {
            /* horizontal line */
            int x_min = std::min( coords[0].first, coords[1].first );
            int x_max = std::max( coords[0].first, coords[1].first );
            for ( int x = x_min; x <= x_max; ++x )
            {
                ret.emplace_back( x, coords[0].second );
            }
        }
        else if ( isDiagonal() )
        {
            /* diagonal line */
            int x_min, x_max, y, dy;
            if ( coords[0].first < coords[1].first )
            {
                x_min = coords[0].first;
                x_max = coords[1].first;
                y = coords[0].second;
                if ( coords[0].second < coords[1].second)
                {
                    dy = 1;
                }
                else
                {
                    dy = -1;
                }
            }
            else
            {
                x_min = coords[1].first;
                x_max = coords[0].first;
                y = coords[1].second;
                if ( coords[1].second < coords[0].second)
                {
                    dy = 1;
                }
                else
                {
                    dy = -1;
                }
            }

            for ( int x = x_min ; x <= x_max; ++x, y += dy )
            {
                ret.emplace_back( x, y );
            }
        }

        return ret;
    }
};

class Grid
{
public:
    Grid()
    {} 

    void DrawLine(const Line& l)
    {
        for (auto &p : l.getPoints())
        {
            map[p]++;
        }
    }

    int getSafePoints()
    {
        int ret = 0;

        for ( auto& p : map )
        {
            if ( p.second >= 2 ) ret++;
        }

        return ret;
    }

    void drawGrid()
    {
        int x_min = std::numeric_limits<int>::max();
        int x_max = std::numeric_limits<int>::min();

        int y_min = std::numeric_limits<int>::max();
        int y_max = std::numeric_limits<int>::min();

        for ( auto i = map.begin(); i != map.end(); ++i )
        {
            x_min = std::min( x_min, i->first.first);
            x_max = std::max( x_max, i->first.first);
            y_min = std::min( y_min, i->first.second);
            y_max = std::max( y_max, i->first.second);
        }

        for (int y = y_min; y<=y_max; ++y)
        {
            for (int x = x_min; x <=x_max; ++x)
            {
                auto p = map.find( std::make_pair(x, y) );

                if ( p == map.end() )
                {
                    std::cout << ".";
                }
                else if (p->second > 9)
                {
                    std::cout << "*";
                }
                else
                {
                    std::cout << p->second;
                }
            }
            std::cout << std::endl;
        }
    }

    std::map<std::pair<int, int>, int> map;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Line> lines;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^\\s*([0-9]+)\\s*,\\s*([0-9]+)\\s*-[>]\\s*([0-9]+)\\s*,\\s*([0-9]+)\\s*$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                lines.emplace_back( 
                    std::stoi(match[1].str()), std::stoi(match[2].str()),
                    std::stoi(match[3].str()), std::stoi(match[4].str())
                );
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    if (lines.empty() )
    {
        std::cerr << "No input data" << std::endl;
        exit(-1);        
    }

    Grid grid;

    for ( auto& l : lines )
    {
        if ( l.isHorizontal() | l.isVertical() )
        {
            grid.DrawLine(l);
        }
    }

    // grid.drawGrid();
    std::cout << "Number of safe points : " << grid.getSafePoints() << std::endl;

    for ( auto& l : lines )
    {
        if ( l.isDiagonal() )
        {
            grid.DrawLine(l);
        }
    }

    // grid.drawGrid();
    std::cout << "Number of safe points : " << grid.getSafePoints() << std::endl;

    return 0;
}
