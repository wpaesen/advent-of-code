#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <set>
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
#include <tuple>

class CucumberGrid
{
public:

    CucumberGrid() {
        m_symbols[0] = '>';
        m_symbols[1] = 'v';
        m_symbols[2] = 's';
        m_symbols[3] = 'V';
        generation = 0;
        steps = 0;
    };

    void addRow(const std::string& l)
    {
        if (! m_grid.empty())
        {
            if (l.length() != m_grid[0].length())
                throw std::invalid_argument("Invalid line length");
        }

        m_grid.push_back(l);
    }

    char& operator() (int x, int y) 
    {
        int real_x = x % getWidth();
        int real_y = y % getHeight();

        return m_grid[real_y][real_x];
    }

    bool iterate()
    {
        bool hasupdates = false;

        char east_org = m_symbols[(generation&0x1)?2:0];
        char east_new = m_symbols[(generation&0x1)?0:2];

        for (int y = 0; y < getHeight(); ++y)
        {
            for (int x = 0; x < getWidth(); ++x)
            {
                auto& cell = operator() (x, y);
                if (cell == east_org)
                {
                    auto& next = operator() (x+1, y);

                    if (next == '.')
                    {
                        if ( x == 0 )
                        { 
                            cell = '*';
                        }
                        else
                        {
                            cell = '.';
                        }

                        next = east_new;
                        hasupdates = true;
                    }
                    else
                    {
                        cell = east_new;
                    }
                }
            }

            {
                auto &first = operator() (0, y);
                if ( first == '*' )
                {
                    first = '.';
                }
            }
        }

        char south_org = m_symbols[(generation&0x1)?3:1];
        char south_new = m_symbols[(generation&0x1)?1:3];

        for (int x = 0; x < getWidth(); ++x)
        {
            for (int y = 0; y < getHeight(); ++y)
            {
                auto& cell = operator() (x, y);
                if (cell == south_org)
                {
                    auto& next = operator() (x, y+1);

                    if (next == '.')
                    {
                        if ( y == 0 )
                        {
                            cell = '*';
                        }
                        else
                        {
                            cell = '.';
                        }

                        next = south_new;
                        hasupdates = true;
                    }
                    else
                    {
                        cell = south_new;
                    }
                }
            }

            {
                auto& first = operator() (x, 0);
                if (first == '*')
                {
                    first = '.';
                }
            }
        }

        generation++;
        if (hasupdates) steps++;

        return hasupdates;
    }

    int getWidth() const
    {
        if (m_grid.empty())
            throw std::invalid_argument("Empty grid doesn't have width");

        return m_grid[0].length();
    }

    int getHeight() const
    {
        return m_grid.size();
    }

    std::vector<std::string> m_grid;
    std::array<char, 4> m_symbols;
    int generation;
    int steps;

    void print(std::ostream& s) const
    {
        for (auto& l : m_grid)
        {
            std::string l2(l);
            for (auto& c : l2)
            {
                if (c == m_symbols[2])
                {
                    c = m_symbols[0];
                }
                if (c == m_symbols[3])
                {
                    c = m_symbols[1];
                }
            }
            s << l2 << std::endl;
        }
    }
};


std::ostream& operator<<(std::ostream& s, const CucumberGrid& t)
{
    t.print(s); return s;
}

int
main(int argc, char **argv)
{

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    CucumberGrid grid;

    try
    {
        std::ifstream infile(argv[1]);

        std::regex matchrule{ "^\\s*([.v>]+)\\s*$" };

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match( line, match, matchrule))
            {
                grid.addRow(match[1].str());
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    while (grid.iterate());

    std::cout << "Grid stable after " << grid.generation << " steps" << std::endl;
    
    return 0;
}