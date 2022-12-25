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
#include <cmath>

struct Limit
{
    Limit()
    {
        min = std::numeric_limits<int>::max();
        max = std::numeric_limits<int>::min();
    }

    void update(int value)
    {
        if (value < min)
            min = value;

        if (value > max)
            max = value;
    }

    bool isValid() const
    {
        return (min != std::numeric_limits<int>::max()) && (max != std::numeric_limits<int>::min());
    }

    int length() const
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

    int min;
    int max;
};

struct Instruction
{
    Instruction(int a_steps) : steps(a_steps), dir('=')
    {}

    Instruction(char a_dir) : steps(0), dir(a_dir)
    {
        if ((dir != 'L') && (dir != 'R'))
            throw std::invalid_argument("Invalid direction");
    }

    int steps;
    char dir;

    void print(std::ostream& os) const
    {
        if (dir == '=')
        {
            os << steps;
        }
        else
        {
            os << dir;
        }
    }
};

std::ostream& operator<<(std::ostream& os, const Instruction& i)
{
    if (i.dir == '=')
    {
        os << i.steps;
    }
    else
    {
        os << i.dir;
    }

    return os;
}

class InstructionList : public std::enable_shared_from_this<InstructionList>
{
public:
    InstructionList() {};
    InstructionList(const std::string& l)
    {
        std::string accu;

        for (auto &c : l)
        {
            if ((c == 'R') || (c == 'L'))
            {
                if (! accu.empty())
                {
                    m_Instructions.emplace_back(std::stoi(accu));
                    accu.clear();
                }
                m_Instructions.emplace_back(c);
            }
            else 
            {
                accu.push_back(c);
            }
        }

        if (! accu.empty())
        {
            m_Instructions.emplace_back(std::stoi(accu));
            accu.clear();
        }
    }

    std::vector<Instruction>::const_iterator begin() const { return m_Instructions.begin(); }
    std::vector<Instruction>::const_iterator end() const { return m_Instructions.end(); }

private:

    std::vector<Instruction> m_Instructions;
};

class Direction
{
public:

    enum DirectionValue : std::size_t {
        EAST  = 0, 
        SOUTH = 1,
        WEST  = 2,
        NORTH = 3,
    };

    static constexpr std::size_t N_DIRECTIONS{4};
    static constexpr std::array<DirectionValue, N_DIRECTIONS> all{EAST, SOUTH, WEST, NORTH};

    Direction() : state(EAST) {}
    Direction(std::size_t d) : state(d%N_DIRECTIONS) {};
    Direction(DirectionValue v) : state(v) {};

    operator std::size_t() const { return state; }
    operator int64_t() const { return nv(); }

    DirectionValue ev() const { return static_cast<DirectionValue>(state); };
    int64_t nv() const { return state%4; }

    operator DirectionValue() const { return ev(); };

    Direction operator+(const Direction &rhs) const
    {
        return Direction((state + rhs.state) % N_DIRECTIONS);
    }

    Direction operator+(int64_t rhs) const
    {
        rhs %= 4;
        if (rhs < 0)
            rhs += 4;

        return Direction((state + rhs) % N_DIRECTIONS);
    }

    Direction operator+(int rhs) const
    {
        return operator+((int64_t)rhs);
    }

    Direction operator-(const Direction &rhs) const
    {
        return Direction((4 + state + rhs.state) % N_DIRECTIONS);
    }

    Direction operator-(int64_t rhs) const
    {
        rhs %= 4;
        if (rhs < 0)
            rhs += 4;

        return Direction((4 + state - rhs) % N_DIRECTIONS);
    }

    Direction operator-(int rhs) const
    {
        return operator-((int64_t)rhs);
    }

    Direction& operator++()
    {
        state = (state+1)%4;
        return *this;
    }

    Direction& operator--()
    {
        state = (4+state-1)%4;
        return *this;
    }

    Direction& operator++(int)
    {
        state = (state+1)%4;
        return *this;
    }

    Direction& operator--(int)
    {
        state = (4+state-1)%4;
        return *this;
    }

    Direction& operator+=(const Direction& rhs)
    {
        state = (state + rhs.state) % N_DIRECTIONS;
        return *this;
    }

    Direction& operator+=(int64_t rhs)
    {
        rhs %= 4;
        if (rhs < 0)
            rhs += 4;

        state = (state + rhs) % N_DIRECTIONS;
        return *this;
    }

    Direction& operator-=(const Direction& rhs)
    {
        state = (4 + state - rhs.state) % N_DIRECTIONS;
        return *this;
    }

    Direction& operator-=(int64_t rhs)
    {
        rhs %= 4;
        if (rhs < 0)
            rhs += 4;

        state = (4 + state - rhs) % N_DIRECTIONS;
        return *this;
    }

    bool operator==(const Direction& rhs) const
    {
        return (state%4) == (rhs.state%4);
    }

    bool operator==(const DirectionValue& rhs) const
    {
        return (state%4) == rhs;
    }

    int steps(const Direction& rhs) const
    {
        return (4 + rhs.state - state)%4;
    }

    std::ostream& print(std::ostream& os) const
    {
        switch (state%4)
        {
        case EAST:
            os << "EAST";
            break;
        case SOUTH:
            os << "SOUTH";
            break;
        case WEST:
            os << "WEST";
            break;            
        case NORTH:
            os << "NORTH";
            break;            
        }

        return os;
    }

private:

    std::size_t state;
};

std::ostream& operator<<(std::ostream& os, const Direction& dir)
{
    dir.print(os);
    return os;
}

class Runner
{
public:
    Runner() : x(0), y(0), direction(Direction::EAST) {};

    int x;
    int y;

    Direction direction;

    int64_t toPassword() const
    {
        int64_t ret = 4 * (x+1);

        ret += 1000 * (y+1);

        ret += direction.nv();

        return ret;
    }
};

class Grid
{
public:

    Grid() {};

    void addRow(const std::string& l)
    {
        int y = m_row_limits.size();
        m_row_limits.push_back(Limit());

        if (m_column_limits.size() < l.size())
        {
            m_column_limits.resize(l.size());
        }

        for (int x = 0; x < l.size(); ++x)
        {
            if (l[x] == '.')
            {
                m_row_limits[y].update(x);
                m_column_limits[x].update(y);
            }
            else if (l[x] == '#')
            {
                m_row_limits[y].update(x);
                m_column_limits[x].update(y);
                m_walls.insert({x, y});
            }
        }
    }

    void runOne(Runner& state, const Instruction& i, int n) const
    {
        (void)n;

        if (i.dir == 'R')
        {
            state.direction++;
        }
        else if (i.dir == 'L')
        {
            state.direction--;
        }
        else 
        {
            int x = state.x;
            int y = state.y;

            for (int j=0; j<i.steps; ++j)
            {
                switch (state.direction.ev())
                {
                case Direction::NORTH:
                    y -= 1;
                    if (y < m_column_limits.at(x).min)
                    {
                        y = m_column_limits.at(x).max;
                    }
                    break;
                case Direction::EAST:
                    x += 1;
                    if (x > m_row_limits.at(y).max)
                    {
                        x = m_row_limits.at(y).min;
                    }
                    break;
                case Direction::SOUTH:
                    y += 1;
                    if (y > m_column_limits.at(x).max)
                    {
                        y = m_column_limits.at(x).min;
                    }
                    break;
                case Direction::WEST:
                    x -= 1;
                    if (x < m_row_limits.at(y).min)
                    {
                        x = m_row_limits.at(y).max;
                    }
                    break;
                }

                if (m_walls.find({x, y}) != m_walls.end())
                    break;

                state.x = x;
                state.y = y;
            }
        }
    }

    Runner run(std::shared_ptr<InstructionList> instructions) const
    {
        for (auto &c : m_column_limits)
        {
            if (! c.isValid())
                throw std::invalid_argument("Column limits wrong");
        }

        for (auto &r : m_row_limits)
        {
            if (! r.isValid())
                throw std::invalid_argument("Row limits wrong");
        }

        Runner state;

        state.x = m_row_limits[0].min;
        state.y = 0;
        state.direction = Direction::EAST;

        while (m_walls.find({state.x, state.y}) != m_walls.end())
        {
            state.x++;
            if (state.x > m_row_limits[0].max)
                throw std::runtime_error("Can't find start position");
        }

        int n = 0;
        for (auto &i : *instructions)
        {
            runOne(state, i, ++n);
        }

        return state;
    }

    std::set<std::array<int, 2>> m_walls;
    std::vector<Limit> m_row_limits;
    std::vector<Limit> m_column_limits;
};

class Face : public std::enable_shared_from_this<Face>
{
public:

    enum FaceId : char {
        BOTTOM = 'D',
        TOP    = 'U',
        FRONT  = 'F',
        BACK   = 'B',
        LEFT   = 'L',
        RIGHT  = 'R'
    };

    static constexpr std::size_t N_FACES{6};

    /* defines which faces are adjacent to each face base on when you look at each face from the outside
     * of the cube, north side up 
     */
    static constexpr std::array<std::pair<Face::FaceId, std::array<Face::FaceId, Direction::N_DIRECTIONS>>, Face::N_FACES> facemap{{
         /*     FACE            EAST         SOUTH         WEST         NORTH    */
        { Face::TOP,    { Face::RIGHT, Face::FRONT , Face::LEFT , Face::BACK   }},
        { Face::FRONT,  { Face::RIGHT, Face::BOTTOM, Face::LEFT , Face::TOP    }},
        { Face::BOTTOM, { Face::RIGHT, Face::BACK  , Face::LEFT , Face::FRONT  }},
        { Face::BACK,   { Face::RIGHT, Face::TOP   , Face::LEFT , Face::BOTTOM }},
        { Face::LEFT,   { Face::FRONT, Face::BOTTOM, Face::BACK , Face::TOP    }},
        { Face::RIGHT,  { Face::BACK,  Face::BOTTOM, Face::FRONT, Face::TOP    }}
    }};

    /* defines the orientation of each adjacent face.  So in the first row, which is for face top, the face adjacent to our east side will have it's north
     * side adjacent to us.
     */
    static constexpr std::array<std::pair<Face::FaceId, std::array<Direction::DirectionValue, Direction::N_DIRECTIONS>>, Face::N_FACES> orientationmap{{
    /*    face                  EAST             SOUTH              WEST             NORTH    */
        { Face::TOP,    { Direction::NORTH , Direction::NORTH, Direction::NORTH, Direction::SOUTH }},
        { Face::FRONT,  { Direction::WEST  , Direction::NORTH, Direction::EAST , Direction::SOUTH }},
        { Face::BOTTOM, { Direction::SOUTH , Direction::NORTH, Direction::SOUTH, Direction::SOUTH }},
        { Face::BACK,   { Direction::EAST  , Direction::NORTH, Direction::WEST , Direction::SOUTH }},
        { Face::LEFT,   { Direction::WEST  , Direction::WEST , Direction::WEST , Direction::WEST  }},
        { Face::RIGHT,  { Direction::EAST  , Direction::EAST , Direction::EAST , Direction::EAST  }}
    }};

    Face(FaceId _id, int _size, int _x, int _y) : id(_id), size(_size), x(_x), y(_y), rotation(0) {}

    /* The face rotation defines the mapping between virtual and physical.  If rotation is 0, 
     * physical north is virtual north,  If rotation is 1, virtual north is physical east.   rotation
     * defines the number of clockwise 90-degree rotations.
     */
    Direction getVirtualDirection(const Direction& physicaldirection) const
    {
        return physicaldirection - int64_t(rotation);
    }

    Direction getPhysicalDirection(const Direction& virtualdirection) const
    {
        return virtualdirection + int64_t(rotation);
    }

    /* Gets the information of an adjacent face.  The direction is the physical direction.  So if rotation is 0, 
     * specifying North will give you the information about the face that is located above the referenced grid in the 
     * unfolded map, irregardless of the rotation.
     *
     * The tuple contains:
     * - Identity of the face that is expected.
     * - Direction of the opposite face that we expect to connect to us on that side.
     * - Unfolded grid position of the top-left corner of the adjacent edge 
     */
    std::tuple<Face::FaceId, Direction::DirectionValue, std::array<int, 2>> getPhysicalAdjacentInfo(const Direction& physicaldirection) const
    {
        auto virtualdirection = getVirtualDirection(physicaldirection);

        std::tuple<Face::FaceId, Direction::DirectionValue, std::array<int, 2>> ret;

        auto facemap_i = std::find_if(facemap.begin(), facemap.end(), [&](const std::pair<Face::FaceId, std::array<Face::FaceId, Direction::N_DIRECTIONS>>& m) {
            return m.first == id;
        });

        if (facemap_i == facemap.end())
            /* can't happen actually, but just in case, this is not in the fast path anyway. */
            throw std::invalid_argument("Can't determine facemap");

        std::get<0>(ret) = facemap_i->second[virtualdirection];

        auto orientationmap_i = std::find_if(orientationmap.begin(), orientationmap.end(), [&](const std::pair<Face::FaceId, std::array<Direction::DirectionValue, Direction::N_DIRECTIONS>>& m) {
            return m.first == id;
        });

        if (orientationmap_i == orientationmap.end())
            /* can't happen actually, but just in case, this is not in the fast path anyway. */
            throw std::invalid_argument("Can't determine orientation");

        std::get<1>(ret) = orientationmap_i->second[virtualdirection];

        auto& pos = std::get<2>(ret);
        switch(physicaldirection.ev())
        {
        case Direction::NORTH:
            pos[0] = x;
            pos[1] = y - size;
            break;
        case Direction::EAST:
            pos[0] = x + size;
            pos[1] = y;
            break;
        case Direction::SOUTH:
            pos[0] = x;
            pos[1] = y + size;
            break;
        case Direction::WEST:
            pos[0] = x - size;
            pos[1] = y;
            break;
        }

        return ret;
    }

    void rotateRight() 
    {
        rotation = (rotation + 1) % Direction::N_DIRECTIONS;
    }

    FaceId getId() const { return id; }

    void draw(std::map<std::array<int,2>, char>& field, int scale_num=1, int scale_denom=1) const
    {        
        /* Draw a schematic representation of the face in the given field */

        int _sx = x*scale_num/scale_denom;
        int _sy = y*scale_num/scale_denom;
        int _ssize = size*scale_num/scale_denom;

        int x_right = _sx + _ssize-1;
        int y_right = _sy + _ssize-1;

        for (int _x = _sx; _x <= x_right; ++_x)
        {
            char tok = '-';
            if ((_x == _sx) || (_x == x_right))
            {
                tok = '+';
            }

            field[{_x, _sy}] = tok;
            field[{_x, y_right}] = tok;
        }

        for (int _y = _sy; _y <= y_right; ++_y)
        {
            char tok = '|';
            if ((_y == _sy) || (_y == y_right))
            {
                tok = '+';
            }

            field[{_sx, _y}] = tok;
            field[{x_right, _y}] = tok;
        }

        field[{_sx + _ssize/2, _sy + _ssize/2}] = id;

        auto origin = rotateCoords({0,0}, -rotation, _ssize);
        field[{_sx + origin[0], _sy + origin[1]}] = '*';

        std::string x_toks(">v<^");
        std::string y_toks("v<^>");

        auto xp = rotateCoords({3, 0}, -rotation, _ssize);
        field[{_sx + xp[0], _sy + xp[1]}] = 'x';
        xp = rotateCoords({2, 0}, -rotation, _ssize);
        field[{_sx + xp[0], _sy + xp[1]}] = x_toks[rotation%4];

        auto yp = rotateCoords({0, 3}, -rotation, _ssize);
        field[{_sx + yp[0], _sy + yp[1]}] = 'y';
        yp = rotateCoords({0, 2}, -rotation, _ssize);
        field[{_sx + yp[0], _sy + yp[1]}] = y_toks[rotation%4];

        yp = rotateCoords({1, 1}, -rotation, _ssize);
        field[{_sx + yp[0], _sy + yp[1]}] = '0' + rotation;


        switch (getVirtualDirection(Direction::NORTH).ev())
        {
        case Direction::EAST:
            field[{_sx + _ssize/2, _sy + 1}] = 'E';
            break;
        case Direction::SOUTH:
            field[{_sx + _ssize/2, _sy + 1}] = 'S';
            break;
        case Direction::WEST:
            field[{_sx + _ssize/2, _sy + 1}] = 'W';
            break;
        case Direction::NORTH:
            field[{_sx + _ssize/2, _sy + 1}] = 'N';
            break;
        }
    }

    void drawUnfolded(std::map<std::array<int,2>, char>& field) const
    {
        for (int _y = 0; _y < size; ++_y)
        {
            for (int _x = 0; _x < size; ++_x )
            {
                auto pc = xfrmVirtualToPhysical({_x, _y});
                field[pc] = isWall({_x, _y}) ? '#' : '.';
            }
        }
    }

    void drawRunner(std::list<std::array<int,2>>& field, const Runner& r) const
    {
        auto pc = xfrmVirtualToPhysical({r.x, r.y});
        field.push_back(pc);
    }

    int getSize() const { return size; }

    std::array<int, 2> xfrmPhysicalToVirtual(const std::array<int, 2>& coords) const
    {
        std::array<int, 2> ret;
        ret[0] = coords[0] - x;
        ret[1] = coords[1] - y;

        return rotateCoords(ret, rotation);
    }


    std::array<int, 2> xfrmVirtualToPhysical(const std::array<int, 2>& coords) const
    {
        std::array<int, 2> ret = rotateCoords(coords, 4-rotation);

        ret[0] += x;
        ret[1] += y;        
    
        return ret;
    }

    /* uses virtual coordinates */
    void setWall(const std::array<int, 2>& coords)
    {
        m_Walls.insert(coords);
    }

    bool isWall(const std::array<int, 2>& coords) const
    {
        return m_Walls.find(coords) != m_Walls.end();
    }

    bool isWall(const Runner& coords) const
    {
        return m_Walls.find({coords.x, coords.y}) != m_Walls.end();
    }

    /* uses fysical coordinates */
    void setWallP(const std::array<int, 2>& coords)
    {
        auto vc = xfrmPhysicalToVirtual(coords);

        for (auto &c: vc)
        {
            if (c < 0) return;
            if (c >= size) return;
        }

        setWall(vc);
    }

    bool isWallP(const std::array<int, 2>& coords) const
    {
        auto vc = xfrmPhysicalToVirtual(coords);

        return isWall(vc);
    }

    std::pair<const Face*, Runner> step(const Runner& r) const
    {
        std::pair<const Face*, Runner> ret{this, r};

        auto& pos = ret.second;
        auto dir = ret.second.direction;

        switch (dir.ev())
        {
        case Direction::NORTH:
            pos.y -= 1;
            if (pos.y < 0)
            {
                /* Fell off the north side */
                ret.first = m_Neighbours[Direction::NORTH];
                pos.y += size;
            }
            break;
        case Direction::EAST:
            pos.x += 1;
            if (pos.x >= size)
            {
                /* Fell off the east side */
                ret.first = m_Neighbours[Direction::EAST];
                pos.x -= size;
            }
            break;
        case Direction::SOUTH:
            pos.y += 1;
            if (pos.y >= size)
            {
                /* Fell off the south side */
                ret.first = m_Neighbours[Direction::SOUTH];
                pos.y -= size;
            }
            break;
        case Direction::WEST:
            pos.x -= 1;
            if (pos.x < 0)
            {
                 /* Fell off the west side */
                ret.first = m_Neighbours[Direction::WEST];
                pos.x += size;
            }
            break;
        }

        if (ret.first != this)
        {
            /* Align the orientation.  If the opposite face is S and we fell of the N side there's no reorientation needed.  Otherwise
             * perform a rotation until our rotations are matched.  Every rotation also rotates the direction accordingly.
             */
            auto orientationmap_i = std::find_if(orientationmap.begin(), orientationmap.end(), [&](const std::pair<Face::FaceId, std::array<Direction::DirectionValue, Direction::N_DIRECTIONS>>& m) {
                return m.first == id;
            });

            if (orientationmap_i == orientationmap.end())
                /* can't happen actually, but just in case, this is not in the fast path anyway. */
                throw std::invalid_argument("Can't determine orientation");

            auto otherside = Direction(orientationmap_i->second[dir]);
            int delta = (dir+2).steps(otherside);

            auto coords = rotateCoords({pos.x, pos.y}, -delta);

            pos.x = coords[0];
            pos.y = coords[1];
            pos.direction += delta;
        }

        return ret;
    }

    void setNeighbours(const std::map<FaceId, std::shared_ptr<Face>>& map)
    {
        auto facemap_i = std::find_if(facemap.begin(), facemap.end(), [&](const std::pair<Face::FaceId, std::array<Face::FaceId, Direction::N_DIRECTIONS>>& m) {
            return m.first == id;
        });

        if (facemap_i == facemap.end())
            /* can't happen actually, but just in case, this is not in the fast path anyway. */
            throw std::invalid_argument("Can't determine facemap");

        for (auto& d : Direction::all)
        {
            auto ni = map.find(facemap_i->second[d]);
            if (ni == map.end())
                throw std::invalid_argument("Can't determine face mapping");

            m_Neighbours[d] = ni->second.get();
        }
    }

private:

    std::array<int, 2> rotateCoords(const std::array<int, 2>& coords, int steps, int _size) const
    {
        std::array<int, 2> ret(coords);

        steps %= 4;
        if (steps < 0)
            steps += 4;

        for (int i=0; i<steps; ++i)
        {
            int _y = ret[1];
            ret[1] = (_size-1)-ret[0];
            ret[0] = _y;
        }

        return ret;
    }

    std::array<int, 2> rotateCoords(const std::array<int, 2>& coords, int steps) const
    {
        return rotateCoords(coords, steps, size);
    }

    FaceId id;
    int size;
    int x;
    int y;

    /* Rotation defines the direction of the North edge in the unfolded grid.   If rotation is 0, the North face is upward.  If 
     * rotation is 1, North face is eastward (right). 
     */
    int rotation;

    std::set<std::array<int, 2>> m_Walls;
    std::array<const Face*, 4> m_Neighbours;
};

std::ostream& operator<<(std::ostream& os, const Face::FaceId id)
{
    switch (id)
    {
    case Face::BOTTOM:
        os << "bottom";
        break;
    case Face::TOP:
        os << "top";
        break;
    case Face::FRONT:
        os << "front";
        break;
    case Face::BACK:
        os << "back";
        break;
    case Face::LEFT:
        os << "left";
        break;
    case Face::RIGHT:
        os << "right";
        break;
    }

    return os;
};

class Cube
{
public:

    Cube(const Grid& unfolded) 
    {
        /* Determine face size. */
        std::size_t surface_area = 0;
        for (auto &r : unfolded.m_row_limits)
        {
            surface_area += r.length();
        }

        std::size_t face_surface_area = surface_area / Face::N_FACES;

        if ((face_surface_area * Face::N_FACES) != surface_area)
            throw std::invalid_argument("Surface area is not multiple of number of faces");

        size = std::sqrt(face_surface_area);
        if ((size * size) != face_surface_area)
            throw std::invalid_argument("Surface dimension is not square of face surface");

        std::map<std::array<int, 2>, std::shared_ptr<Face>> unresolved_faces;
        std::shared_ptr<Face> top;

        for (int y = 0; y < unfolded.m_row_limits.size(); y += size)
        {
            for (int x = unfolded.m_row_limits[y].min; x < unfolded.m_row_limits[y].max; x += size)
            {
                if (faces.empty())
                {
                    /* First frame we encouter is the top frame */
                    top = std::make_shared<Face>(Face::TOP, size, x, y);
                    faces.emplace(Face::TOP, top);
                }
                else
                {
                    unresolved_faces.emplace(std::array<int,2>({x,y}), std::shared_ptr<Face>());
                }
            }
        }

        std::set<Face::FaceId> unassigned_faces{ Face::BOTTOM, Face::FRONT, Face::BACK, Face::LEFT, Face::RIGHT };
        std::vector<std::shared_ptr<Face>> new_faces; 
        new_faces.push_back(top);

        bool print_dbg = true;

        while (! new_faces.empty())
        {
            std::vector<std::shared_ptr<Face>> newer_faces;

            for (auto &f : new_faces)
            {
                if (f->getId() == Face::LEFT)
                {
                    print_dbg = true;
                }
                
                for (auto &d : Direction::all)
                {
                    auto face_info = f->getPhysicalAdjacentInfo(d);
                    auto& target_pos = std::get<2>(face_info);

                    auto face_i = unresolved_faces.find(target_pos);
                    if (face_i != unresolved_faces.end())
                    {
                        auto nf = std::make_shared<Face>(std::get<0>(face_info), size, target_pos[0],target_pos[1]);

                        /* Match the face's rotation */
                        std::size_t i=0;
                        for (; i < Direction::N_DIRECTIONS; ++i, nf->rotateRight())
                        {
                            if (nf->getVirtualDirection(d+2) == std::get<1>(face_info))
                                break;
                        }

                        if (i == Direction::N_DIRECTIONS)
                            throw std::invalid_argument("Can't determine rotation");

                        newer_faces.push_back(nf);
                        faces.emplace(nf->getId(), nf);

                        unresolved_faces.erase(face_i);
                        unassigned_faces.erase(nf->getId());
                    }
                }
            }

            new_faces = newer_faces;
        }

        if (! unassigned_faces.empty())
            throw std::invalid_argument("Not all faces could be resolved");

        for (auto &f : faces)
        {
            f.second->setNeighbours(faces);
        }

        /* Fill in walls in each face */
        for (auto &w : unfolded.m_walls)
        {
            for (auto &f : faces)
            {
                f.second->setWallP(w);
            }
        }
    }

    void drawUnfolded(std::ostream& os, const std::set<std::array<int,2>>& highligts = std::set<std::array<int,2>>()) const
    {
        std::list<std::array<int,2>> path;
        drawUnfolded(os, path, nullptr, highligts);
    }

    void drawUnfolded(std::ostream& os, std::list<std::array<int,2>> &path, const std::pair<const Face*, Runner>* state = nullptr, const std::set<std::array<int,2>>& highlights = std::set<std::array<int,2>>()) const
    {
        std::map<std::array<int,2>, char> field;
        Limit x_axis;
        Limit y_axis;

        for (auto& f: faces)
        {
            f.second->drawUnfolded(field);
            /*
            if ((state) && (state->first == f.second.get()))
            {
                f.second->drawRunner(path, state->second);
            }
            */
        }

        for (auto& p : field)
        {
            x_axis.update(p.first[0]);
            y_axis.update(p.first[1]);
        }

        for (int y = y_axis.min; y <= y_axis.max; ++y)
        {
            std::string line(x_axis.length(), ' ');

            for (int x = x_axis.min; x<= x_axis.max; ++x)
            {
                auto c = field.find({x, y});
                if (c != field.end())
                    line[x - x_axis.min] = c->second;
                
                auto p = std::find_if(path.begin(), path.end(), [&](const std::array<int, 2>& p) { 
                    return (p[0] == x) && (p[1] == y);
                });

                if (p != path.end())
                {
                    auto on = std::next(p);
                    if (on == path.end())
                    {
                        line[x - x_axis.min] = 'X';
                    }
                    else
                    {
                        line[x - x_axis.min] = '*';
                    }
                }
            }

            while ((! line.empty()) && (*line.rbegin() == ' '))
            {
                line.pop_back();
            }

            /* Check for highlights */
            std::string line2;
            for (int x = x_axis.min; x<= x_axis.max; ++x)
            {
                if (highlights.find({x, y}) != highlights.end())
                {
                    line2 += "\033[1;31m";
                    if (line[x] == ' ')
                    {
                        line2 += 'O';
                    }
                    else
                    {
                        line2 += line[x];
                    }

                    line2 += "\033[0m";
                }
                else
                {
                    line2 += line[x];                    
                }
            }

            os << line2 << std::endl;
        }
    }


    void draw(std::ostream& os) const
    {
        std::map<std::array<int,2>, char> field;
        Limit x_axis;
        Limit y_axis;

        int scale_num = 1;
        int scale_denom = 1;

        if (faces.begin()->second->getSize() < 10) 
        {
            scale_num=3;
        }
        else if (faces.begin()->second->getSize() > 10)
        {
            scale_denom=5;
        } 

        for (auto& f: faces)
        {
            f.second->draw(field, scale_num, scale_denom);
        }

        for (auto& p : field)
        {
            x_axis.update(p.first[0]);
            y_axis.update(p.first[1]);
        }

        for (int y = y_axis.min; y <= y_axis.max; ++y)
        {
            std::string line(x_axis.length(), ' ');

            for (int x = x_axis.min; x<= x_axis.max; ++x)
            {
                auto c = field.find({x, y});
                if (c != field.end())
                    line[x - x_axis.min] = c->second;
            }

            os << line << std::endl;
        }
    }

    void printstate(const std::pair<const Face*, Runner>& s) const
    {
        return ;

        auto ppos = s.first->xfrmVirtualToPhysical({s.second.x, s.second.y});
        auto dir = s.first->getPhysicalDirection(s.second.direction);

        std::cout << "x=" << ppos[0] << ",y=" << ppos[1] << ",d=" << (1+dir.nv())%4 << std::endl;
    }

    Runner run(std::shared_ptr<InstructionList> instructions, bool debug = false) const
    {
        std::pair<const Face*, Runner> state{getFace(Face::TOP), Runner()};
        std::list<std::array<int, 2>> path;

        if (debug)
        {
            drawUnfolded(std::cout, path, &state);
            std::cout << std::endl;
        }

        printstate(state);

        for (auto &i : *instructions)
        {

            if (i.dir == 'R')
            {
                state.second.direction++;
            }
            else if (i.dir == 'L')
            {
                state.second.direction--;
            }
            else 
            {
                for (int j=0; j<i.steps; ++j)
                {
                    auto next_state = state.first->step(state.second);

                    if (next_state.first->isWall(next_state.second))
                        break;

                    /* Draw state */
                    next_state.first->drawRunner(path, next_state.second);

                    if (debug)
                    {
                        drawUnfolded(std::cout, path, &next_state);
                        std::cout << std::endl;
                        getchar();
                    }

                    state = next_state;
                }
            }
        }

        /* We are done, now transform the runner state to it's physical form */
        auto ppos = state.first->xfrmVirtualToPhysical({state.second.x, state.second.y});

        state.second.x = ppos[0];
        state.second.y = ppos[1];
        state.second.direction = state.first->getPhysicalDirection(state.second.direction);

        /* TODO */
        return state.second;
    }

    const Face* getFace(Face::FaceId id) const
    {
        auto i = faces.find(id);
        if (i == faces.end())
            return nullptr;

        return i->second.get();
    }

private:


    std::size_t size;
    std::map<Face::FaceId, std::shared_ptr<Face>> faces;
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
    std::shared_ptr<InstructionList> instructions;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            if (line.empty())
                continue;

            if (line.find_first_of(".#") != std::string::npos)
            {
                grid.addRow(line);
            }
            else
            {
                instructions = std::make_shared<InstructionList>(line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    if (! instructions)
    {
        std::cerr << "Could not parse instructions" << std::endl;
        std::exit(-1);
    }

    auto state = grid.run(instructions);

    std::cout << "State x=" << state.x << ", y=" << state.y << ", dir=" << state.direction << std::endl;
    std::cout << "Password=" << state.toPassword() << std::endl << std::endl << std::endl << std::endl;

    Cube cube(grid);

    // cube.drawUnfolded(std::cout);

    auto state2 = cube.run(instructions, false);
    std::cout << "State x=" << state2.x << ", y=" << state2.y << ", dir=" << state2.direction << std::endl;
    std::cout << "Password=" << state2.toPassword() << std::endl << std::endl << std::endl << std::endl;

    return 0;
}
