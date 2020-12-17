#include <string>
#include <array>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <thread>

template<size_t N>
class Coord
{
    public:
        Coord()
        {
            p.fill(0);
        }

        Coord(const std::array<int,N>& a_Coords) : p(a_Coords) {};

        template<size_t N2>
        Coord(const Coord<N2>& a_Other)
        {
            p.fill(0);
            for (size_t i = 0; i<std::min(N,N2); ++i)
            {
                p[N-i-1] = a_Other[N2-i-1];
            }
        }

        Coord(const Coord& a_Other)
        {
            p = a_Other.p;
        }

        bool operator<(const Coord& other) const
        {
            for (size_t i=0; i<N; ++i)
            {
                if (p[i] < other.p[i]) return true;
            }

            return false;
        }

        void minimize(const Coord& other)
        {
            for (size_t i=0; i<N; ++i)
            {
                if (other[i] < p[i]) p[i] = other[i];
            }
        }

        void maximize(const Coord& other)
        {
            for (size_t i=0; i<N; ++i)
            {
                if (other[i] > p[i]) p[i] = other[i];
            }
        }

        int operator[] (size_t i) const
        {
            return p[i];
        }

        int& operator[](size_t i)
        {
            return p[i];
        }

        Coord operator+( const Coord& T2 ) const
        {
            Coord ret;

            for (size_t i=0; i<N; ++i)
            {
                ret[i] = p[i] + T2[i];
            }

            return ret;
        }

        bool operator==( const Coord& T2 ) const
        {
            for (size_t i=0; i<N; ++i)
            {
                if (p[i] != T2.p[i]) return false;
            }
            return true;
        }

        bool empty()
        {
            for (auto v : p)
            {
                if (v != 0) return false;
            }

            return true;
        }

        size_t hash() const noexcept
        {
            size_t ret = 0;
            for (const auto& d : p)
            {
                ret = (ret * 31) + std::hash<int>()(d);
            }
            return ret;
        }

        std::string toString() const
        {
            std::string ret;
            for (const auto& d : p)
            {
                std::array<char, 32> buf;
                if (! ret.empty())
                {
                    ret += std::string(buf.data(), std::snprintf(buf.data(), buf.size(), ", %d", d));
                }
                else
                {
                    ret += std::string(buf.data(), std::snprintf(buf.data(), buf.size(), "(  %d", d));
                }
            }
            ret += " )";

            return ret;
        }

    private:
        std::array<int, N> p;
};

namespace std
{
    template<size_t N> struct hash<Coord<N>>
    {
        std::size_t operator() (Coord<N> const& s) const noexcept
        {
            return s.hash();
        }
    };
}

template<size_t N>
class Cell : public std::vector<Coord<N>>
{
    public:
        Cell(const Coord<N>& a_Coord)
            : std::vector<Coord<N>>()
            , me(a_Coord)
        {

            /* Generate a list of neighbour coordinates */
            Coord<N> i;
            generate(i, 0);
        }

        Coord<N> me;

    private:

        void generate(Coord<N>& var, size_t depth)
        {
            if (depth >= N)
            {
                if (! var.empty())
                {
                    this->emplace_back( var + me );
                }
            }
            else
            {
                for (var[depth] = -1; var[depth] < 2; ++var[depth])
                {
                    generate(var, depth+1);
                }
            }
        }
};


template<size_t N>
class Map
{
    public:
        Map() {};

        Map(const Map& other) : min(other.min), max(other.max), m_Map(other.m_Map) {};

        bool test(const Coord<N>& where) const
        {
            try
            {
                return m_Map.at(where);
            }
            catch(std::out_of_range&)
            {
            }

            return false;
        }

        void set(const Coord<N>& where, bool state = true)
        {
            m_Map[where] = state;
            if (state)
            {
                min.minimize(where);
                max.maximize(where);
            }
        }

        Coord<N> min;
        Coord<N> max;

        void print()
        {
            Coord<N> c;
            _print(c, 0);
        }

        void iterate(const Map<N>& old)
        {
            Coord<N> i;
            _iterate(old,i, 0);
        }

        size_t Count()
        {
            size_t ret = 0;
            for (auto &c: m_Map)
            {
                if (c.second) ret++;
            }

            return ret;
        }

    private:

        std::unordered_map<Coord<N>, bool> m_Map;

        void _print(Coord<N>& o, size_t depth)
        {
            if (depth + 2 >= N)
            {
                std::cout << "Layer " << o[0];
                for (size_t i=1; i<depth; ++i)
                {
                    std::cout << ", " << o[i];
                }

                std::cout << std::endl;

                for (o[depth] = min[depth]; o[depth] <= max[depth]; o[depth]++)
                {
                    std::array<char, 16> buf;
                    std::snprintf(buf.data(), buf.size(), "% 4d ", o[depth]);
                    std::cout << buf.data();

                    for (o[depth+1] = min[depth+1]; o[depth+1] <= max[depth+1]; o[depth+1]++)
                    {
                        if (test(o))
                        {
                            std::cout << "#";
                        }
                        else
                        {
                            std::cout << ".";
                        }
                    }
                    std::cout << std::endl;
                }
            }
            else
            {
                for (o[depth] = min[depth]; o[depth] <= max[depth]; ++o[depth])
                {
                    _print(o, depth+1);
                }
            }
        }

        void _iterate(const Map<N>& old, Coord<N>& i, size_t depth)
        {
            if (depth >= N)
            {
                Cell<N> c( i );

                size_t active_neighbours = 0;
                for (auto &neighbour: c)
                {
                    if (old.test(neighbour))
                    {
                        active_neighbours++;
                        if (active_neighbours > 3) break;
                    }
                }

                bool new_state = false;
                if ( old.test(i) )
                {
                    // Cell is active
                    if ((active_neighbours == 2) || (active_neighbours == 3))
                    {
                        new_state = true;
                    }
                }
                else
                {
                    // Cell is dead
                    if (active_neighbours == 3)
                    {
                        new_state = true;
                    }
                }

                set(i, new_state);
            }
            else
            {
                for (i[depth] = (old.min[depth]-1); i[depth] <= (old.max[depth]+1); i[depth]++)
                {
                    _iterate(old, i, depth+1);
                }
            }
        }
};

template<size_t N>
class Resolver
{
    public:

        Resolver(std::shared_ptr<Map<N>> a_Map, size_t n_Iterations)
            : map(a_Map)
            , iterations(n_Iterations)
        {
        };

        void operator() ()
        {
            for (size_t i=0; i<iterations; ++i)
            {
                std::shared_ptr<Map<N>> map2 = std::make_shared<Map<N>>();
                map2->iterate(*map);
                map.swap( map2 );
            }
        }

        std::shared_ptr<Map<N>> map;
        size_t iterations;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::shared_ptr<Map<3>> map3d = std::make_shared<Map<3>>();
    std::shared_ptr<Map<4>> map4d = std::make_shared<Map<4>>();

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_w("^\\s*([#.]+)\\s*$");

        Coord<3> p;

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_w))
            {
                const std::string &field = match[1].str();
                p[2] = 0;

                for (auto c : field)
                {
                    map3d->set(p, (c=='#'));
                    map4d->set(p, (c=='#'));

                    p[2]++;
                }

                p[1]++;
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }


    std::cout << "Input space:" << std::endl;
    std::cout << "- Min          : " << map3d->min.toString() << std::endl;
    std::cout << "- Max          : " << map3d->max.toString() << std::endl;
    std::cout << "- Active cubes : " << map3d->Count() << std::endl;

    map3d->print();

    std::cout << "Starting work ..." << std::endl;;

    Resolver<3> r3d(map3d, 6);
    Resolver<4> r4d(map4d, 6);

    std::thread t3d(std::ref(r3d));
    std::thread t4d(std::ref(r4d));

    t3d.join();

    std::cout << "3D resolution : " << std::endl;
    std::cout << "- Min          : " << r3d.map->min.toString() << std::endl;
    std::cout << "- Max          : " << r3d.map->max.toString() << std::endl;
    std::cout << "- Active cubes : " << r3d.map->Count() << std::endl;

    t4d.join();

    std::cout << "4D resolution : " << std::endl;
    std::cout << "- Min          : " << r4d.map->min.toString() << std::endl;
    std::cout << "- Max          : " << r4d.map->max.toString() << std::endl;
    std::cout << "- Active cubes : " << r4d.map->Count() << std::endl;

    return 0;
}
