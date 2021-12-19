#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
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

namespace Matrix 
{
    int cos(unsigned angle)
    {
        constexpr std::array<int, 4> angles{{ 1, 0, -1, 0}};
        return angles[(angle & 0x3)];
    }

    int sin(unsigned angle)
    {
        constexpr std::array<int, 4> angles{{ 0, 1, 0, -1}};
        return angles[(angle & 0x3)];
    }

    std::array<std::array<int,3>,3> rotX(unsigned angle)
    {
        std::array<std::array<int,3>,3> ret;

        ret[0][0] = 1;
        ret[0][1] = 0;
        ret[0][2] = 0;

        ret[1][0] = 0;
        ret[1][1] = cos(angle);
        ret[1][2] = -sin(angle);

        ret[2][0] = 0;
        ret[2][1] = sin(angle);
        ret[2][2] = cos(angle);

        return ret;
    }

    std::array<std::array<int,3>,3> rotY(unsigned angle)
    {
        std::array<std::array<int,3>,3> ret;

        ret[0][0] = cos(angle);
        ret[0][1] = 0;
        ret[0][2] = sin(angle);

        ret[1][0] = 0;
        ret[1][1] = 1;
        ret[1][2] = 0;

        ret[2][0] = -sin(angle);
        ret[2][1] = 0;
        ret[2][2] = cos(angle);

        return ret;
    }

    std::array<std::array<int,3>,3> rotZ(unsigned angle)
    {
        std::array<std::array<int,3>,3> ret;

        ret[0][0] = cos(angle);
        ret[0][1] = -sin(angle);
        ret[0][2] = 0;

        ret[1][0] = sin(angle);
        ret[1][1] = cos(angle);
        ret[1][2] = 0;

        ret[2][0] = 0;
        ret[2][1] = 0;
        ret[2][2] = 1;

        return ret;
    }
   
    std::array<int,3> mul(const std::array<std::array<int,3>,3>& A, const std::array<int,3>& B)
    {
        std::array<int,3> ret;

        for (size_t y = 0; y<ret.size(); ++y)
        {
            ret[y] = 0;
            for (size_t x = 0; x<ret.size(); ++x)
            {
                ret[y] += B[x] * A[y][x];
            }            
        }

        return ret;
    }

    std::array<int,3> rotate(const std::array<int,3>& pos, const std::array<int, 3>& angles )
    {   
        return mul(rotZ(angles[2]), mul(rotY(angles[1]), mul(rotX(angles[0]), pos)));
    }

    std::array<int,3> translate(const std::array<int,3>& pos, const std::array<int, 3>& xlate )
    {
        std::array<int,3> ret;

        for (size_t i=0; i<ret.size(); ++i)
        {
            ret[i] = pos[i] + xlate[i];
        }

        return ret;
    }

    std::array<int,3> reverse(const std::array<int,3>& pos)
    {
        std::array<int,3> ret;

        for (size_t i=0; i<ret.size(); ++i)
        {
            ret[i] = -pos[i];
        }

        return ret;
    }

    std::array<int,3> delta(const std::array<int,3>& a, const std::array<int,3>& b)
    {
        std::array<int,3> ret;

        for (size_t i=0; i<ret.size(); ++i)
        {
            ret[i] = a[i] - b[i];
        }

        return ret; 
    }

    constexpr std::array<int,3> zero() 
    {
        return {0,0,0};
    }

    constexpr std::array<std::array<int,3>,4*4*4> rotationList()
    {
        std::array<std::array<int,3>,4*4*4> ret{{
            { 0, 0, 0 },
            { 0, 0, 1 },
            { 0, 0, 2 },
            { 0, 0, 3 },
            { 0, 1, 0 },
            { 0, 1, 1 },
            { 0, 1, 2 },
            { 0, 1, 3 },
            { 0, 2, 0 },
            { 0, 2, 1 },
            { 0, 2, 2 },
            { 0, 2, 3 },
            { 0, 3, 0 },
            { 0, 3, 1 },
            { 0, 3, 2 },
            { 0, 3, 3 },
            { 1, 0, 0 },
            { 1, 0, 1 },
            { 1, 0, 2 },
            { 1, 0, 3 },
            { 1, 1, 0 },
            { 1, 1, 1 },
            { 1, 1, 2 },
            { 1, 1, 3 },
            { 1, 2, 0 },
            { 1, 2, 1 },
            { 1, 2, 2 },
            { 1, 2, 3 },
            { 1, 3, 0 },
            { 1, 3, 1 },
            { 1, 3, 2 },
            { 1, 3, 3 },
            { 2, 0, 0 },
            { 2, 0, 1 },
            { 2, 0, 2 },
            { 2, 0, 3 },
            { 2, 1, 0 },
            { 2, 1, 1 },
            { 2, 1, 2 },
            { 2, 1, 3 },
            { 2, 2, 0 },
            { 2, 2, 1 },
            { 2, 2, 2 },
            { 2, 2, 3 },
            { 2, 3, 0 },
            { 2, 3, 1 },
            { 2, 3, 2 },
            { 2, 3, 3 },
            { 3, 0, 0 },
            { 3, 0, 1 },
            { 3, 0, 2 },
            { 3, 0, 3 },
            { 3, 1, 0 },
            { 3, 1, 1 },
            { 3, 1, 2 },
            { 3, 1, 3 },
            { 3, 2, 0 },
            { 3, 2, 1 },
            { 3, 2, 2 },
            { 3, 2, 3 },
            { 3, 3, 0 },
            { 3, 3, 1 },
            { 3, 3, 2 },
            { 3, 3, 3 }
        }};

        return ret;
    }
};

class Beacon : public std::enable_shared_from_this<Beacon>
{
public:
    Beacon(int ax, int ay, int az, int aid)
        : p({ax, ay, az})
        , pc({ax, ay, az})
        , id(aid)
    {}

    std::array<int, 3> p;   /* position */
    std::array<int, 3> pc;  /* position corrected */

    int id;

    bool operator<(const Beacon& other) const
    {
        for (size_t i = 0; i<2; ++i) 
        {
            if (p[2-i] < other.p[2-i]) return true;
            if (p[2-i] > other.p[2-i]) return false;
        }

        return false;
    }

    int distance(const std::shared_ptr<Beacon>& other) const
    {
        if (! other) return std::numeric_limits<int>::max();
        return distance(*other);
    }

    int distance(const Beacon& other) const
    {
        float ret = 0;

        for (size_t i=0; i<p.size(); ++i)
        {
            ret += std::pow(p[i] - other.p[i], 2);
        }

        return std::sqrt(ret) * 1000;
    }
};

struct BeaconCmp
{
    bool operator()(const std::shared_ptr<Beacon>& a, const std::shared_ptr<Beacon>& b) const
    {
        if ((! a) && (! b)) return false;
        if (!b) return true;
        if (!a) return false;

        return *a < *b; 
    }
};

class BeaconPair
{
public:
    BeaconPair( std::shared_ptr<Beacon> a, std::shared_ptr<Beacon> b)
    {
        m_beacons[0] = a;
        m_beacons[1] = b;

        if (!a)
        {
            m_distance = std::numeric_limits<int>::max();
        }
        else
        {
            m_distance = a->distance(b);
        }
    }

    int distance() const { return m_distance; };

    std::array<std::shared_ptr<Beacon>, 2> m_beacons;

    bool operator<(const BeaconPair& other) const
    {
        return m_distance < other.m_distance;
    }

    void swap()
    {
        std::swap(m_beacons[0], m_beacons[1]);
    }

private:

    int m_distance;
};

class Scanner
{
public:
    Scanner(int a_id)
        : id(a_id)
    {
        rotation.fill(0);
        translation.fill(0);
    }

    int getId() const { return id; }

    void addBeacon(int x, int y, int z)
    {
        measurements.insert(
            std::make_shared<Beacon>( x, y, z, measurements.size()+1 )
        );
    }

    const std::set<std::shared_ptr<Beacon>, BeaconCmp>& getMeasurements() const
    {
        return measurements;
    }

    std::set<std::shared_ptr<Beacon>, BeaconCmp>& getMeasurements()
    {
        return measurements;
    }

    std::vector<BeaconPair> distanceMap()
    {
        if ((measurements.size() > 1) && (m_distanceMap.empty()))
        {
            for (auto i = measurements.begin(); i != measurements.end(); ++i)
            {
                auto j = i;
                for (j++; j != measurements.end(); ++j)
                {
                   m_distanceMap.emplace_back( *i, *j );
                }
            }

             std::sort( m_distanceMap.begin(), m_distanceMap.end() );
        }

        return m_distanceMap;
    }

    void setRotationTranslation(const std::array<int, 3>& a_rotation, const std::array<int, 3>& a_translation = Matrix::zero())
    {
        rotation = a_rotation;
        translation = a_translation;
        calculationPositions();
    }

    void setRotation(const std::array<int, 3>& a_rotation)
    {
        rotation = a_rotation;
        calculationPositions();
    }

    void setTranslation(const std::array<int, 3>& a_translation)
    {
        translation = a_translation;
        calculationPositions();
    }

    std::array<int, 3> getRealPosition()
    {
        return translation;
    }

private:

    void calculationPositions()
    {
        for (auto &in : measurements) 
        {
            in->pc = Matrix::translate( Matrix::rotate( in->p, rotation ), translation );
        }
    }

    int id;
    std::set<std::shared_ptr<Beacon>, BeaconCmp> measurements;

    std::vector<BeaconPair> m_distanceMap;

    std::array<int, 3> rotation;
    std::array<int, 3> translation;

};

class MatchingPair
{
public:

    MatchingPair(const BeaconPair& a, const BeaconPair& b, std::shared_ptr<Scanner> sa, std::shared_ptr<Scanner> sb)
        : pairs({{a, b}})
        , scanners({{ sa, sb }})
    {
    }

    std::array<BeaconPair, 2> pairs;
    std::array<std::shared_ptr<Scanner>,2> scanners;

    std::array<int, 3> rotation;
    std::array<int, 3> translation;

    bool operator<(const MatchingPair& other)
    {
        return pairs[0].distance() < other.pairs[0].distance();
    }

    bool aligned() const
    {
        if (pairs[0].m_beacons[0]->pc != pairs[1].m_beacons[0]->pc) return false;
        if (pairs[0].m_beacons[1]->pc != pairs[1].m_beacons[1]->pc) return false;

        return true;
    }

    void setRotationTranslation(const std::array<int, 3>& rot, const std::array<int, 3>& trans = Matrix::zero())
    {
        rotation = rot;
        translation = trans;

        scanners[1]->setRotationTranslation(rot, trans);
    }

    void setRotation(const std::array<int, 3>& rot)
    {
        rotation = rot;
        scanners[1]->setRotation(rot);
    }

    void setTranslation(const std::array<int, 3>& trans)
    {
        translation = trans;
        scanners[1]->setTranslation(trans);
    }

private:
    
};

class Map
{
public:
    Map() {};

    std::vector<std::shared_ptr<Scanner>> scanners;

    void Match()
    {
        if (scanners.empty()) return;

        std::list<std::shared_ptr<Scanner>> matched;
        std::list<std::shared_ptr<Scanner>> remaining;

        for (auto &s : scanners)
        {
            if (matched.empty())
            {
                matched.push_back(s);
            }
            else
            {
                remaining.push_back(s);
            }
        }

        while (! remaining.empty())
        {
            bool found = false;
            for (auto j = matched.begin(); (!found) && (j != matched.end()); ++j)
            {
                for (auto i = remaining.begin(); (!found) && (i != remaining.end()); ++i)
                {
                    if (Match(*j, *i))
                    {
                        matched.push_back(*i);
                        remaining.erase(i);
                        found = true;
                    }
                }
            }

            if ( ! found )
            {
                throw std::invalid_argument("No solution possible");
            }
        }
    }

    std::set<std::array<int, 3>> sortBeacons()
    {
        std::set<std::array<int, 3>> ret;

        for (auto& s: scanners)
        {
            for (auto &b : s->getMeasurements())
            {
                ret.insert(b->pc);
            }
        }

        return ret;
    }

    int largestManhattenDistance()
    {
        int ret = 0;

        for (auto i = scanners.begin(); i != scanners.end(); ++i)
        {
            for (auto j = i; j != scanners.end(); ++j)
            {
                int delta = manhattenDistance(*i, *j);
                if (delta > ret)
                {
                    ret = delta;
                }
            }
        }

        return ret;
    }

private:

    std::vector<MatchingPair> findMatching( std::shared_ptr<Scanner> first, std::shared_ptr<Scanner> second )
    {
        std::vector<BeaconPair> first_b = first->distanceMap();
        std::vector<BeaconPair> second_b = second->distanceMap();

        std::vector<MatchingPair> ret;

        for (auto i=first_b.begin(); i != first_b.end(); ++i)
        {
            for (auto j = second_b.begin(); j != second_b.end(); ++j)
            {
                if ( i->distance() == j->distance() )
                {
                    ret.emplace_back( *i, *j, first, second );
                } 
            }
        }

        std::sort(ret.begin(), ret.end());

        return ret;
    }

    bool Match( std::shared_ptr<Scanner> first, std::shared_ptr<Scanner> second )
    {
        std::vector<MatchingPair> pairs = findMatching( first, second );

        pairs.erase(
            std::remove_if(pairs.begin(), pairs.end(), [second](MatchingPair& pair) {

                /* Now check if we can find a rotation for B that aligns all pairs */
                for (auto& rot : Matrix::rotationList())
                {
                    pair.setRotationTranslation(rot);
                    pair.setTranslation(Matrix::delta( pair.pairs[0].m_beacons[0]->pc, pair.pairs[1].m_beacons[0]->pc ));

                    if (pair.aligned()) 
                    {
                        return false;
                    }
                }

                pair.pairs[1].swap();

                /* Check again if we can find a rotation for B that aligns all pairs */
                for (auto& rot : Matrix::rotationList())
                {
                    pair.setRotationTranslation(rot);
                    pair.setTranslation(Matrix::delta( pair.pairs[0].m_beacons[0]->pc, pair.pairs[1].m_beacons[0]->pc ));

                    if (pair.aligned()) return false;
                }

                return true;
            }),
            pairs.end()
        );

        /* Count the distinct transformations */
        std::map<std::pair<std::array<int,3>, std::array<int,3>>, int> xfrms;
        for (auto& m : pairs)
        {
            xfrms[std::make_pair(m.rotation, m.translation)]++;
        }

        /* Finalize on the transformation with the highest amount of occurence */
        auto res = std::max_element(xfrms.begin(), xfrms.end());
        if (res == xfrms.end())
        {
            return false;
        }

        second->setRotationTranslation(res->first.first, res->first.second);

        return true;
    }

    int manhattenDistance( std::shared_ptr<Scanner> first, std::shared_ptr<Scanner> second )
    {
        auto pos_a = first->getRealPosition();
        auto pos_b = second->getRealPosition();

        auto delta = Matrix::delta(pos_a, pos_b);

        return abs(delta[0]) + abs(delta[1]) + abs(delta[2]);
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

    Map map;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_scanner("^\\s*---\\sscanner\\s([0-9]+)\\s---\\s*$");
        const std::regex matchrule_beacon("^\\s*([-0123456789]+)\\s*,\\s*([-0123456789]+)\\s*,\\s*([-0123456789]+)\\s*$");

        std::shared_ptr<Scanner> scanner;

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_scanner))
            {
                scanner = std::make_shared<Scanner>( std::stoi(match[1].str()) );
                map.scanners.emplace_back(scanner);
            }
            else if (std::regex_match(line, match, matchrule_beacon))
            {
                if (scanner)
                {
                    scanner->addBeacon(
                        std::stoi(match[1].str()),
                        std::stoi(match[2].str()),
                        std::stoi(match[3].str())
                    );
                }
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    map.Match();

    std::cout << "There are " << map.sortBeacons().size() << " Beacons" << std::endl;
    std::cout << "Larget mh distane " << map.largestManhattenDistance() << std::endl;

    return 0;
}
