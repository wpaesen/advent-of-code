#include <array>
#include <bits/c++config.h>
#include <cerrno>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <ostream>
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

constexpr std::array<char, 3> axes{ 'X', 'Y', 'Z'};

template<size_t N>
class Volume
{
public:

    Volume() {};

    Volume(const Volume& other)
       : m_extents(other.m_extents)
       , m_size(other.m_size)
       , cuts(other.cuts)
    {
    }

    Volume(const std::initializer_list<int> args)
    {
        if (args.size() != 2*N)
            throw std::invalid_argument("Invalid initialiser list");

        auto i = args.begin();
        for (size_t init_dim = 0; ( i != args.end() ) && ( init_dim < N); init_dim++, i = std::next(i, 2) )
        {
            std::array<int, 2> extent;

            extent[0] = *i;
            extent[1] = *(i+1);

            setExtents(init_dim, extent);
        }

    }
    void setExtents(size_t dimension, int min, int max)
    {
        setExtents(dimension, {min, max});
    }

    void setExtents(size_t dimension, const std::array<int, 2>& values)
    {
        if (dimension >= N)
        {
            throw std::out_of_range("Invalid dimentions");
        }

        if (values[0] < values[1])
        {
            m_extents[dimension][0] = values[0];
            m_extents[dimension][1] = values[1];
        }
        else
        {
            m_extents[dimension][0] = values[1];
            m_extents[dimension][1] = values[0];
        }

        m_size = _size();
    }

    /* Check if this volume is equal to other in all dimensions */
    bool equals(const Volume& other) const
    {
        return m_extents == other.m_extents;
    }

    /* Check if this volume overlaps / contains other */
    bool contains(const Volume& other) const
    {
        for (size_t i=0; i<N; ++i)
        {
            if ( ! ( (m_extents[i][0] <= other.m_extents[i][0]) && ( (other.m_extents[i][1] <= m_extents[i][1] )) ) ) return false;
        }

        return true;
    }

    /* Check if both volumes intersect */
    bool intersects(const Volume& other) const
    {
        std::bitset<N> intersections;
        for (size_t i=0; i<N; ++i)
        {
            for (auto &pos : other.m_extents[i])
            {
                if ( (m_extents[i][0] <= pos) && (pos <= m_extents[i][1]) ) 
                {
                    intersections.set(i);
                    break;
                }
            }
            if (! intersections.test(i))
            {
                for (auto &pos : m_extents[i])
                {
                    if ( (other.m_extents[i][0] <= pos) && (pos <= other.m_extents[i][1]) )
                    {
                        intersections.set(i);
                        break;
                    }
                }
            }
        }

        return intersections.all();
    }

    /* Limit the given volume to our bounds */
    Volume limit(const Volume& other) const
    {
        Volume ret(other);

        for (size_t i=0; i<N; ++i)
        {
            std::array<int, 2> extent = ret.m_extents[i];

            if ( (ret.m_extents[i][0] < m_extents[i][0]) && (ret.m_extents[i][1] >= m_extents[i][0]) )
            {
                extent[0] = m_extents[i][0];
            }

            if ( (ret.m_extents[i][0] <= m_extents[i][1]) && (ret.m_extents[i][1] > m_extents[i][1]) )
            {
                extent[1] = m_extents[i][1];
            }

            ret.setExtents(i, extent);
        }

        return ret;
    }

    /* Add another cut to our cutlist */
    bool addCut(const Volume& cut)
    {
        /* Limit the cut to our max volume */
        auto _cut = limit(cut);

        if (contains(_cut))
        {
            /* When the cut is equal to ourselves, we basically disappear */
            if (equals(_cut))
            {
                cuts.clear();
                cuts.push_back(_cut);
                return false;
            }

            auto i = cuts.begin();
            for (; i != cuts.end(); i = std::next(i))
            {
                if (_cut.contains(*i))
                {
                    *i = _cut;
                    break;
                }
                else if (i->contains(_cut))
                {
                    break;
                }
            }

            if (i == cuts.end())
            {
                cuts.insert( cuts.end(), _cut);
            }
        }

        return true;
    }

    void print(std::ostream& s, bool showCuts = false) const
    {
        for (size_t i=0; i<N; ++i)
        {
            if (i != 0) s<< ",";
            s << axes[i] << "=" << std::setw(8) << m_extents[i][0] << ".." << std::setw(8) << m_extents[i][1];
        }

        if (showCuts)
        {
            s << std::endl;
            for (auto &c : cuts)
            {
                s << " - C : "; 
                c.print(s, false); 
                s << std::endl;
            }
        }
    }

    struct Plane
    {
        enum Direction
        {
            UP,
            DOWN
        };

        Plane() : dimension(0), positions({0,0}), direction(UP) {};

        Plane( size_t a_dimension, int a_position, Direction a_direction )
            : dimension(a_dimension)
            , direction(a_direction)
        {
            if (! valid())
                throw std::invalid_argument("Bad dimension");

            if (direction == UP)
            {
                positions[0] = a_position - 1;
                positions[1] = a_position;
            }
            else
            {
                positions[0] = a_position;
                positions[1] = a_position+1;
            }
        }
      
        size_t dimension;
        std::array<int, 2> positions;
        Direction direction;

        bool operator==(const Plane& other) const
        {
            if (dimension != other.dimension) return false;
            if (positions != other.positions) return false;

            return true;
        }

        bool operator<(const Plane& other) const
        {
            if (dimension < other.dimension) return true;
            if (dimension > other.dimension) return false;

            if (positions[0] < other.positions[0]) return true;
            if (positions[0] > other.positions[0]) return false;

            if (positions[1] < other.positions[1]) return true;
            if (positions[1] > other.positions[1]) return false;

            return false;
        }

        bool valid() const
        {
            return (dimension >= 0) && (dimension < N) && ((direction == UP) || (direction == DOWN));
        }

        void print(std::ostream& s) const
        {
            s << "Plane(" << axes[dimension] << "," << positions[0] << ((direction == UP) ? ",+" : ",-") << ")";
        }
    };

    /* Enumerate the faces of the rectangle (6 in total) */
    std::list<Plane> getFaces() const
    {
        std::list<Plane> ret;

        for (size_t i=0; i<N; ++i)
        {
            ret.push_back(Plane(i, m_extents[i][0], Plane::UP));
            ret.push_back(Plane(i, m_extents[i][1], Plane::DOWN));
        }

        return ret;
    }

    /* Check if volume intersects plane */
    bool intersects(const Plane& plane) const
    {
        if (! plane.valid()) return false;

        if (plane.positions[0] < m_extents[plane.dimension][0]) return false;
        if (plane.positions[1] > m_extents[plane.dimension][1]) return false;

        return true;
    }

    std::list<Volume> combine( const Volume& other, size_t dimension ) const
    {
        std::list<Volume> ret;

        if (dimension >= N)
            throw std::invalid_argument("Invalid dimension");

        /* Check if volumes are equal in the nonspecified dimensions */
        for (size_t i=0; i <N; ++i)
        {
            if (i == dimension) continue;
            if (m_extents[i] != other.m_extents[i]) return ret;
        }

        /* Now see if the volumes are either adjacent or intersecting in the
         * specified dimensions
         */
        const std::array<int,2> dims_l = m_extents[dimension];
        const std::array<int,2> dims_r = other.m_extents[dimension];

        bool canMerge = false;

        if (( dims_l[0] < dims_r[0] ) && ( dims_l[1]+1 >= dims_r[0] ))
        {
            canMerge = true;
        }
        else if (( dims_r[0] < dims_l[0] ) && ( dims_r[1]+1 >= dims_l[0] ))
        {
            canMerge = true;
        }

        if (canMerge)
        {
            ret.emplace_back(*this);
            ret.front().setExtents(dimension, std::min(dims_l[0], dims_r[0]), std::max(dims_l[1], dims_r[1]));
        }

        return ret;
    }

    /* Try to combine volumes in the list to reduce the list size */
    static std::list<Volume> combine( const std::list<Volume>& inspace )
    {
        std::list<Volume> ret = inspace;

        bool hadUpdates = true;
        while (hadUpdates)
        {
            hadUpdates = false;
            for (size_t d = 0; (!hadUpdates) && (d<N); ++d)
            {
                std::map<uint64_t, std::list<Volume>> sizemap;

                for (auto &v: ret)
                {
                    sizemap[v.size(d)].push_back(v);
                }

                ret.clear();

                for (auto &fs : sizemap)
                {
                    for (auto i = fs.second.begin(); i != fs.second.end(); i = std::next(i))
                    {
                        auto j = ( i == fs.second.end() ) ? fs.second.end() : std::next(i);
                        while (j != fs.second.end())
                        {
                            auto v = i->combine(*j, d);
                            if (! v.empty())
                            {
                                j = fs.second.erase(j);
                                *i = v.front();
                                hadUpdates = true;
                            }
                            else
                            {
                                j = std::next(j);
                            }
                        }
                    }

                    ret.insert( ret.end(), fs.second.begin(), fs.second.end() );
                }
            }
        }

        return ret;
    }

    /* Explodes the volume according to plane.  If no intersection, returns volume */
    std::list<Volume> explode(const Plane& plane) const
    {
        std::list<Volume> ret;

        if (intersects(plane))
        {
            ret.push_back(*this);
            ret.push_back(*this);

            ret.front().setExtents( plane.dimension, m_extents[plane.dimension][0], plane.positions[0]);
            ret.back().setExtents( plane.dimension, plane.positions[1], m_extents[plane.dimension][1]);
        }
        else
        {
            ret.push_back(*this);
        }

        return ret;
    }

    /* Explodes the volume according to the cuts.  If no cuts, return volume itself.
     * If the cut makes the volume disappear, returns an empty list.
     */
    std::list<Volume> explode() const
    {
        /* make a set of all distinct faces in the cutset
         */
        std::set<Plane> faces;
        for (auto &c : cuts)
        {
            for (auto &f : c.getFaces())
            {
                faces.insert( f );
            }
        }

        /* Slice the volume based on all the faces. */
        std::list<Volume> ret;

        ret.push_back(*this);
        ret.front().cuts.clear();

        for (auto &f : faces)
        {
            std::list<Volume> vl;
            for (auto &v : ret)
            {
                vl += v.explode(f);
            }
            ret = vl;
        }

        /* Now check for each part of the volume if there is a cut that matches it
         * in that case the volume part disappears
         */
        ret.remove_if([&]( const Volume& v) {
            for (auto &c: cuts)
            {
                if (c.contains(v)) return true;
            }
            return false;
        });

        return combine(ret);
    }   

    /* return a list of decomposed volumes by intersecting with other.  Empty 
     * list if no intersection.  If volumes are equal a single item list is returned.
     */
    std::list<Volume> explode(const Volume& other) const
    {
        std::list<Volume> ret;

        if (equals(other))
        {
            ret.push_back(*this);
        }
        else if (intersects(other))
        {
            std::set<Plane> faces;
         
            ret.push_back(*this);
            ret.push_back(other);

            for (auto &v: ret)
            {
                for (auto &f : v.getFaces())
                {
                    faces.insert( f );
                }
            }

            for (auto &f : faces)
            {
                std::list<Volume> ll;

                for (auto &v : ret)
                {
                    ll += v.explode(f);
                }

                ret = ll;
            }

            ret = reduce(ret);
        }

        return combine(ret);;
    }

    /* Removes equal volumes and volumes contained by others in the list */
    static std::list<Volume> reduce( const std::list<Volume>& space )
    {
        std::list<Volume> ret = space;

        auto sortFunc = [](const Volume& lhs, const Volume& rhs)
        {
            return lhs.size() > rhs.size();
        };

        ret.sort( sortFunc );

        /* Remove duplicates and contained bits */
        for (auto i = ret.begin(); i != ret.end(); i = std::next(i))
        {
            auto j = (i == ret.end()) ? ret.end() : std::next(i);
            while (j != ret.end())
            {
                if ( i->contains(*j))
                {
                    j = ret.erase(j);
                }
                else
                {
                     j = std::next(j);
                }
            }
        }

        return ret;
    }

    static std::list<Volume> explode( const std::list<Volume>& space )
    {
        std::list<Volume> ret;

        for (auto &v : space)
        {
            ret += v.explode();
        }

        /* Remove duplicates and contained volumes */
        ret = reduce(ret);

        std::list<Volume> conflicts;

        for (auto& v: ret)
        {
            v.m_flag = false;
        }

        /* Ret now contains a full list of individual cut up rectangles, now find intersectiosn */            
        for (auto i = ret.begin(); i != ret.end(); i = std::next(i))
        {
            auto j = (i == ret.end()) ? ret.end() : std::next(i);
            while (j != ret.end())
            {
                if ( i->intersects(*j))
                {
                    i->m_flag = true;
                    j->m_flag = true;
                }

                j = std::next(j);
            }
        }

        ret.remove_if( [&conflicts](const Volume& v) {
            if (v.m_flag)
            {
                conflicts.push_back(v);
            }

            return v.m_flag;
        });

        while (! conflicts.empty())
        {
            /* Conflicts now contains volumes that are conflicting with each other, intersect and explode them */
            size_t n_conflicts = 0;
            for (auto i = conflicts.begin(); i != conflicts.end();)
            {
                auto j = (i == ret.end()) ? ret.end() : std::next(i);
                while ( j != conflicts.end() )
                {
                    auto k = i->explode(*j);
                    if (! k.empty())
                    {
                        n_conflicts++;
                        j = conflicts.erase(j);
                        i = conflicts.erase(i);

                        ret.insert( conflicts.end(), k.begin(), k.end() );
                        break;
                    }

                    j = std::next(j);
                }

                if (j == conflicts.end())
                {
                    i = std::next(i);
                }
            }

            conflicts = reduce(conflicts);

            if (n_conflicts == 0)
            {
                ret.insert( ret.end(), conflicts.begin(), conflicts.end() );
                conflicts.clear();
            }
        }
        
        ret = reduce(ret);
        ret = combine(ret);

        return ret;
    }

    uint64_t size() const
    {
        return m_size;
    }

    uint64_t size(size_t dimension) const
    {
        return _size(dimension);
    }

    static uint64_t totalSize(const std::list<Volume>& space) 
    {
        uint64_t ret = 0;
        for (auto &v: space)
        {
            ret += v.size();
        }
        return ret;
    }

private:

    uint64_t _size(size_t dimension = N) const
    {
        uint64_t ret = 1;

        for (size_t i=0; i<N; ++i)
        {
            if (i != dimension)
            {
                ret *= 1 + ( m_extents[i][1] - m_extents[i][0] );
            }
        }

        return ret;
    }

    std::array<std::array<int, 2>, N> m_extents;
    
    std::list<Volume> cuts;

    uint64_t m_size;
    bool m_flag;
};

template<size_t N>
std::ostream& operator<<(std::ostream& s, const Volume<N>& v)
{
    v.print(s); return s;
}

template<size_t N>
std::list<Volume<N>>& operator+=( std::list<Volume<N>>& lhs, const std::list<Volume<N>>& rhs)
{
    lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    return lhs;
}

template<size_t N>
std::list<Volume<N>>& operator+=( std::list<Volume<N>>& lhs, const Volume<N>& rhs)
{
    lhs.insert(lhs.end(), rhs);
    return lhs;
}

template<size_t N>
std::list<Volume<N>>& operator-=( std::list<Volume<N>>& lhs, const Volume<N>& rhs)
{
    auto i = lhs.begin();
    for (; i != lhs.end();)
    {
        if (! i->addCut(rhs))
        {
            i = lhs.erase(i);
        }
        else
        {
            i = std::next(i);
        }
    }

    return lhs;
}

template<size_t N>
std::list<Volume<N>> operator&( const std::list<Volume<N>>& lhs, const Volume<N>& rhs)
{
    std::list<Volume<N>> ret;

    for (auto &v : lhs)
    {
        auto v2 = rhs.limit(v);

        if (rhs.contains(v2))
        {
            ret.push_back(v2);
        }
    }

    return ret;
}


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<std::pair<char, Volume<3>>> operations;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^\\s*(o[nf]f*)\\s+x=([-0-9]+)[.][.]([-0-9]+)\\s*,\\s*y=([-0-9]+)[.][.]([-0-9]+)\\s*,\\s*z=([-0-9]+)[.][.]([-0-9]+)\\s*$");;

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                try
                {
                    Volume<3> v;

                    for (size_t i=0; i<3; ++i)
                    {
                        v.setExtents(i, { std::stoi(match[2+(i*2)].str()), std::stoi(match[3+(i*2)].str()) });
                    }

                    char op = ( match[1].str() == "on" ) ? '+' : '-';

                    operations.emplace_back( op, v );
                }
                catch (std::invalid_argument&)
                {
                }
            } 
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    std::list<Volume<3>> space;
    
    for (auto &op : operations)
    {
        if (op.first == '+')
        {
            space += op.second;
        }
        else
        {
            space -= op.second;
        }
    }
    
    space = Volume<3>::explode(space);

    std::cout << "  Init procedure : " << Volume<3>::totalSize( space & Volume<3>({-50, 50, -50, 50, -50, 50}) ) << " Cubes on" << std::endl;
    std::cout << "Reboot procedure : " << Volume<3>::totalSize( space ) << " Cubes on" << std::endl;

    return 0;
}
