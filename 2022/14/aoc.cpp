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

struct Pixel
{
    Pixel() : x(0), y(0) {}
    Pixel(int _x, int _y) : x(_x), y(_y) {};

    int x;
    int y;

    bool operator<(const Pixel& rhs) const
    {
        return (x < rhs.x) || (y < rhs.y); 
    }

    bool operator>(const Pixel& rhs) const
    {
        return (x > rhs.x) || (y > rhs.y);
    }

    void print(std::ostream& os) const
    {
        os << x << "," << y;
    }

    Pixel operator-(const Pixel& rhs) const
    {
        return Pixel(x - rhs.x, y - rhs.y);
    }

    Pixel operator+(const Pixel& rhs) const
    {
        return Pixel(x + rhs.x, y + rhs.y);
    }

    bool operator==(const Pixel& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }

    Pixel min(const Pixel& rhs) const
    {
        return Pixel(std::min(x, rhs.x), std::min(y, rhs.y) );
    }

    Pixel max(const Pixel& rhs) const
    {
        return Pixel(std::max(x, rhs.x), std::max(y, rhs.y) );
    }
};

std::ostream& operator<<(std::ostream& os, const Pixel& p)
{
    p.print(os);
    return os;
}

class Rock
{
public:

    /* define an iterator, so we can iterate over each rock pixel */
    class iterator
    {
    public:

        iterator(const Rock& a_rock) : rock(a_rock), end(false) {
            if (rock.vertices.empty())
                throw std::invalid_argument("Ghost rock");

            pixel = rock.vertices.begin();
            pos = *pixel;
            pixel = std::next(pixel);
        };

        explicit iterator(const Rock& a_rock, bool end) : rock(a_rock), end(true)
        {
            pixel = rock.vertices.end();
        }

        iterator& operator++()
        {
            if (pixel != rock.vertices.end())
            {
                const Pixel& dst = *pixel;

                if (pos.x != dst.x)
                {
                    if (pos.x < dst.x)
                    {
                        pos.x++;
                    }
                    else 
                    {
                        pos.x--;
                    }

                    if (pos.x == dst.x)
                    {
                        pixel = std::next(pixel);
                    }
                }
                else if (pos.y != dst.y)
                {
                    if (pos.y < dst.y)
                    {
                        pos.y++;
                    }
                    else 
                    {
                        pos.y--;
                    }

                    if (pos.y == dst.y)
                    {
                        pixel = std::next(pixel);
                    }
                }
                else
                {
                    throw std::runtime_error("Invalid line segment");
                }
            } else {
                end = true;
            }

            return *this;
        }

        Pixel& operator*()
        {
            return pos;
        }

        bool operator==(const iterator& other) const
        {
            if (end == other.end) return true;
            return (pixel == other.pixel) && (pos == other.pos);
        }

        bool operator!=(const iterator& other) const
        {
            return !(*this == other);
        }

    private:

        std::vector<Pixel>::const_iterator pixel;
        const Rock& rock;
        Pixel pos;
        bool end;
    };

    Rock(const std::string& line)
    {
        std::size_t head = 0;

        while (head != std::string::npos)
        {
            std::size_t tail = line.find("->", head);
            if (tail == std::string::npos)
            {
                addVertex(line.substr(head));
                break;
            }
            else
            {
                addVertex(line.substr(head, tail-head));
                head = tail+2;
            }
        }
    }

    Rock(const Pixel& p1, const Pixel& p2)
    {
        vertices.push_back(p1);
        vertices.push_back(p2);
    }

    std::vector<Pixel> vertices;

    void print(std::ostream& os) const
    {
        std::string sep = "";

        for (auto& v : vertices)
        {
            std::cout << sep;
            std::cout << v.x << "," << v.y;
            if (sep.empty())
                sep = " -> ";
        }
    }

    iterator begin() const
    {
        return iterator(*this);
    }

    iterator end() const
    {
        return iterator(*this, true);
    }

private:

    void addVertex(const std::string& vertex)
    {
        auto sep = vertex.find(",");
        if (sep == std::string::npos)
            throw std::invalid_argument("vertex needs to have 2 numbers");

        vertices.emplace_back( std::stoi(vertex.substr(0, sep)), std::stoi(vertex.substr(sep+1)));
    }
};

std::ostream& operator<<(std::ostream& os, const Rock& r)
{
    r.print(os);
    return os;
}

class Grid
{
public:

    Grid(bool a_withfloor = false) : initialised(false), upper_left(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()), lower_right(std::numeric_limits<int>::min(), std::numeric_limits<int>::min()), withfloor(a_withfloor){
        updateSize(Pixel(500, 0));

    };

    void placeRocks(const std::vector<Rock>& rocks)
    {
        for (auto& r : rocks)
        {
            updateSize(r);
        }

        if (withfloor)
        {
            /* calculate floor as a Rock and add it to the graph.  Could also be done by a simple equation, but why bother. */
            Pixel p1(500, 0);
            Pixel p2(500, 0);

            while (p1.y < lower_right.y + 2)
            {
                p1.y += 1;
                p1.x -= 1;
                p2.y += 1;
                p2.x += 1;
            }

            floor = std::make_unique<Rock>(p1, p2);
            updateSize(*floor);
        }   

        std::size_t rows = 1 + lower_right.y - upper_left.y;
        std::size_t cols = 1 + lower_right.x - upper_left.x;

        for (std::size_t r = 0; r < rows; ++r)
        {
            m_Grid.emplace_back(cols, '.');
        }

        initialised = true;

        for (auto& r : rocks)
        {
            addRock(r);
        }

        if (floor)
        {
            addRock(*floor);
        }

        at(500, 0) = '+';        
    }

    const Pixel& getUpperLeft() const { return upper_left; }
    const Pixel& getLowerRight() const { return lower_right; }

    char& at(int x, int y) 
    {
        if (! initialised)
            return offgrid;

        if ((x < upper_left.x) || (x > lower_right.x))
            return offgrid;
        if ((y < upper_left.y) || (y > lower_right.y))
            return offgrid;

        return m_Grid[y - upper_left.y][x - upper_left.x];
    }

    char& at(const Pixel& p)
    {
        return at(p.x, p.y);
    }

    const char& at(int x, int y) const
    {
        if (! initialised)
            return offgrid;

        if ((x < upper_left.x) || (x > lower_right.x))
            return offgrid;
        if ((y < upper_left.y) || (y > lower_right.y))
            return offgrid;

        return m_Grid[y - upper_left.y][x - upper_left.x];
    }

    const char& at(const Pixel& p) const
    {
        return at(p.x, p.y);
    }

    void print(std::ostream& os) const
    {
        for (auto& r : m_Grid)
        {
            os << r << std::endl;
        }
    }

    bool isBlocked() const
    {
        return at(Pixel(500, 0)) == 'o';
    }

    bool drop()
    {
        Pixel p(500, 0);

        if (isBlocked()) return false;

        while (p.y <= lower_right.y)
        {
            int new_y = p.y + 1;

            offgrid = '.';
            if (at(p.x, new_y) == '.')
            {
                p.y = new_y;
            }
            else if (at(p.x -1, new_y) == '.')
            {
                p.y = new_y;
                p.x -= 1;
            }
            else if (at(p.x+1, new_y) == '.')
            {
                p.y = new_y;
                p.x += 1;
            }
            else
            {
                at(p) = 'o';
                return true;
            }
        }

        return false;
    }

private:

    void addRock(const Rock& r)
    {
        for (auto &p : r)
        {
            at(p) = '#';
        }
    }

    void updateSize(const Rock& r)
    {
        for (auto& p : r.vertices)
        {
            updateSize(p);
        }
    }

    void updateSize(const Pixel& p)
    {
        upper_left = upper_left.min(p);
        lower_right = lower_right.max(p);
    }

    bool initialised;
    Pixel upper_left;
    Pixel lower_right;
    char offgrid;
    bool withfloor;
    std::unique_ptr<Rock> floor;

    std::vector<std::string> m_Grid;
};

std::ostream& operator<<(std::ostream& os, const Grid& g)
{
    g.print(os);
    return os;
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Rock> rocks;
    try
    {
        std::ifstream infile(argv[1]);

        std::vector<std::string> lines;

        std::string line;
        while (std::getline(infile, line))
        {
            rocks.emplace_back(line);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    Grid grid;
    grid.placeRocks(rocks);

    std::size_t i=0;
    while (grid.drop())
    {
        ++i;
    }

    std::cout << "Floorless cave filled after " << i << " iterations" << std::endl;

    Grid grid2(true);
    grid2.placeRocks(rocks);

    i = 0;
    while (grid2.drop())
    {
        ++i;
    }

    std::cout << "Floored cave filled after " << i << " iterations" << std::endl;

    return 0;
}
