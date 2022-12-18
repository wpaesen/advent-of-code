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

template<std::size_t N, typename T>
std::array<T, N> operator+(const std::array<T, N>& lhs, const std::array<T, N>& rhs)
{
    std::array<T, N> ret;

    for (std::size_t i=0; i<ret.size(); ++i)
    {
        ret[i] = lhs[i] + rhs[i];
    }

    return ret;
}

template<std::size_t N, typename T>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& rhs)
{
    char sep ='(';

    for (auto& c : rhs)
    {
        os << sep << c;
        sep = ',';
    }
    os << ")";

    return os;
}

template<std::size_t N, typename T>
class Extents
{
public:

    Extents() 
    {
        pmin.fill( std::numeric_limits<T>::max() );
        pmax.fill( std::numeric_limits<T>::min() );
    }

    Extents(const Extents& other) : pmin(other.pmin), pmax(other.pmax) 
    {}

    void update( const std::array<T, N>& pos )
    {
        for (std::size_t i=0; i<pos.size(); ++i)
        {
            if (pos[i] < pmin[i])
                pmin[i] = pos[i];
            if (pos[i] > pmax[i])
                pmax[i] = pos[i];
        }
    }

    void blowup( T i )
    {
        for (auto& p : pmin)
        {
            p -= i;
        }

        for (auto& p : pmax)
        {
            p += i;
        }
    }

    bool shrink( T i )
    {
        bool shrunk = false;
        for (std::size_t dim = 0; dim < pmin.size(); ++dim)
        {
            auto npmin = pmin[dim] + i;
            auto npmax = pmax[dim] - i;

            if (npmin <= npmax)
            {
                pmin[dim] = npmin;
                pmax[dim] = npmax;
                shrunk = true;
            }
            else if (npmin <= pmax[dim])
            {
                pmin[dim] = npmin;                
                shrunk = true;
            }
        }

        return shrunk;
    }

    const std::array<T, N>& getMin() const { return pmin; }
    const std::array<T, N>& getMax() const { return pmax; }

    bool contain( const std::array<T, N>& pos )
    {
        for (std::size_t i = 0; i < pos; ++i)
        {
            if (pos[i] < pmin) return false;
            if (pos[i] > pmax) return false;
        }
        return true;
    }

    struct iterator {
        iterator() : face(2*N), extents(nullptr) { pos.fill(0); };
        iterator(const Extents& a_extents) : face(0), extents(& a_extents), pos(a_extents.getMin()) { visited.emplace(pos); };

        std::size_t face;
        std::array<T, N> pos;
        const Extents* extents;

        std::set<std::array<T, N>> visited;

        bool next()
        {
            if ((face < 2*N) && (extents))
            {
                std::size_t fixed_dim = face / 2;
                for (std::size_t dim = 0; dim < N; ++dim)
                {
                    if (dim != fixed_dim)
                    {
                        if (pos[dim] < extents->getMax()[dim])
                        {
                            pos[dim]++;
                            return true;
                        }

                        pos[dim] = extents->getMin()[dim];
                    }
                }

                /* If we end up here, this face is fully iterated.  So we go to the next face */
                face++;

                if (face < 2*N)
                {
                    pos = extents->getMin();

                    fixed_dim = face / 2;
                    if (face & 0x01)
                    {
                        pos[fixed_dim] = extents->getMax()[fixed_dim];
                    }
                }

                return true;
            }

            return false;
        }

        iterator& operator++()
        {
            while (next())
            {
                auto v = visited.emplace(pos);
                if (v.second == true)
                    return *this;
            }

            return *this;
        }

        bool operator==(const iterator& rhs ) const
        {
            if (face != rhs.face) return false;

            if (face < (2*N))
            {
                for (std::size_t i=0; i<N; ++i)
                {
                    if (pos[i] != rhs.pos[i]) return false;
                }
            }

            return true;
        }

        bool operator!=(const iterator& rhs ) const
        {
            return ! operator==(rhs);
        }

        const std::array<T, N>& operator*() const
        {
            return pos;
        };
    };

    /* iterates all the positions on the outer shell of the extents */
    iterator begin() const
    {
        return iterator(*this);
    }

    const iterator& end() const
    {
        return iterend;
    }

private:

    iterator iterend;
    std::array<T, N> pmin;
    std::array<T, N> pmax;
};

template<std::size_t N, typename T>
class Sides
{
public:
    Sides()
    {
        for (auto &side : sides)
        {
            side.fill(0);
        };

        for (std::size_t dim=0; dim < N; ++dim)
        {
            sides[(dim*2)][dim] = 1;
            sides[(dim*2)+1][dim] = -1;
        }
    }

    operator const std::array<std::array<T, N>, 2*N>&() const
    {
        return sides;
    };

    typename std::array<std::array<T, N>, 2*N>::const_iterator begin() const
    {   
        return sides.begin();
    }

    const typename std::array<std::array<T, N>, 2*N>::const_iterator end() const
    {
        return sides.end();
    }

    const std::array<T, N>& operator[](std::size_t i) const
    {
        return sides[i];
    }

    std::size_t size() const
    {
        return sides.size();
    }

    static const Sides& instance()
    {
        static Sides inst;
        return inst;
    }

private:

    std::array<std::array<T, N>, 2*N> sides;
};

template<std::size_t N, typename T>
class Cube
{
public:
    enum Kind 
    {
        SOLID,
        REACHABLE_AIR,
        AIR
    };

    Cube(const std::array<T, N>& a_pos, Kind a_kind = AIR) : pos(a_pos), kind(a_kind) {
        covered.fill(false);
    }

    typename std::array<std::array<T, N>, 2*N> getNeighbours() const
    {
        typename std::array<std::array<T, N>, 2*N> ret;

        auto sides = Sides<N, T>::instance();
        for (std::size_t i=0; i<sides.size(); ++i)
        {
            ret[i] = pos + sides[i];
        }

        return ret;
    }

    typename std::vector<std::array<T, N>> getNeighbours(const Extents<N,T>& extents) const
    {
        typename std::vector<std::array<T, N>> ret;

        auto sides = Sides<N, T>::instance();
        for (std::size_t i=0; i<sides.size(); ++i)
        {
            auto p = pos + sides[i];
            if (extents.contains(p))
            {
                ret.emplace_back(p);
            }
        }

        return ret;
    }

    bool& getSideState(std::size_t i)
    {
        return covered[i];
    }

    bool getSideState(std::size_t i) const
    {
        return covered[i];
    }

    static std::size_t oppositeSide(std::size_t i)
    {
        return i ^ 1;
    };

    std::size_t getUncoveredSides() const
    {
        std::size_t ret = 0;
        for (auto&i : covered)
        {
            if (! i) ret++;
        }
        return ret;
    }

    bool isSolid() const { return kind == SOLID; }
    bool isReachableAir() const { return kind == REACHABLE_AIR; }
    bool isPocket() const { return kind == AIR; }

    void setReachableAir() 
    {
        if (kind == AIR)
            kind = REACHABLE_AIR;
    }

    Kind getKind() const { return kind; }

private:

    Kind kind;
    std::array<T, N> pos;
    std::array<bool, 2*N> covered;
};


template<std::size_t N, typename T>
class Grid
{
public:
    Grid() {};

    void addCube(const std::array<T, N>& coords) 
    {
        cubes.emplace(coords, Cube<N,T>(coords, Cube<N,T>::SOLID));
        extents.update(coords);
    }

    bool isOccupied(const std::array<T, N>& pos) const
    {
        return cubes.find(pos) != cubes.end();
    }

    Cube<N, T>* lookup(const std::array<T, N>& pos)
    {
        auto i = cubes.find(pos);
        if (i == cubes.end())
        {            
            return nullptr;
        }

        return & i->second;
    }

    std::size_t calculateCovered()
    {
        for (auto& c : cubes)
        {
            if (! c.second.isSolid())
                continue;

            extents.update(c.first);
            auto s = c.second.getNeighbours();

            for (std::size_t i=0; i<s.size(); ++i)
            {
                if (! c.second.getSideState(i))
                {
                    auto n = lookup(s[i]);
                    if ((n) && (n->isSolid()))
                    {
                        c.second.getSideState(i) = true;
                        n->getSideState(n->oppositeSide(i)) = true;                        
                    }
                }
            }
        }

        std::size_t ret = 0;
        for (auto& c : cubes)
        {
            ret += c.second.getUncoveredSides();
        }

        return ret;
    }

    std::size_t calculatePocketed()
    {
        /* Pour air into the system. Force air by sweeping a wall of air on every side to the inside. */
        Extents<N, T> shell(extents);
        shell.blowup(1);

        for (auto& p : shell)
        {
            cubes.emplace(p, Cube<N, T>(p, Cube<N, T>::REACHABLE_AIR ));
        }

        while (shell.shrink(1))
        {
            for (auto& p : shell)
            {
                auto c = lookup(p);
                if (! c)
                {
                    auto c2 = cubes.emplace(p, Cube<N, T>(p, Cube<N, T>::AIR));
                    auto s = c2.first->second.getNeighbours();
                    for (std::size_t i=0; i<s.size(); ++i)
                    {
                        auto n = lookup(s[i]);
                        if ((n) && (n->isReachableAir()))
                        {
                            c2.first->second.setReachableAir();
                            break;
                        }
                    }
                }
            }
        }

        /* Find all air pockets */
        std::list<Cube<N,T>*> pockets;
        for (auto& c : cubes)
        {
            if (c.second.isPocket())
            {
                pockets.push_back(& c.second);
            }
        }

        /* Check if after first sweep of the cube if some air pockets have become reachable. */
        bool changed = false;
        do 
        {
            changed = false;
            for (auto c = pockets.begin(); c != pockets.end(); )
            {
                bool remove = false;
                auto s = (*c)->getNeighbours();
                for (std::size_t i=0; i<s.size(); ++i)
                {
                    auto n = lookup(s[i]);
                    if ((n) && (n->isReachableAir()))
                    {
                        changed = true;
                        remove = true;
                        (*c)->setReachableAir();
                        break;
                    }
                }

                if (remove)
                {
                    c = pockets.erase(c);
                }
                else
                {
                    c = std::next(c);
                }
            }
        } while (changed);

        std::size_t ret = 0;
        for (auto& c : cubes)
        {
            if (c.second.isSolid())
            {
                auto s = c.second.getNeighbours();
                for (std::size_t i=0; i<s.size(); ++i)
                {
                    auto n = lookup(s[i]);
                    if ((n) && (n->isReachableAir()))
                    {
                        ret++;
                    }
                }
            }
        }

        return ret;
    }

private:

    std::map<std::array<T, N>,Cube<N,T>> cubes;
    Extents<N, T> extents;
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    Grid<3, int> grid;
    try
    {
        std::ifstream infile(argv[1]);

        std::regex matchrule{"^\\s*([0-9]+),([0-9]+),([0-9]+)\\s*$"};

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule))
            {
                grid.addCube({std::stoi(match[1].str()),std::stoi(match[2].str()),std::stoi(match[3].str())});
            }
            else
            {
                throw std::invalid_argument("Syntax error");
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    std::cout << grid.calculateCovered() << " Uncovered cube sides" << std::endl;
    std::cout << grid.calculatePocketed() << " Uncovered cube exterior surface" << std::endl;

    return 0;
}
