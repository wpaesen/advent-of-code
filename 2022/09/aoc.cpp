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

struct Command {
    enum Direction : std::size_t {
        NORTH = 0,
        EAST  = 1,
        SOUTH = 2,
        WEST  = 3
    };

    Command() : dir(NORTH), steps(0) {};
    Command(const std::string& cmd) : line(cmd) 
    {
        if (cmd.size() < 3)
            throw std::invalid_argument("Short command");

        switch (cmd[0])
        {
        case 'R':
            dir = EAST;
            break;
        case 'U':
            dir = NORTH;
            break;
        case 'L':
            dir = WEST;
            break;
        case 'D':
            dir = SOUTH;
            break;
        default:
            throw std::invalid_argument("Unknown direction");
            break;
        }

        steps = std::stoi(cmd.substr(2));
    }

    Direction dir;
    std::size_t steps;
    std::string line;
};

template<std::size_t N>
class Space
{
public:
    
    struct Knot
    {  
        Knot() : lead(nullptr), name('#') { pos.fill(0); };

        std::array<int, 2> pos;
        Knot* lead;

        char name;

        operator Knot* () {
            return this;
        }

        void step(const Command& cmd)
        {
            if (lead != nullptr)
                throw std::invalid_argument("Can't direction step on knot with lead");

            switch (cmd.dir)
            {
            case Command::NORTH: pos[1] += 1; break;
            case Command::EAST:  pos[0] += 1; break;
            case Command::SOUTH: pos[1] -= 1; break;
            case Command::WEST:  pos[0] -= 1; break;
            }
        }

        void step()
        {
            if (lead == nullptr)
                throw std::invalid_argument("Can't tail step on knot without lead");

            std::array<int, 2>& head = lead->pos;
            /* Check what the tail (me) has to do */
            if (head[0] == 0)
            {
                if (std::abs(head[1]) > 1)
                {
                    int delta = (head[1] > 0) ? 1 : -1;

                    pos[1] += delta;
                    head[1] -= delta;
                }
            }
            else if (head[1] == 0)
            {
                if (std::abs(head[0]) > 1)
                {
                    int delta = (head[0] > 0) ? 1 : -1;

                    pos[0] += delta;
                    head[0] -= delta;
                }
            }
            else if (((std::abs(head[0]) > 1) || std::abs(head[1]) > 1))
            { 
                /* We need to do a diagonal step. On step in each direction */
                for (std::size_t i=0; i<2; ++i)
                {
                    int delta = (head[i]) > 0 ? 1 : -1; 
                    pos[i] += delta;
                    head[i] -= delta;
                }
            }
      
            for (std::size_t i=0; i<2; ++i)
            {
                if (head[i] > 1)
                    throw std::runtime_error("BUG, head broke away from tail");
            }
        }
    };


    Space() {

        for (auto i = knots.begin(); i != knots.end(); i = std::next(i))
        {
            auto j = std::next(i);
            if (j != knots.end())
            {
                i->lead = j;
            }
        }

        char name = '0';
        for (auto i = knots.rbegin(); i != knots.rend(); i = std::next(i), name += 1)
        {
            i->name = name;
        }

        knots.begin()->name = 'T';
        knots.rbegin()->name = 'H';
    };

    void run(const std::vector<Command> &commands)
    {
        for (auto& c : commands)
        {
            runCommand(c);
        }
    }

    void runCommand(const Command& command)
    {
        if ((N > 2) && (do_debug))
            std::cout << "====== " << command.line << " ==== " << std::endl; 

        for (std::size_t i = 0; i<command.steps; ++i)
        {
            step(command);
        }

        print_step();
    }

    std::size_t countVisitedSites() const
    {
        return visits.size();
    }

private:

    /* debug helpers */
    struct pos
    {
        pos() : x(0), y(0), name('.') {};
        pos(int val) : x(val), y(val), name('.') {}
        pos(const Knot& rhs) : x(rhs.pos[0]), y(rhs.pos[1]), name(rhs.name) {};
        pos(int a_x, int a_y, char a_name) : x(a_x), y(a_y), name(a_name){};

        int x;
        int y;
        char name;

        int operator[](std::size_t i) const
        {
            if (i == 0) return x;
            return y;
        }

        int& operator[](std::size_t i) 
        {
            if (i == 0) return x;
            return y;
        }

        pos operator+(const Knot& rhs)
        {
            pos ret;

            ret.x = x + rhs.pos[0];
            ret.y = y + rhs.pos[1];
            ret.name = rhs.name;

            return ret;
        }
    };

    void print(const std::vector<pos>& positions) const
    {
        pos maxpos(std::numeric_limits<int>::min());
        pos minpos(std::numeric_limits<int>::max());

        for (auto& p : positions)
        {
            for (std::size_t i=0; i<2; ++i)
            {
                if (maxpos[i] < p[i])
                    maxpos[i] = p[i];

                if (minpos[i] > p[i])
                    minpos[i] = p[i];
            }
        }

        if ((maxpos[0] - minpos[0]) < 6)
        {
            maxpos[0] = minpos[0] + 6;
        }
        if ((maxpos[1] - minpos[1]) < 5)
        {
            maxpos[1] = minpos[1] + 5;
        }

        std::string line;
        line.append( 1 + maxpos.x - minpos.x, '.');

        std::vector<std::string> grid;
        for (int i = minpos.y; i<= maxpos.y; ++i)
        {
            grid.push_back(line);
        }

        for (auto& p : positions)
        {
            int y = p.y - minpos.y;
            int x = p.x - minpos.x;

            grid[y][x] = p.name;
        }

        for (auto i = grid.rbegin(); i != grid.rend(); i = std::next(i))
        {
            std::cout << *i << std::endl;
        }
        std::cout << std::endl;
    }

    void print_step() const
    {
        if (! do_debug)
            return;

        if (N <= 2) return;

        std::vector<pos> positions;
        positions.reserve(N+1);

        positions.emplace_back(pos());
        positions.back().name = 's';

        for (auto& k : knots)
        {
            positions.emplace_back(positions.back() + k);
        }

        print(positions);
    }

    void step(const Command& cmd)
    {
        auto j = knots.rbegin();
        j->step(cmd);

        for (j = std::next(j); j != knots.rend(); j = std::next(j))
        {
            j->step();
        }

        visits.insert(knots.begin()->pos);
    }

    /* 0 is the tail, N-1 is the head. Apart from the tail, every knot's position
     * is expressed relative to it's previous knot.
     */
    std::array<Knot, N> knots;

    std::set<std::array<int, 2>> visits;
public:

    void print_visits() const
    {
        if (! do_debug)
            return;

        std::vector<pos> positions;

        positions.emplace_back(0,0,'s');

        for (auto& v : visits)
        {
            if ((v[0] != 0) || (v[1] != 0))
            {
                positions.emplace_back(v[0], v[1], '#');
            }
        }

        print(positions);
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

    std::vector<Command> commands;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            commands.emplace_back(line);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    Space<2> shortRope;
    shortRope.run(commands);
    std::cout << "Short rope " << shortRope.countVisitedSites() << " positions visited" << std::endl;

    Space<10> longRope;
    longRope.run(commands);
    longRope.print_visits();

    std::cout << "Long rope " << longRope.countVisitedSites() << " positions visited" << std::endl;

    return 0;
}
