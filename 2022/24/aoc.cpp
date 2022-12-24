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
#include <future>
#include <functional>

static constexpr bool do_debug(false);

struct Blizzard
{
public:

    Blizzard(std::size_t size, char a_forward_symbol, char a_backward_symbol) : m_Size(size), forward_symbol(a_forward_symbol), backward_symbol(a_backward_symbol)
    {
    }

    void addForward(std::size_t position)
    {
        m_Forward.insert(position);
    }

    void addBackward(std::size_t position)
    {
        m_Backward.insert(position);
    }

    bool isOccupied(std::size_t position, std::size_t a_Cycle) const
    {
        a_Cycle %= m_Size;
        position %= m_Size;

        for (auto &f: m_Forward)
        {
            if (((m_Size + f + a_Cycle - position)%m_Size) == 0) return true;
        }

        for (auto &f: m_Backward)
        {
            if (((2*m_Size + f - a_Cycle - position)%m_Size) == 0) return true;
        }

        return false;
    }

    char getStatus(std::size_t position, std::size_t a_Cycle) const
    {
        char ret = '.';

        a_Cycle %= m_Size;
        position %= m_Size;

        for (auto &f: m_Forward)
        {
            if (((m_Size + f + a_Cycle - position)%m_Size) == 0)
            {
                ret = forward_symbol;
                break;
            }
        }

        for (auto &f: m_Backward)
        {
            if (((2*m_Size + f - a_Cycle - position)%m_Size) == 0) 
            {
                if (ret == '.')
                {
                    ret = backward_symbol;
                }
                else
                {
                    ret = '2';
                }

                break;
            }
        }

        return ret;
    }

private:
    std::size_t m_Size;
    std::set<std::size_t> m_Forward;
    std::set<std::size_t> m_Backward;

    char forward_symbol;
    char backward_symbol;
};

class Coordinate
{
public:
    Coordinate() : x(std::numeric_limits<std::size_t>::max()), y(std::numeric_limits<std::size_t>::max()) {};
    Coordinate(std::size_t _x, std::size_t _y) : x(_x), y(_y) {};

    std::size_t x;
    std::size_t y;

    bool operator<(const Coordinate& rhs) const
    {
        if (y < rhs.y) return true;
        if (y == rhs.y) return x < rhs.x;
        return false;
    }

    bool operator==(const Coordinate& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y); 
    }

    bool isValid() const
    {
        return (x != std::numeric_limits<std::size_t>::max()) && (y != std::numeric_limits<std::size_t>::max());
    }
};

std::ostream& operator<<(std::ostream& os, const Coordinate& co)
{   
    if (co.isValid())
    {
        os << "(" << co.x << "," << co.y << ")";
    }
    else
    {
        os << "(INVALID)";
    }
    return os;
}

class Grid
{
public:
    Grid(std::size_t width, const std::vector<std::string>& lines, std::size_t a_entrance_pos, std::size_t a_exit_pos) : entrance_pos(a_entrance_pos), exit_pos(a_exit_pos)
    {
        for (std::size_t i=0; i<width; i++)
        {
            cols.emplace_back(lines.size(), 'v', '^');
        }

        for (std::size_t i=0; i<lines.size(); i++)
        {
            rows.emplace_back(width, '>', '<');
        }

        for (std::size_t y = 0; y<lines.size(); ++y)
        {
            const std::string& line = lines[y];
            for (std::size_t x = 0; x<width; ++x)
            {
                switch (line[x])
                {
                case '.':
                    break;
                case 'v':
                    cols[x].addForward(y);
                    break;
                case '^':
                    cols[x].addBackward(y);
                    break;
                case '>':
                    rows[y].addForward(x);
                    break;
                case '<':
                    rows[y].addBackward(x);
                    break;
                default:
                    throw std::invalid_argument("Unknown character in input");
                    break;
                }
            }
        }
    }

    bool isOccupied(const Coordinate& pos, std::size_t cycle=0) const
    {
        return rows.at(pos.y).isOccupied(pos.x, cycle) || cols.at(pos.x).isOccupied(pos.y, cycle);
    }

    std::size_t walk() const
    {
        return walk(Coordinate(entrance_pos, 0), Coordinate(exit_pos, rows.size()-1), 1);
    }

    std::size_t walkandreturn() const
    {
        Coordinate at_entrance(entrance_pos, 0);
        Coordinate at_exit(exit_pos, rows.size()-1);

        std::size_t first_leg = walk(at_entrance, at_exit, 1);
        std::size_t second_leg = walk(at_exit, at_entrance, first_leg+1);
    
        return walk(at_entrance, at_exit, second_leg+1);
    }

    std::size_t walk(const Coordinate& start, const Coordinate& destination, std::size_t first_cycle) const
    {

        while (true)
        {
            /* First find first cycle which has the start position unoccupied */
            while (isOccupied(start, first_cycle+1))
            {
                first_cycle++;
            }

            std::size_t cycle = first_cycle++;

            std::pair<bool, std::set<Coordinate>> state;
            state.first = false;
            state.second.insert(start);

            while (! state.second.empty())
            {
                cycle++;
                auto ret = walk(state.second, cycle, destination);

                if (ret.first)
                    return cycle;

                state = ret;
            }

            /* If we end up here it was not possible to reach the target.  That means we wait until the next opportunity arises to start
             * the travel.
             */
        }

        return std::numeric_limits<std::size_t>::max();
    }

    void Draw(std::ostream& os, std::size_t cycle=0) const
    {
        std::string buffer(cols.size()+2, '#');
        buffer[entrance_pos+1] = '.';

        os << buffer << std::endl;

        for (std::size_t y=0; y<rows.size(); ++y)
        {
            for (std::size_t x=0; x<cols.size(); ++x)
            {
                char horiz_state = rows[y].getStatus(x, cycle);
                char vert_state = cols[x].getStatus(y, cycle);

                if (horiz_state == '.')
                {
                    buffer[x+1] = vert_state;
                }
                else if (vert_state == '.')
                {
                    buffer[x+1] = horiz_state;
                }
                else 
                {
                    char n = '.';
                    if (horiz_state == '2')
                    {
                        n = '2';
                    }
                    else
                    {
                        n = '1';
                    }

                    if (vert_state == '2')
                    {
                        n += 2;
                    }
                    else
                    {
                        n += 1;
                    }

                    buffer[x+1] = n;
                }
            }

            os << buffer << std::endl;
        }

        for (std::size_t x=0; x<cols.size(); ++x)
        {
            if (x == exit_pos)
            {
                buffer[x+1] = '.';
            }
            else
            {
                buffer[x+1] = '#';
            }
        }

        os << buffer << std::endl;
    }

private:

    std::vector<Coordinate> getNeighbours(const Coordinate& pos) const
    {
        std::vector<Coordinate> ret;
        ret.reserve(4);

        /* Put the ones where we go up or left first */
        if (pos.x > 0)
        {
            ret.emplace_back(pos.x-1, pos.y);
        }
        if (pos.y > 0)
        {
            ret.emplace_back(pos.x, pos.y-1);
        }        
        if (pos.x+1 < cols.size())
        {
            ret.emplace_back(pos.x+1, pos.y);
        }
        if (pos.y+1 < rows.size())
        {
            ret.emplace_back(pos.x, pos.y+1);
        }

        return ret;
    }

    std::pair<bool, std::set<Coordinate>> walk(const std::set<Coordinate>& pos, std::size_t cycle, const Coordinate& destination) const
    {
        std::set<Coordinate> newpos;

        for (auto &p : pos)
        {
            if (p == destination)
                /* We have arrived */
                return std::make_pair(true, std::set<Coordinate>());

            if (newpos.find(p) == newpos.end())
            {
                if (! isOccupied(p, cycle))
                {
                    newpos.insert(p);
                }
            }

            /* Check if we can move to any of our neighbour positions in the next cycle */
            for (auto &n: getNeighbours(p))
            {
                /* Check if this position has already been reached by another path in this cycle */
                if (newpos.find(n) == newpos.end())
                {
                    /* If not, test it */
                    if (! isOccupied(n, cycle))
                    {
                        newpos.insert(n);
                    }
                }
            }
        }

        return std::make_pair(false, newpos);
    }

    std::vector<Blizzard> rows;
    std::vector<Blizzard> cols;
    std::size_t entrance_pos;
    std::size_t exit_pos;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    std::size_t grid_width = 0;
    std::size_t entrance_pos = 0;
    std::size_t exit_pos = 0;
    std::vector<std::string> lines;
    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line) && (exit_pos == 0)) 
        {
            if (line.empty())
                continue;

            if (grid_width==0)
            {
                /* This is the first line, find the entrance position and the grid width */
                grid_width = line.size()-2;

                for (entrance_pos = 0; entrance_pos < grid_width; ++entrance_pos)
                {
                    if (line[entrance_pos+1] == '.')
                        break;
                }
            }
            else if (line.find_first_not_of("#.") == std::string::npos)
            {
                /* This is the last line, find the exit */
                for (exit_pos = 0; exit_pos < grid_width; ++exit_pos)
                {
                    if (line[exit_pos+1] == '.')
                        break;
                }
            }
            else if (line.size() != grid_width+2)  
            {
                throw std::invalid_argument("can't parse input file");
            }
            else
            {
                lines.push_back(line.substr(1, grid_width));
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    Grid grid(grid_width, lines, entrance_pos, exit_pos);

    auto scoreA = grid.walk();
    std::cout << "Best path A " << scoreA << std::endl;

    auto scoreB = grid.walkandreturn();
    std::cout << "Best path B " << scoreB << std::endl;

    return 0;
}
