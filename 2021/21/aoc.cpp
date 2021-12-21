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

class SimpleGame
{
public:
    SimpleGame(unsigned player1, unsigned player2)
        : positions({{player1, player2}})
        , score({{0, 0}})
        , round(0)
    {

    }

    void iterate()
    {
        std::array<unsigned, 2> newscore = score;;

        unsigned dice = 1 + (((round * 2) * 3) % 100);
        for (unsigned i=0; i<2; ++i)
        {
            unsigned steps = (dice + 1 + (i*3)) * 3;

            positions[i] = 1+(positions[i] + steps -1)%10;
            newscore[i] += positions[i];
        }

        if (newscore[0] >= 1000)
        {
            score[0] = newscore[0];
        }
        else if (newscore[1] >= 1000)
        {
            score[1] = newscore[1];
        }
        else
        {
            score = newscore;
        }

        round++;
    }

    unsigned endscore() const
    {
        return ((round * 6)- ((score[0] >= 1000) ? 3 : 0)) * std::min(score[0], score[1]) ;
     }

    std::array<unsigned, 2> positions;
    std::array<unsigned, 2> score;
    
    unsigned round;

    bool hasWinner() const { return (score[0] >= 1000) || (score[1] >= 1000); }
};

class DiracGame
{
public:
    class GameState
    {
    public:
        GameState() : instances(0) {};
       
        GameState(const GameState& other)
            : position(other.position)
            , score(other.score)
            , instances(other.instances)
        {}

        std::array<unsigned, 2> position;
        std::array<unsigned, 2> score;
        uint64_t instances;

        unsigned key() const
        {
            unsigned ret = 0;

            ret |= ((score[0]&0xff) << 24);
            ret |= ((score[1]&0xff) << 16);
            ret |= ((position[0]&0xff) << 8);
            ret |= ((position[1]&0xff)); 

            return ret;
        }

        bool operator<(const GameState& other) const
        {
            return key() < other.key();
        }

        bool operator==(const GameState& other) const
        {
            return key() == other.key();
        }

        bool completed() const
        {
            if (score[0] >= 21) return true;
            if (score[1] >= 21) return true;

            return false;
        }

        GameState cast(unsigned player, unsigned value) const
        {
            GameState ret(*this);

            player = (player-1) & 0x1;

            ret.position[player] = (ret.position[player] + value)%10;
            ret.score[player] += ret.position[player]+1;

            return ret;
        }

        GameState& operator*=( uint64_t a_instances )
        {
            if (instances == 0)
            {
                instances = a_instances;
            }
            else
            {
                instances *= a_instances;
            }
            return *this;
        }

        GameState& operator+=( const GameState& other )
        {
            add( other );
            return *this;
        }

        void add( const GameState& other )
        {
            if (other.key() != key())
            {
                position = other.position;
                score = other.score;
                instances = other.instances;
            }
            else
            {
                instances += other.instances;
            }
        }
    };

    class ScoreBoard
    {
    public:
        ScoreBoard() {};

        ScoreBoard(const ScoreBoard& other)
        {
            states.clear();
            for (auto& s : other.states)
            {
                add(s.second);
            }
        }

        std::map<unsigned, GameState> states;

        void add(const GameState& state)
        {
            auto &val = states[state.key()];
            val += state;
        }

        bool completed() const
        {
            /* Check if there are still rounds to be played */
            for (auto& state : states)
            {
                if (! state.second.completed()) return false;
            }
            return true;
        }

        ScoreBoard& operator=( const ScoreBoard& other )
        {
            states.clear();

            for (auto& s : other.states)
            {
                add(s.second);
            }

            return *this;
        }

        std::pair<uint64_t, uint64_t> countWins() const
        {
            uint64_t player1 = 0;
            uint64_t player2 = 0;

            for (auto& state: states)
            {
                if (state.second.score[0] >= 21)
                {
                    player1 += state.second.instances;
                }
                else if (state.second.score[1] >= 21)
                {
                    player2 += state.second.instances;
                }
            }

            return std::make_pair(player1, player2);
        }
    };

    ScoreBoard state;
    std::map<unsigned, unsigned> casts;

    DiracGame(unsigned player1, unsigned player2)
    {
        for (unsigned i = 0; i<3; ++i)
        {
            for (unsigned j = 0; j<3; ++j)
            {
                for (unsigned k =0; k<3; ++k)
                {
                    casts[(i+1)+(j+1)+(k+1)] += 1;
                }
            }
        }

        GameState initialstate;

        initialstate.position[0] = (player1-1)%10;
        initialstate.position[1] = (player2-1)%10;

        state.add(initialstate);
    }

    bool iterate()
    {
        ScoreBoard next;

        for (auto& state: state.states)
        {
            if (state.second.completed())
            {
                next.add(state.second);
            }
            else
            {
                /* Check all possible casts for player 1 */
                for (auto& cast: casts)
                {
                    GameState turn = state.second.cast( 1, cast.first );
                    turn *= cast.second;

                    if (turn.completed())
                    {
                        next.add(turn);
                    }
                    else
                    {
                        /* Check all possible casts for player 2 */
                        for (auto& cast2: casts)
                        {
                            GameState turn2 = turn.cast( 2, cast2.first );

                            turn2 *= cast2.second;
                            next.add(turn2);
                        }
                    }   
                }
            }
        }

        state = next;

        return ! state.completed();
    }

    uint64_t mostWins() const
    {
        auto w = state.countWins();
        return std::max(w.first, w.second);
    }
};


int
main(int argc, char **argv)
{
    constexpr unsigned player1_startpos = 5;
    constexpr unsigned player2_startpos = 8;
    
    SimpleGame game1(player1_startpos, player2_startpos);

    while (! game1.hasWinner())
    {
        game1.iterate();
    }

    std::cout << "----- Game 1 ------" << std::endl;
    std::cout << "Rounds " << game1.round*6 << std::endl;
    std::cout << "Endscore " << game1.endscore() << std::endl;


    DiracGame game2(player1_startpos, player2_startpos);

    while (game2.iterate());

    std::cout << std::endl << "----- Game 2 ------" << std::endl;
    std::cout << "Most wins " << game2.mostWins() << std::endl;

    return 0;
}
