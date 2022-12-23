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

struct Limit
    {
        Limit()
        {
            min = std::numeric_limits<int64_t>::max();
            max = std::numeric_limits<int64_t>::min();
        }

        void update(int64_t value)
        {
            if (value < min)
                min = value;

            if (value > max)
                max = value;
        }

        bool isValid() const
        {
            return (min != std::numeric_limits<int64_t>::max()) && (max != std::numeric_limits<int64_t>::min());
        }

        int64_t length() const
        {
            return 1 + max - min;
        }

        bool operator==(const Limit& rhs) const
        {
            return (min == rhs.min) && (max == rhs.max);
        }

        bool operator !=(const Limit& rhs) const
        {
            return (min != rhs.min) || (max != rhs.max);
        }

        int64_t min;
        int64_t max;
    };

class Elf
{
public:
    Elf(int64_t _x, int64_t _y) : pos({_y, _x}) {};

    std::array<int64_t, 2> pos;
    std::array<int64_t, 2> newpos;
    char move;
};

template<std::size_t N>
std::array<int64_t, N> operator+(const std::array<int64_t, N>& lhs, const std::array<int64_t,N>& rhs)
{
    std::array<int64_t, N> ret;

    for (std::size_t i=0; i<N; ++i)
    {
        ret[i] = lhs[i] + rhs[i];
    }

    return ret;
}

template<std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<int64_t, N>& rhs)
{
    char tok = '(';

    for (auto c = rhs.rbegin(); c != rhs.rend(); c = std::next(c))
    {
        os << tok << *c;
        tok = ',';
    }

    os << ')';
    return os;
}

enum Direction : std::size_t {
        NW    = 0,
        NORTH = 1,
        NE    = 2,
        WEST  = 3,
        EAST  = 4,
        SW    = 5,
        SOUTH = 6,
        SE    = 7
    };

std::ostream& operator<<(std::ostream& os, const Direction& rhs)
{
    switch (rhs)
    {
    case NW:
        os << "NW";
        break;
    case NORTH:
        os << "NORTH";
        break;        
    case NE:
        os << "NE";
        break;
    case WEST:
        os << "WEST";
        break;
    case EAST:
        os << "EAST";
        break;
    case SW:
        os << "SW";
        break;
    case SOUTH:
        os << "SOUTH";
        break;
    case SE:  
        os << "SE";
        break;
    }
   
    return os;
}


class Grid
{
public:

    static constexpr std::array<std::array<int64_t, 2>, 8> check_neighbour{{
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    }};

    Grid() : max_y(0), max_x(0) {
        {
            /* If there is no Elf in the N, NE, or NW adjacent positions, the Elf proposes moving north one step. */
            std::bitset<8> mask;
            mask.set(NORTH);
            mask.set(NE);
            mask.set(NW);
            m_DanceMoves.emplace_back(mask, NORTH, '^');
        }

        {
            /* If there is no Elf in the S, SE, or SW adjacent positions, the Elf proposes moving south one step. */
            
            std::bitset<8> mask;
            mask.set(SOUTH);
            mask.set(SE);
            mask.set(SW);
            m_DanceMoves.emplace_back(mask, SOUTH, 'v');
        }

        {
            /* If there is no Elf in the W, NW, or SW adjacent positions, the Elf proposes moving west one step. */
            std::bitset<8> mask;
            mask.set(WEST);
            mask.set(NW);
            mask.set(SW);
            m_DanceMoves.emplace_back(mask, WEST, '<');
        }

        {
            /* If there is no Elf in the E, NE, or SE adjacent positions, the Elf proposes moving east one step. */
            std::bitset<8> mask;
            mask.set(EAST);
            mask.set(NE);
            mask.set(SE);
            m_DanceMoves.emplace_back(mask, EAST, '>');
        }
    };

    Elf* operator[](const std::array<int64_t,2 >& pos)
    {
        auto elf = m_Grid.find(pos);
        if (elf != m_Grid.end())
        {
            return elf->second.get();
        }
        return nullptr;
    }

    const Elf* operator[](const std::array<int64_t,2 >& pos) const
    {
        auto elf = m_Grid.find(pos);
        if (elf != m_Grid.end())
        {
            return elf->second.get();
        }
        return nullptr;
    }

    void addLine(const std::string& l)
    {
        for (int x=0; x <l.size(); ++x)
        {
            if (l[x] == '#')
            {
                m_Grid.emplace(std::array<int64_t, 2>({max_y, x}), std::make_shared<Elf>(x, max_y));
            }
        }

        if (l.size() > max_x)
            max_x = l.size();

        max_y++;
    }

    std::bitset<8> getNeighbours(const std::array<int64_t,2 >& pos) const
    {
        std::bitset<8> ret;

        for (std::size_t i=0; i<check_neighbour.size(); ++i)
        {
            auto p = pos + check_neighbour[i];
            if ((*this)[p])
            {
                ret.set(i);
            }
        }

        return ret;
    }

    void print(std::ostream& os, bool movemap = false) const
    {
        std::array<int64_t, 2> pos;

        std::array<Limit, 2> limits;

        limits[0].update(-2);
        limits[0].update(9);
        limits[1].update(-3);
        limits[1].update(10);

        for (auto & elf : m_Grid)
        {
            for (std::size_t i=0; i<elf.first.size(); ++i)
            {
                limits[i].update(elf.first[i]);
            }
        }

        for (pos[0] = limits[0].min; pos[0] <= limits[0].max; ++pos[0])
        {
            for (pos[1] = limits[1].min; pos[1]<=limits[1].max; ++pos[1])
            {
                auto elf = (*this)[pos];
                if (elf != nullptr)
                {
                    if (movemap)
                    {
                        os << elf->move;
                    }
                    else
                    {
                        os << "#";
                    }
                }
                else
                {
                    os << ".";
                }
            }
            os << std::endl;
        }
    }

    bool dance_1()
    {
        std::map<std::array<int64_t, 2>, std::vector<std::shared_ptr<Elf>>> moves;

        std::size_t first_direction = std::numeric_limits<std::size_t>::max();

        for (auto& elf : m_Grid)
        {
            auto neighbours = getNeighbours(elf.second->pos);     

            char movechar = '#';
            if (neighbours.any())
            {
                for (std::size_t i=0; i<m_DanceMoves.size(); ++i)
                {
                    auto& move = m_DanceMoves[i];
                    if ((neighbours & std::get<0>(move)).none())
                    {
                        if (first_direction > m_DanceMoves.size())
                            first_direction = i;

                        elf.second->newpos = elf.second->pos + check_neighbour[std::get<1>(move)];
                        movechar = std::get<2>(move);

                        moves[elf.second->newpos].push_back(elf.second);
                        break;
                    }
                }
            }
           
            elf.second->move = movechar;
        }

        if (moves.size() == 0)
            return false;

        for (auto &move : moves)
        {
            if (move.second.size() == 1)
            {
                move.second[0]->pos = move.second[0]->newpos;
            }
        }

        std::map<std::array<int64_t, 2>, std::shared_ptr<Elf>> newGrid;
        for (auto& elf : m_Grid)
        {
            newGrid.emplace(elf.second->pos, elf.second);
        }

        first_direction = 0;
        if (first_direction < m_DanceMoves.size())
        {
            m_DanceMoves.push_back(m_DanceMoves[first_direction]);
            auto iter = m_DanceMoves.begin();
            std::advance(iter, first_direction);
            m_DanceMoves.erase(iter);
        }

        m_Grid = newGrid;

        return true;
    }

    int64_t countEmptySpots() const
    {
        std::array<Limit, 2> limits;

        for (auto & elf : m_Grid)
        {
            for (std::size_t i=0; i<elf.first.size(); ++i)
            {
                limits[i].update(elf.first[i]);
            }
        }

        int64_t max_tiles = limits[0].length() * limits[1].length();
        max_tiles -= m_Grid.size();

        return max_tiles;
    }

private:

    int64_t max_y;
    int64_t max_x;
    std::map<std::array<int64_t, 2>, std::shared_ptr<Elf>> m_Grid;

    std::vector<std::tuple<std::bitset<8>, Direction, char>> m_DanceMoves; 
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

        std::string line;
        while (std::getline(infile, line))
        {
            if (line.empty())
                continue;

            grid.addLine(line);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }


    std::size_t round=0;

    if (do_debug)
    {
        std::cout << "== Initial State ==" << std::endl;
        grid.print(std::cout);
        std::cout << std::endl;
    }

    while (round < 5)
    {
        grid.dance_1();
        round++;

        if (do_debug)
        {
            std::cout << "== End of Round " << round << " ==" << std::endl;
            grid.print(std::cout);
            std::cout << std::endl; 
        }
    }

    while (round < 10)
    {
        grid.dance_1();
        round++;
    }

    if (do_debug)
    {
        std::cout << "== End of Round " << round << " ==" << std::endl;
        grid.print(std::cout);
        std::cout << std::endl;
    }

    std::cout << grid.countEmptySpots() << " empty tiles " << std::endl;

    while (grid.dance_1())
    {
        round++;
    }

    std::cout << round+1 << " rounds until no elves move" << std::endl;

    return 0;
}
