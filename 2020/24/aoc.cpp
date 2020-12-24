#include <string>
#include <array>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include <set>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <future>
#include <forward_list>
#include <list>

template<size_t N>
class Coord
{
    public:
        enum Neighbour {
           EAST      = 0,
           SOUTHEAST = 1,
           SOUTHWEST = 2,
           WEST      = 3,
           NORTHWEST = 4,
           NORTHEAST = 5
        };

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

        Coord operator+( Neighbour n ) const
        {
            Coord ret(*this);

            ret += n;

            return ret;
        }

        Coord& operator +=( const Neighbour n )
        {
            switch(n)
            {
                case EAST      : p[0] +=  4;             break;
                case SOUTHEAST : p[0] +=  2; p[1] += -6; break;
                case SOUTHWEST : p[0] += -2; p[1] += -6; break;
                case WEST      : p[0] += -4;             break;
                case NORTHWEST : p[0] += -2; p[1] +=  6; break;
                case NORTHEAST : p[0] +=  2; p[1] +=  6; break;
                default:
                    throw std::invalid_argument("Unknown neigbour");
            }

            return *this;
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

        static const std::array<Neighbour, 6> neighbours;

    private:
        std::array<int, N> p;
};

template<>
const std::array<Coord<2>::Neighbour, 6> Coord<2>::neighbours = {
{
  EAST      ,
  SOUTHEAST ,
  SOUTHWEST ,
  WEST      ,
  NORTHWEST ,
  NORTHEAST
}};

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

class Sequence
{
    public:

        Sequence(const std::string& line)
        {
            char last = ' ';
            for (auto c : line)
            {
                switch (c)
                {
                    case 'e':
                        switch (last)
                        {
                            case ' ':
                                m_Seq.emplace_back(Coord<2>::EAST);
                                break;
                            case 's':
                                m_Seq.emplace_back(Coord<2>::SOUTHEAST);
                                break;
                            case 'n':
                                m_Seq.emplace_back(Coord<2>::NORTHEAST);
                                break;
                            default:
                                throw std::invalid_argument("Syntax error");
                                break;
                        }
                        last = ' ';
                        break;
                    case 'w':
                        switch (last)
                        {
                            case ' ':
                                m_Seq.emplace_back(Coord<2>::WEST);
                                break;
                            case 's':
                                m_Seq.emplace_back(Coord<2>::SOUTHWEST);
                                break;
                            case 'n':
                                m_Seq.emplace_back(Coord<2>::NORTHWEST);
                                break;
                            default:
                                throw std::invalid_argument("Syntax error");
                                break;
                        }
                        last = ' ';
                        break;
                    case 's':
                        last = 's';
                        break;
                    case 'n':
                        last = 'n';
                        break;
                    default:
                        throw std::invalid_argument("Syntax error");
                        break;
                }
            }

            if (last != ' ')
            {
                throw std::invalid_argument("Syntax error");
            }


            for (auto& i : m_Seq)
            {
                m_Delta += i;
            }
        }

        operator std::string()
        {
            std::string ret;

            ret += m_Delta.toString();

            for (auto s: m_Seq)
            {
                switch (s)
                {
                    case Coord<2>::EAST      : ret += 'e';  break;
                    case Coord<2>::SOUTHEAST : ret += "se"; break;
                    case Coord<2>::WEST      : ret += 'w';  break;
                    case Coord<2>::SOUTHWEST : ret += "sw"; break;
                    case Coord<2>::NORTHWEST : ret += "nw"; break;
                    case Coord<2>::NORTHEAST : ret += "ne"; break;
                    default:
                        break;
                }
            }


            return ret;
        }

        const Coord<2>& getPosition()
        {
            return m_Delta;
        }

    private:

        std::vector<Coord<2>::Neighbour> m_Seq;
        Coord<2>                         m_Delta;
};

class List
{
    public:
        List() {};

        void addSequence(const std::string& line)
        {
            m_Sequences.emplace_back(line);
        }

        void resolve()
        {
            m_Tiles.clear();

            for (auto &s: m_Sequences)
            {
                m_Tiles[s.getPosition()] += 1;
            }

            cleanup();
        }

        int countBlack()
        {
            int ret = 0;
            int pos = 0;
            for (auto& k : m_Tiles)
            {
                // std::cout << "Tile " << ++pos << " fliped " << k.second << " times" << std::endl;
                if (k.second & 0x1) ret++;
            }

            return ret;
        }

        void cleanup()
        {
            /* Remove all white tiles */
            for (auto k = m_Tiles.begin(); k != m_Tiles.end();)
            {
                if (k->second & 0x1)
                {
                    ++k;
                }
                else
                {
                    k = m_Tiles.erase(k);
                }
            }
        }

        void print()
        {
            for (auto &s : m_Sequences )
            {
                std::cout << (std::string)s << std::endl;
            }
        }

        void print2()
        {
            for (auto &t : m_Tiles )
            {
                std::cout << "Tile " << t.first.toString() << " : " << t.second << std::endl;
            }
        }

        void iterate()
        {
            std::unordered_map<Coord<2>, unsigned> changes;
            std::unordered_map<Coord<2>, std::bitset<6>> white_tiles; /*  white tiles with black neighbours */

            for (auto& t : m_Tiles)
            {

                /* We only consider black tiles */
                if ((t.second & 0x01) == 0)
                {
                    white_tiles[t.first] = 1;
                }
                else
                {
                    /* Any black tile with zero or more
                     * than 2 black tiles immediately adjacent to it is flipped to white.
                     */
                    size_t c = 0;
                    for (auto n : Coord<2>::neighbours)
                    {
                        Coord<2> p(t.first);
                        p += n;

                        unsigned state = 0;
                        auto z = m_Tiles.find(p);
                        if (z != m_Tiles.end())
                        {
                            state = z->second;
                        }
                        if (state & 0x01)
                        {
                            c++;
                        }
                        else
                        {
                            /* This is a white tile adjacent to a black tile.  Set our corresponding
                             * neighbour flag. So we can consider it for the next flip rule.
                             */
                            white_tiles[p].set((int)n);
                        }
                    }

                    if ((c == 0) || (c > 2))
                    {
                        changes[t.first] += 1;
                    }
                }
            }

            /*
             * Any white tile with exactly 2 black tiles immediately
             * adjacent to it is flipped to black.  That means any white tile with exactly
             * 2 bits set in the bitmask.
             */
            for (auto &t : white_tiles)
            {
                if (t.second.count() == 2)
                {
                    changes[t.first] += 1;
                }
            }

            for (auto& t : changes)
            {
                m_Tiles[t.first] += t.second;
            }

            cleanup();
        }

    private:

        std::vector<Sequence> m_Sequences;
        std::unordered_map<Coord<2>, unsigned> m_Tiles;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    List list;
    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_w("^([snew]+)\\s*$");
        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_w))
            {
                list.addSequence(match[1].str());
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        return -1;
    }

    list.resolve();

    std::cout << list.countBlack() << " tiles with black side facing up" << std::endl;
    size_t day = 1;
    for (; day <= 10; ++day)
    {
        list.iterate();
        // std::cout << list.countBlack() << " tiles with black side facing up (Day " << day << ")" << std::endl;
    }

    for (; day <= 100; ++day)
    {
        list.iterate();
        if ((day % 10) == 0)
        {
            //std::cout << list.countBlack() << " tiles with black side facing up (Day " << day << ")" << std::endl;
        }
    }
    std::cout << list.countBlack() << " tiles with black side facing up (Day " << (day-1) << ")" << std::endl;

    return 0;
}
