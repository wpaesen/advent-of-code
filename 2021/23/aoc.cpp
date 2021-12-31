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
#include <thread>
#include <future>

template<size_t depth>
class PodSpace
{
public:
    
    class Position
    {
    public:
        Position(char a_name, int a_index)
            : m_name( a_name )
            , m_index( a_index )
            , m_final( false )
        {
            if (m_name == 'H')
            {
                if ((a_index < 1) || (a_index > 11))
                    throw std::invalid_argument("Invalid position index");

            }
            else if ((m_name >= 'A') && (m_name <= 'D'))
            {
                if ((a_index < 1) || (a_index > depth))
                    throw std::invalid_argument("Invalid position index");
            }
            else
            {
                throw std::invalid_argument("Invalid Position name");
            }
        }

        Position()
        {
            m_name = 'X';
            m_index = -1;
            m_final = true;
        }

        bool operator< (const Position& other) const
        {
            return std::make_pair(m_name, m_index) < std::make_pair(other.m_name, other.m_index);
        }

        bool operator== (const Position& other) const
        {
            return std::make_pair(m_name, m_index) == std::make_pair(other.m_name, other.m_index);
        }

        char getName() const
        {
            return m_name;
        }

        std::string getId() const
        {        
            std::string ret;

            ret += getName();
            ret += std::to_string(m_index);

            return ret;
        }

        /* Find the hallway position opposite a room, or return same position if
         * already in the hallway
         */
        Position hallway() const
        {
            switch (m_name)
            {
                case 'A':
                    return Position('H', 3);
                case 'B':
                    return Position('H', 5);
                case 'C':
                    return Position('H', 7);
                case 'D':
                    return Position('H', 9);
                default:
                    break;
            }

            return *this;
        }

        bool forbidden() const
        {
            if (m_name == 'H')
            {
                switch (m_index)
                {
                    case 3:
                    case 5:
                    case 7:
                    case 9:
                        return true;
                    default:
                        break;
                }
            }

            return false;
        }

        Position up() const
        {
            if ( m_name != 'H')
            {
                int index = m_index-1;
                if (index > 0)
                {
                    return Position(m_name, index);
                }
                else
                {
                    return hallway();
                }
            }

            return *this;
        }

        size_t distance( const Position& destination ) const
        {
            if (m_name == destination.m_name)
            {
                return std::abs(m_index - destination.m_index);
            }

            size_t ret = 0;

            Position lhs(*this);
            Position rhs(destination);

            if (lhs.m_name != 'H')
            {
                ret += lhs.m_index;
                lhs = lhs.hallway();
            }

            if (rhs.m_name != 'H')
            {
                ret += rhs.m_index;
                rhs = rhs.hallway();
            }

            return ret + lhs.distance(rhs);
        }

        bool isAbove( const Position& other ) const
        {
            if (other.m_name == 'H') return false;
            if (m_name == 'H') return true;

            return m_index < other.m_index;
        }

        bool isInBetween( const Position& _lhs, const Position &_rhs ) const
        {
            if (m_name == 'H')
            {
                /* Our position is hallway, so we need to find the 
                 * hallway position corresponding to lhs and rhs.
                 */
                Position lhs = _lhs.hallway();
                Position rhs = _rhs.hallway();

                int min_index = std::min(lhs.m_index, rhs.m_index);
                int max_index = std::max(lhs.m_index, rhs.m_index);

                return (min_index <= m_index) && (m_index <= max_index);
            }
            else
            {
                /* Our position is in a room.  So check if one of the lhs or 
                 * rhs has the same name.  When that is the case, iterate upward
                 * and check one of the positions match.
                 */
                if (_lhs.getName() == m_name)
                {
                    Position lhs(_lhs);
                    do {
                        if (lhs == *this) return true;
                        lhs = lhs.up();
                    } while (lhs.getName() != 'H');
                }
                else if (_rhs.getName() == m_name)
                {
                    Position rhs(_rhs);
                    do {
                        if (rhs == *this) return true;
                        rhs = rhs.up();
                    } while (rhs.getName() != 'H');
                }
            }

            return false;
        }

        bool isFinal() const { return m_final; }
        void setFinal() { m_final = true; }

    private:
        char m_name;
        int m_index;
        bool m_final;
    };

    class Pod
    {
    public:
        Pod(char a_name, int a_index)
            : m_name( a_name )
            , m_index( a_index )
        {
            if ((m_name < 'A') || (m_name > 'D'))
                throw std::invalid_argument("Invalid Pod name");

            if ((m_index < 1) || (m_index > depth))
                throw std::invalid_argument("Invalid Pod index");
        }

        Pod()
        {
            m_name = 'X';
            m_index = -1;
        }

        bool operator< (const Pod& other) const
        {
            return std::make_pair(m_name, m_index) < std::make_pair(other.m_name, other.m_index);
        }

        bool operator== (const Pod& other) const
        {
            return std::make_pair(m_name, m_index) == std::make_pair(other.m_name, other.m_index);
        }
        
        bool operator!= (const Pod& other) const
        {
            return std::make_pair(m_name, m_index) != std::make_pair(other.m_name, other.m_index);
        }

        bool operator== (const char a_name ) const
        {
            return m_name == a_name;
        }

        /* returns true if the position name is the same as the pod name.  Then the pod is in 
         * it's proper position 
         */
        bool operator== (const Position& pos) const
        {
            return m_name == pos.m_name;
        }

        char getName() const
        {
            return m_name;
        }

        std::string getId() const
        {
            std::string ret;

            ret += getName();
            ret += std::to_string(m_index);

            return ret;
        }

        size_t energy() const
        {
            return std::pow(10, (m_name - 'A'));
        }

        

    private:
        char m_name;
        int m_index;
    };

    static const std::array<Position, 7>& allHallwayPositions()
    {
        static std::array<Position, 7> ret{{ 
            Position('H', 1),
            Position('H', 2),
            Position('H', 4),
            Position('H', 6),
            Position('H', 8),
            Position('H', 10),       
            Position('H', 11)                      
        }};

        return ret;
    }

    std::map<Pod, Position> m_State;
    std::array<Position, 4> m_NextDestination;
    int64_t m_cost;

    PodSpace()
    {
        for (size_t i=0; i<m_NextDestination.size(); ++i)
        {
            m_NextDestination[i] = Position('A' + i, depth);
        }
        m_cost = 0;
    }

    PodSpace(const std::array<std::array<char, depth>,4>& a_rooms)
    {
        std::array<std::vector<Pod>, 4> allPods;

        for (char c = 'A'; c <= 'D'; ++c)
        {
            for (int i = 0; i<depth; ++i)
            {
                allPods[c - 'A'].push_back(Pod(c, i+1));
            }
        }

        for (size_t i=0; i<a_rooms.size(); ++i)
        {
            for (size_t d=0; d<a_rooms[i].size(); ++d)
            {
                Position pos('A' + i, d+1);

                size_t podIdx = a_rooms[i][d] -'A';

                if (allPods.at(podIdx).empty())
                {
                    throw std::invalid_argument("Invalid initial state");
                }

                Pod pod = *allPods.at(podIdx).rbegin();
                allPods.at(podIdx).pop_back();

                m_State[pod] = pos;
            }
        }

        for (size_t i=0; i<a_rooms.size(); ++i)
        {
            m_NextDestination[i] = Position('A' + i, depth);
        }

        /* Check if the initial state is already partially solved */
        bool initialSolveRetry = true;
        while (initialSolveRetry)
        {
            initialSolveRetry = false;

            for (auto p = m_State.begin(); p != m_State.end(); p = std::next(p))
            {
                if ( p->second.isFinal() ) continue;

                auto dest = getDestination(p->first);
                if (p->second == dest)
                {
                    getDestination(p->first) = getDestination(p->first).up();
                    p->second.setFinal();
                    initialSolveRetry = true;
                }
            }
        }
    }

    PodSpace(const PodSpace& other, int64_t a_cost)
        : m_State( other.m_State )
        , m_NextDestination(other.m_NextDestination)
        , m_cost(a_cost)
    {}

    bool isSolved() const
    {
        for (auto &d : m_NextDestination)
        {
            if (d.getName() != 'H') return false;
        }

        return true;
    }

    const Position& getDestination(const Pod& pod) const
    {
        return m_NextDestination.at(pod.getName()-'A');
    }

    Position& getDestination(const Pod& pod)
    {
        return m_NextDestination.at(pod.getName()-'A');
    }

    std::pair<bool, int64_t> resolve() const
    {
        if (isSolved()) return std::make_pair(true, 0);

        int64_t lowestCost = std::numeric_limits<int64_t>::max();

        /* Try to move any of the pods to either it's destination position, or any of the free positions in the hallway 
         */
        for (auto pod = m_State.begin(); pod != m_State.end(); pod = std::next(pod))
        {
            if (pod->second.isFinal()) continue;

            {
                /* Try to move to the destination slot */
                Position dest = getDestination( pod->first );
                if (dest.getName() == 'H')
                {
                    /* This is impossible */
                    throw std::logic_error("Logic collapsed");
                }

                if (canMoveTo( pod->first, dest ))
                {
                    PodSpace newspace(*this);
                    int64_t cost = pod->second.distance(dest) * pod->first.energy();

                    newspace.m_State[pod->first] = dest;
                    newspace.m_State[pod->first].setFinal();

                    newspace.getDestination( pod->first ) = getDestination( pod->first ).up();
                    /* Resolve the next steps */
                    auto res = newspace.resolve();
                    if (res.first)
                    {
                        cost += res.second;
                        if (cost < lowestCost)
                        {
                            lowestCost = cost;
                        }
                    }
                }
            }

            /* If we are not in the hallway, try to move to any of the hallway positions */
            if (pod->second.getName() != 'H')
            {
                for (auto &dest : allHallwayPositions())
                {
                    if (canMoveTo( pod->first, dest ))
                    {
                        PodSpace newspace(*this);
                        int64_t cost = pod->second.distance(dest) * pod->first.energy();

                        newspace.m_State[pod->first] = dest;

                        /* Resolve the next steps */
                        auto res = newspace.resolve();

                        if (res.first)
                        {
                            cost += res.second;
                            if (cost < lowestCost)
                            {
                                lowestCost = cost;
                            }
                        }
                    }
                }
            }
        }

        if (lowestCost < std::numeric_limits<int64_t>::max())
        {
            return std::make_pair(true, lowestCost);
        }
        else
        {
            return std::make_pair(false, 0);
        }
    }

    bool canMoveTo(const Pod& pod, const Position& destination) const
    {
        if (destination.forbidden()) 
            return false;

        auto startIter = m_State.find(pod);
        if (startIter == m_State.end())
            throw std::logic_error("Logic collapsed 2");

        const Position& start = startIter->second;

        if (start == destination) 
            return false;

        for (auto &others : m_State)
        {
            if (others.first != pod)
            {
                if (others.second == destination) return false;
                if (others.second.isInBetween(start, destination)) return false;
            }
        }

        return true;
    }

    void print(std::ostream& s) const
    {
        s << "#############" << std::endl;
        s << "#";
        for (size_t i=0; i<11; ++i)
        {
            std::cout << whatIsAt(Position('H',i+1));
        }

        s << "#" << std::endl;

        for (auto i = 0; i<depth; ++i)
        {
            if (i==0)
            {
                s << "###";
            }
            else
            {
                s << "  #";
            }

            for (auto j = 0; j<4; ++j)
            {
                std::cout << whatIsAt(Position('A'+j, i+1)) << "#";
            }

            if (i==0)
            {
                s << "##";
            }

            s << std::endl;
        }

        s << "  #########  " << std::endl;
        s << std::endl;
    }

private:

    std::string whatIsAt(const Position& p) const
    {
        auto match = std::find_if(m_State.begin(), m_State.end(), [&p](const std::pair<Pod, Position>& item) {
            return item.second == p;
        });

        std::string ret;

        if (match == m_State.end())
        {
            ret += '.';
        }
        else
        {
            if (match->second.isFinal())
            {
                ret += "\033[31m";
                ret += match->first.getName();
                ret += "\033[0m";
            }
            else
            {
                ret += match->first.getName();
            }
        }

        return ret;
    }

};

template<size_t N>
std::ostream& operator<<(std::ostream& s, const PodSpace<N>& t)
{
    t.print(s); return s;
}

int
main(int argc, char **argv)
{

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::array<std::array<char,2>,4> rooms;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^\\s*([#]+)([ABCD])[#]([ABCD])[#]([ABCD])[#]([ABCD])([#]+)\\s*$");;

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                size_t line = (match[1].length() > 1) ? 0 : 1;

                for (auto i=0; i<rooms.size(); ++i)
                {
                    rooms[i][line] = match[i+2].str()[0];
                }
            } 
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    std::packaged_task<std::pair<bool, int64_t>(const std::array<std::array<char,2>,4>&)> part1([](const std::array<std::array<char,2>,4> cfg){
        PodSpace<2> situation(cfg);

        return situation.resolve();
    });

    std::packaged_task<std::pair<bool, int64_t>(const std::array<std::array<char,2>,4>&)> part2([](const std::array<std::array<char,2>,4> cfg){
        std::array<std::array<char,4>,4> rooms2;

        rooms2[0][1] = 'D';
        rooms2[1][1] = 'C';
        rooms2[2][1] = 'B';
        rooms2[3][1] = 'A';

        rooms2[0][2] = 'D';
        rooms2[1][2] = 'B';
        rooms2[2][2] = 'A';
        rooms2[3][2] = 'C';

        for (auto i=0; i<4; ++i)
        {
            rooms2[i][0] = cfg[i][0];
            rooms2[i][3] = cfg[i][1];
        }

        PodSpace<4> situation(rooms2);

        return situation.resolve();
    }); 

    auto resultPart1 = part1.get_future();
    auto resultPart2 = part2.get_future();

    std::thread task1( std::move(part1), rooms );
    std::thread task2( std::move(part2), rooms );

    task1.join();
    auto result1 = resultPart1.get();
    if (result1.first)
    {
        std::cout << "Part 1 lowest cost solution : " << result1.second << std::endl << std::endl;
    }
    else
    {
        std::cout << "Part 1 has NO SOLUTION" << std::endl;
    }

    task2.join();
    auto result2 = resultPart2.get();
    if (result2.first)
    {
        std::cout << "Part 2 lowest cost solution : " << result2.second << std::endl << std::endl;
    }
    else
    {
        std::cout << "Part 2 has NO SOLUTION" << std::endl;
    }    

    return 0;
}
