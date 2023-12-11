#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
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
#include <optional>
#include <string_view>

static bool with_debug = true;
using namespace std::string_view_literals;

enum Direction {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3,
};

enum Corner {
    NE = 0,
    SE = 1,
    SW = 2,
    NW = 3,
};

/* clang-format off */
static constexpr std::array<std::pair<char, std::string_view>, 6> s_graphics{{
    {'-', "─"sv}, {'|', "│"sv}, {'L', "└"sv}, {'J', "┘"sv}, {'7', "┐"sv}, {'F', "┌"}
}};

static constexpr std::array<std::pair<std::bitset<4>, char>, 6> s_connections{{
    {(1 << NORTH) | (1 << SOUTH), '|'},
    {(1 << EAST)  | (1 << WEST) , '-'},
    {(1 << EAST)  | (1 << NORTH), 'L'},
    {(1 << WEST)  | (1 << NORTH), 'J'},
    {(1 << SOUTH) | (1 << EAST) , 'F'},
    {(1 << SOUTH) | (1 << WEST) , '7'}
}};
/* clang-format on*/

class Grid
{
public:
    using coord_t = std::pair<int, int>;
    using distance_t = int64_t;

    static constexpr std::array<Direction, 4> directions{{NORTH, EAST, SOUTH, WEST}};
    static constexpr std::array<Corner, 4> corners{{NE, SE, SW, NW}};

    Grid() : m_Max(-1, -1), m_Start(-1, -1)
    {
    }

    void addLine(const std::string& s)
    {
        if (m_Max.first == -1) {
            m_Max.first = s.size() - 1;
            m_Max.second = 0;
        } else {
            if (m_Max.first != (s.size() - 1))
                throw std::invalid_argument("Irregular grid size");

            m_Max.second += 1;
        }

        coord_t p = m_Max;
        for (p.first = 0; p.first < s.size(); ++p.first) {
            auto c = s[p.first];
            if (c != '.')
                m_Map.emplace(p, std::make_unique<Node>(s[p.first], p));
            if (c == 'S') {
                m_Start = p;
            }
        }
    }

    std::ostream& write(std::ostream& os) const
    {
        coord_t p;
        for (p.second = 0; p.second <= m_Max.second; ++p.second) {
            for (p.first = 0; p.first <= m_Max.first; ++p.first) {
                auto elm = m_Map.find(p);
                if (elm == m_Map.end()) {
                    std::cout << ".";
                } else {
                    std::string out{elm->second->type};
                    for (auto& g : s_graphics) {
                        if (g.first == elm->second->type) {
                            out = g.second;
                        }
                    }

                    std::cout << "\033[48;5;57m" << out << "\033[m";
                }
            }
            std::cout << std::endl;
        }

        return os;
    }

    void finalize()
    {
        if (m_Map.empty())
            throw std::invalid_argument("Can't work on empty grid");

        for (auto& p : m_Map) {
            for (auto& con : p.second->connections()) {
                auto elm = m_Map.find(con.second);
                if (elm != m_Map.end()) {
                    p.second->adjacent[con.first] = elm->second.get();
                }
            }
        }
    }

    void reset()
    {
        for (auto& p : m_Map) {
            p.second->distance = std::numeric_limits<distance_t>::max();
        }
    }

    void walk()
    {
        std::vector<Node*> steps;

        auto start_elm = m_Map.find(m_Start);
        if (start_elm == m_Map.end())
            throw std::invalid_argument("Start node not found");

        Node* start = start_elm->second.get();

        for (auto& p : directions) {
            auto neigh = move_to(start_elm->first, p);
            auto elm = m_Map.find(neigh);
            if (elm != m_Map.end()) {
                auto adj = elm->second->get_adjacent(opposite(p));
                if (adj == start) {
                    start->adjacent[p] = elm->second.get();
                    steps.emplace_back(elm->second.get());
                }
            }
        }

        if (steps.empty())
            throw std::invalid_argument("No start path");

        start->distance = 0;
        int64_t distance = 1;

        while (!steps.empty()) {
            for (auto& s : steps) {
                s->distance = distance;
            }

            distance++;

            for (auto& s : steps) {
                s = s->unvisited_adjacent();
            }

            steps.erase(std::remove_if(steps.begin(), steps.end(), [](const Node* s) { return s == nullptr; }), steps.end());
        }
    }

    void cleanup()
    {
        /* Remove all nodes that are not visited when walking the mainloop */
        for (auto i = m_Map.begin(); i != m_Map.end();) {
            if (i->second->distance == std::numeric_limits<distance_t>::min()) {
                i = m_Map.erase(i);
            } else {
                i = std::next(i);
            }
        }

        /* Remove all nodes that dont have 2 adjacent nodes.  Do this until no removes are done */
        bool changed = true;
        while (changed) {
            changed = false;

            for (auto i = m_Map.begin(); i != m_Map.end();) {
                if (i->second->n_adjacent() == 2) {
                    i = std::next(i);
                } else {
                    changed = true;
                    for (auto& n : i->second->adjacent) {
                        n.second->remove_adjacent(i->second.get());
                    }
                    i = m_Map.erase(i);
                }
            }
        }

        /* Mutate the start node into it's corresponding symbol */
        auto start_node = m_Map.find(m_Start);
        if (start_node == m_Map.end())
            throw std::invalid_argument("Threw away start node");

        start_node->second->set_symbol();
    }

    void scan_inside()
    {
        /* Find a reference node. That's a node where we are sure we know what inside and outside is.  Do this by
         * scanning the perimeter.  If not found on outer perimeter, reduce perimeter.
         */

        coord_t corner_ul{0, 0};
        coord_t corner_lr{m_Max};

        Node* reference = nullptr;

        while ((reference == nullptr) && (corner_ul.first < corner_lr.first) && (corner_ul.second < corner_lr.second)) {
            for (int x = corner_ul.first; (x <= corner_lr.first) && (reference == nullptr); ++x) {
                coord_t p{x, corner_ul.second};
                auto elm = m_Map.find(p);
                if (elm != m_Map.end()) {
                    /* Got one, coming from north */
                    reference = elm->second.get();
                    reference->set_outside(NORTH);
                }
            }

            for (int x = corner_ul.first; (x <= corner_lr.first) && (reference == nullptr); ++x) {
                coord_t p{x, corner_lr.second};
                auto elm = m_Map.find(p);
                if (elm != m_Map.end()) {
                    /* Got one, coming from south */
                    reference = elm->second.get();
                    reference->set_outside(SOUTH);
                }
            }

            for (int y = corner_ul.second; (y <= corner_lr.second) && (reference == nullptr); ++y) {
                coord_t p{corner_ul.first, y};
                auto elm = m_Map.find(p);
                if (elm != m_Map.end()) {
                    /* Got one, coming from west */
                    reference = elm->second.get();
                    reference->set_outside(WEST);
                }
            };

            for (int y = corner_ul.second; (y <= corner_lr.second) && (reference == nullptr); ++y) {
                coord_t p{corner_lr.first, y};
                auto elm = m_Map.find(p);
                if (elm != m_Map.end()) {
                    /* Got one, coming from west */
                    reference = elm->second.get();
                    reference->set_outside(EAST);
                }
            };

            corner_ul.first++;
            corner_ul.second++;

            corner_lr.first--;
            corner_lr.second--;
        }

        if (!reference)
            throw std::invalid_argument("No reference node found");

        Node* prev = nullptr;
        Node* iter = reference;
        while (true) {
            /* Add O and I to the grid */
            for (auto& d : directions) {
                auto pos = move_to(iter->pos, d);
                auto elm = m_Map.find(pos);
                if (elm == m_Map.end()) {
                    if (iter->is_outside(d)) {
                        m_Map.emplace(pos, std::make_unique<Node>('O', pos));
                    } else {
                        m_Map.emplace(pos, std::make_unique<Node>('I', pos));
                    }
                }
            }

            auto next = iter->next(prev);

            if (next.second == reference)
                break;

            for (auto& c : iter->get_outside_adjacent(next.first)) {
                next.second->set_outside(c);
            }

            prev = iter;
            iter = next.second;
        }

        /* For nodes that are marked as I, check if they have non-marked neighbours */
        bool mutation = true;
        while (mutation) {
            mutation = false;

            std::vector<coord_t> internal_nodes;

            for (auto& n : m_Map) {
                if (n.second->type == 'I') {
                    internal_nodes.emplace_back(n.first);
                }
            }

            for (auto& n : internal_nodes) {
                for (auto& d : directions) {
                    auto pos = move_to(n, d);
                    auto elm = m_Map.find(pos);
                    if (elm == m_Map.end()) {
                        m_Map.emplace(pos, std::make_unique<Node>('I', pos));
                        mutation = true;
                    }
                }
            }
        }
    }

    distance_t getMaxDistance() const
    {
        auto i = std::max_element(m_Map.begin(), m_Map.end(), [](const decltype(m_Map)::value_type& a, const decltype(m_Map)::value_type& b) {
            return a.second->distance < b.second->distance;
        });

        return i->second->distance;
    }

    std::size_t getInternalNodes() const
    {
        std::size_t ret = 0;

        for (auto& n : m_Map) {
            if (n.second->type == 'I') {
                ret++;
            }
        }
        return ret;
    }

private:
    static coord_t move_to(const coord_t& p, const Direction& d)
    {
        switch (d) {
            case NORTH:
                return {p.first, p.second - 1};
            case EAST:
                return {p.first + 1, p.second};
            case SOUTH:
                return {p.first, p.second + 1};
            case WEST:
                return {p.first - 1, p.second};
        }

        return p;
    }

    static Direction opposite(const Direction& d)
    {
        switch (d) {
            case NORTH:
                return SOUTH;
            case EAST:
                return WEST;
            case SOUTH:
                return NORTH;
            case WEST:
                return EAST;
        }

        return d;
    }

    struct Node {
        Node(char _type, coord_t a_pos) : type(_type), pos(a_pos), distance(std::numeric_limits<distance_t>::min()){};

        coord_t pos;
        char type;
        distance_t distance;

        std::unordered_map<Direction, Node*> adjacent;

        std::pair<Direction, Node*> next(const Node* previous)
        {
            /* Jump to first node which is not the previous node */
            for (auto& a : adjacent) {
                if (a.second != previous) {
                    return a;
                }
            }

            return {NORTH, nullptr};
        }

        std::set<Corner> outside;

        std::size_t n_adjacent() const
        {
            return adjacent.size();
        }

        std::set<Direction> adjacent_edges() const
        {
            std::set<Direction> ret;

            for (auto& a : adjacent) {
                ret.emplace(a.first);
            }

            return ret;
        }

        void set_symbol()
        {
            if (type == 'S') {
                std::bitset<4> key;
                key.reset();

                for (auto& d : directions) {
                    if (adjacent.find(d) != adjacent.end()) {
                        key.set((int)d);
                    }
                }

                for (auto &c : s_connections)
                {
                    if (key == c.first) {
                        type = c.second;
                        return;
                    }
                }

                if (type == 'S'){
                    throw std::invalid_argument("Can't determine start symbol " + key.to_string());
                }
            }
        }

        std::ostream& write(std::ostream& os) const
        {
            os << "Node at " << pos.first << ", " << pos.second << ")" << std::endl;

            os << (is_outside(NW) ? 'O' : 'I') << " " << (is_outside(NE) ? 'O' : 'I') << std::endl;
            os << " " << graph() << " " << std::endl;
            os << (is_outside(SW) ? 'O' : 'I') << " " << (is_outside(SE) ? 'O' : 'I') << std::endl;

            return os;
        };

        void remove_adjacent(const Node* a)
        {
            for (auto i = adjacent.begin(); i != adjacent.end();) {
                if (i->second == a) {
                    i = adjacent.erase(i);
                } else {
                    i = std::next(i);
                }
            }
        }

        Node* get_adjacent(const Direction& dir) const
        {
            auto elm = adjacent.find(dir);
            if (elm != adjacent.end()) {
                return elm->second;
            } else {
                return nullptr;
            }
        }

        Node* unvisited_adjacent() const
        {
            for (auto& i : adjacent) {
                if (i.second->distance == std::numeric_limits<distance_t>::min()) {
                    return i.second;
                }
            }

            return nullptr;
        }

        std::string graph() const
        {
            switch (type) {
                case '-':
                    return "─";
                case '|':
                    return "│";
                case 'L':
                    return "└";
                case 'J':
                    return "┘";
                case '7':
                    return "┐";
                case 'F':
                    return "┌";
                default:
                    break;
            }

            return std::string(type, 1);
        }

        /* Return outside corners for node adjacent to dir */
        std::set<Corner> get_outside_adjacent(const Direction& dir) const
        {
            std::set<Corner> ret;

            switch (dir) {
                case NORTH:
                    if (outside.find(NE) != outside.end()) {
                        ret.emplace(SE);
                    }
                    if (outside.find(NW) != outside.end()) {
                        ret.emplace(SW);
                    }
                    break;
                case EAST:
                    if (outside.find(NE) != outside.end()) {
                        ret.emplace(NW);
                    }
                    if (outside.find(SE) != outside.end()) {
                        ret.emplace(SW);
                    }
                    break;
                case SOUTH:
                    if (outside.find(SE) != outside.end()) {
                        ret.emplace(NE);
                    }
                    if (outside.find(SW) != outside.end()) {
                        ret.emplace(NW);
                    }
                    break;
                case WEST:
                    if (outside.find(SW) != outside.end()) {
                        ret.emplace(SE);
                    }
                    if (outside.find(NW) != outside.end()) {
                        ret.emplace(NE);
                    }
                    break;
            }

            return ret;
        }

        bool is_outside(const Corner& c) const
        {
            return outside.find(c) != outside.end();
        }

        bool is_outside(const Direction& dir) const
        {
            switch (dir) {
                case NORTH:
                    if (outside.find(NE) == outside.end())
                        return false;
                    if (outside.find(NW) == outside.end())
                        return false;

                    return true;
                case EAST:
                    if (outside.find(NE) == outside.end())
                        return false;
                    if (outside.find(SE) == outside.end())
                        return false;

                    return true;
                case SOUTH:
                    if (outside.find(SE) == outside.end())
                        return false;
                    if (outside.find(SW) == outside.end())
                        return false;

                    return true;
                case WEST:
                    if (outside.find(NW) == outside.end())
                        return false;
                    if (outside.find(SW) == outside.end())
                        return false;

                    return true;
            }

            return true;
        }

        void set_outside(const Corner& c)
        {
            switch (type) {
                case '|':
                    switch (c) {
                        case NE:
                        case SE:
                            outside.emplace(NE);
                            outside.emplace(SE);
                            break;
                        case SW:
                        case NW:
                            outside.emplace(NW);
                            outside.emplace(SW);
                            break;
                    }
                    break;
                case '-':
                    switch (c) {
                        case NE:
                        case NW:
                            outside.emplace(NE);
                            outside.emplace(NW);
                            break;
                        case SE:
                        case SW:
                            outside.emplace(SE);
                            outside.emplace(SW);
                            break;
                    }
                    break;
                case 'L':
                    switch (c) {
                        case NE:
                            outside.emplace(NE);
                            break;
                        case SE:
                        case SW:
                        case NW:
                            outside.emplace(NW);
                            outside.emplace(SW);
                            outside.emplace(SE);
                            break;
                    }
                    break;
                case 'J':
                    switch (c) {
                        case NW:
                            outside.emplace(NW);
                            break;
                        case NE:
                        case SE:
                        case SW:
                            outside.emplace(NE);
                            outside.emplace(SE);
                            outside.emplace(SW);
                            break;
                    }
                    break;
                case '7':
                    switch (c) {
                        case SW:
                            outside.emplace(SW);
                            break;
                        case NE:
                        case SE:
                        case NW:
                            outside.emplace(NE);
                            outside.emplace(SE);
                            outside.emplace(NW);
                            break;
                    }
                    break;
                case 'F':
                    switch (c) {
                        case SE:
                            outside.emplace(SE);
                            break;
                        case NE:
                        case SW:
                        case NW:
                            outside.emplace(NE);
                            outside.emplace(SW);
                            outside.emplace(NW);
                            break;
                    }
                    break;
                default:
                    break;
            }
        }

        void set_outside(const Direction& dir)
        {
            switch (type) {
                case '|':
                    switch (dir) {
                        case NORTH:
                        case SOUTH:
                            throw std::invalid_argument("Can't set direction");
                            break;
                        case EAST:
                            outside.emplace(NE);
                            outside.emplace(SE);
                            break;
                        case WEST:
                            outside.emplace(NW);
                            outside.emplace(SW);
                            break;
                    }
                    break;
                case '-':
                    switch (dir) {
                        case NORTH:
                            outside.emplace(NE);
                            outside.emplace(NW);
                            break;
                        case SOUTH:
                            outside.emplace(SE);
                            outside.emplace(SW);
                            break;
                        case EAST:
                        case WEST:
                            throw std::invalid_argument("Can't set direction");
                            break;
                    }
                    break;
                case 'L':
                    switch (dir) {
                        case NORTH:
                        case EAST:
                            throw std::invalid_argument("Can't set direction");
                            break;
                        default:
                            outside.emplace(SE);
                            outside.emplace(SW);
                            outside.emplace(NW);
                            break;
                    }
                    break;
                case 'J':
                    switch (dir) {
                        case NORTH:
                        case WEST:
                            throw std::invalid_argument("Can't set direction");
                            break;
                        default:
                            outside.emplace(NE);
                            outside.emplace(SE);
                            outside.emplace(SW);
                            break;
                    }
                    break;
                case '7':
                    switch (dir) {
                        case SOUTH:
                        case WEST:
                            throw std::invalid_argument("Can't set direction");
                            break;
                        default:
                            outside.emplace(NE);
                            outside.emplace(SE);
                            outside.emplace(NW);
                            break;
                    }
                    break;
                case 'F':
                    switch (dir) {
                        case SOUTH:
                        case EAST:
                            throw std::invalid_argument("Can't set direction");
                            break;
                        default:
                            outside.emplace(NE);
                            outside.emplace(SW);
                            outside.emplace(NW);
                            break;
                    }
                    break;
                default:
                    break;
            }
        }

        std::vector<std::pair<Direction, coord_t>> connections() const
        {
            std::vector<std::pair<Direction, coord_t>> ret;

            switch (type) {
                case '|':
                    ret.emplace_back(NORTH, move_to(pos, NORTH));
                    ret.emplace_back(SOUTH, move_to(pos, SOUTH));
                    break;
                case '-':
                    ret.emplace_back(EAST, move_to(pos, EAST));
                    ret.emplace_back(WEST, move_to(pos, WEST));
                    break;
                case 'L':
                    ret.emplace_back(NORTH, move_to(pos, NORTH));
                    ret.emplace_back(EAST, move_to(pos, EAST));
                    break;
                case 'J':
                    ret.emplace_back(NORTH, move_to(pos, NORTH));
                    ret.emplace_back(WEST, move_to(pos, WEST));
                    break;
                case '7':
                    ret.emplace_back(SOUTH, move_to(pos, SOUTH));
                    ret.emplace_back(WEST, move_to(pos, WEST));
                    break;
                case 'F':
                    ret.emplace_back(SOUTH, move_to(pos, SOUTH));
                    ret.emplace_back(EAST, move_to(pos, EAST));
                    break;
                default:
                    break;
            }

            return ret;
        }
    };

    std::map<coord_t, std::unique_ptr<Node>> m_Map;

    coord_t m_Max;
    coord_t m_Start;
};

namespace std
{
    string to_string(const Grid& node)
    {
        std::ostringstream buf;
        node.write(buf);
        return buf.str();
    }

    ostream& operator<<(ostream& os, const Grid& node)
    {
        return node.write(os);
    }

} // namespace std

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    Grid grid;

    try {
        with_debug = true || !(std::string(argv[1]) == "input.dat");

        std::ifstream infile(argv[1]);

        std::string line;

        while (std::getline(infile, line)) {
            if (!line.empty()) {
                grid.addLine(line);
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    grid.finalize();
    grid.walk();
    grid.cleanup();
    grid.scan_inside();

    if (with_debug) {
        std::cout << grid << std::endl;
    }

    std::cout << "A max distance " << grid.getMaxDistance() << std::endl;
    std::cout << "B internal nodes " << grid.getInternalNodes() << std::endl;

    return 0;
}
