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

class Crc64
{
    public:

        Crc64() : crc(0)
        {
        }

        void push(size_t b)
        {
            size_t index = (b ^ crc)&0xff;

            crc >>= 8;
            crc ^= table[index];
        }

        operator uint64_t() const { return crc; }
        bool operator==(const Crc64& a_other) const { return crc == a_other.crc; }
        bool operator<(const Crc64& a_other) const { return crc < a_other.crc; }

        Crc64& operator<<(size_t b)
        {
            push(b);
            return *this;
        }

        Crc64& operator<<(int b)
        {
            push((size_t)b);
            return *this;
        }

    private:

        uint64_t crc;

        class Crc64Table
        {
            public:
                Crc64Table()
                {
                    for (uint64_t i=0; i<table.size(); ++i)
                    {
                        uint64_t crc = i;

                        for (uint64_t j=0; j<8; ++j)
                        {
                            if (crc & 0x01)
                            {
                                crc >>= 1;
                                crc ^= poly;
                            }
                            else
                            {
                                crc >>= 1;
                            }
                        }

                        table[i] = crc;

                    }
                }

                uint64_t operator[]( size_t i )
                {
                    return table[i];
                }

            private:

                std::array<uint64_t, 256> table;
                static constexpr uint64_t poly = 0xc96c5795d7870f42;
        };

        static Crc64Table table;
};

Crc64::Crc64Table Crc64::table = Crc64::Crc64Table();

class Game
{
    static constexpr int PLAYERS = 2;

    public:

        /* Create a blank game deck. */
        Game(bool a_verbose = false) :rounds(0), game_count_x(1), verbose(a_verbose)
        {
            game_count = &game_count_x;
            game_index = *game_count;
            _winner = PLAYERS;
            n_cards = 0;
        };

        /* Create a game deck based on another deck */
        Game(const Game& other) : rounds(0), game_count_x(0), game_count(other.game_count), verbose(other.verbose)
        {
            game_index = ++(*game_count);
            n_cards = 0;

            for (size_t i=0; i<stacks.size(); ++i)
            {
                for (auto c = other.stacks[i].begin(); c != other.stacks[i].end(); ++c)
                {
                    stacks[i].push_back(*c);
                }
                n_cards += stacks[i].size();
            }

            _winner = PLAYERS;
        }

        /* Create a sub-game deck, taking into account the give number of cards to take from each players deck. */
        Game(const Game* const other, const std::array<size_t, PLAYERS>& sizes) : rounds(0), game_count_x(0), game_count(other->game_count), verbose(other->verbose)
        {
            game_index = ++(*game_count);
            n_cards = 0;

            for (size_t i=0; i<stacks.size(); ++i)
            {
                size_t j=0;
                for (auto c = other->stacks[i].begin(); (j<sizes[i]) && (c != other->stacks[i].end()); ++c, ++j)
                {
                    stacks[i].push_back(*c);
                }
                n_cards += stacks[i].size();
            }

            _winner = PLAYERS;
        }

        /* reset the current game stats, not touching the decks. */
        void restart()
        {
            game_count_x = 1;
            game_count = &game_count_x;
            game_index = *game_count;
            old_rounds.clear();
        }

        /* add a new card for the specific player. */
        void dealCard(size_t player, size_t card)
        {
            if ((player < 1) || ( player > stacks.size() ))
            {
                throw std::invalid_argument("Too much players");
            }

            stacks[player-1].push_back(card);
            n_cards++;
        }

        /* calculate the score of the current winner.  If now winner has been determined the returned
         * score is 0
         */
        int64_t score()
        {
            if (verbose) print();

            int64_t score = 0;
            if (_winner < stacks.size())
            {
                int64_t value = 1;
                for (auto c = stacks[_winner].rbegin(); c != stacks[_winner].rend(); ++c, ++value)
                {
                    score += ((int64_t)(*c)) * value;
                }
            }

            return score;
        }

        /* find out who is the winner.  Will return PLAYER of no winner has been determined. */
        size_t winner()
        {
            return _winner;
        }

        /* play the game (so run rounds until a winner has been determined) */
        void play(bool recursive = false)
        {
            if (verbose) std::cout << "=== Game " << game_index << " ===" << std::endl;
            while (round(recursive));
        }

        /* play a single round.  Returns false if a winner has been determined */
        bool round(bool recursive = false)
        {
            std::array<size_t, PLAYERS> cards;

            for (size_t i = 0; i<cards.size(); ++i)
            {
                if (stacks[i].empty())
                {
                    return false;
                }
            }

            rounds++;
            if (verbose)
            {
                std::cout << "-- Round " << rounds << " (Game " << game_index << ") --" << std::endl;
                print();
            }

            /* Check if the round has already been played. */
            auto crc = getDeckCrc();
            if (old_rounds.find(crc) != old_rounds.end())
            {
                if (verbose) std::cout << "Loop detected, Player 1 wins" << std::endl;
                _winner = 0;
                return false;
            }

            bool recurse = recursive;
            for (size_t i = 0; i<cards.size(); ++i)
            {
                cards[i] = stacks[i].front();
                stacks[i].pop_front();
                if (stacks[i].size() < cards[i])
                {
                    recurse = false;
                }
                if (verbose) std::cout << "Player " << i+1 << " plays: " << cards[i] << std::endl;
            }

            size_t l_winner = 0;
            if (recurse)
            {
                Game sub(this, cards);

                if (verbose) std::cout << "Playing a sub-game to determine the winner..." <<std::endl;

                sub.play(true);
                l_winner = sub.winner();

                if (verbose) std::cout << "Player " << l_winner+1 << " wins subgame!" << std::endl;

                for (size_t i = 0; i<stacks.size(); ++i)
                {
                    size_t idx = (l_winner + i) % stacks.size();
                    stacks[l_winner].push_back(cards[idx]);
                }
            }
            else
            {
                auto winner_i = std::max_element(cards.begin(), cards.end());
                l_winner = std::distance(cards.begin(), winner_i);

                for (size_t i = 0; i<stacks.size(); ++i)
                {
                    size_t idx = (l_winner + i) % stacks.size();
                    stacks[l_winner].push_back(cards[idx]);
                }
            }

            if (stacks[l_winner].size() == n_cards)
            {
                _winner = l_winner;
            }

            old_rounds.insert(crc);

            if (verbose) std::cout << "Player " << l_winner + 1 << " wins round " << rounds << " of game " << game_index << "!" << std::endl << std::endl;

            return (_winner == stacks.size());
        }

        /* Print the current deck status, does not take into account verbose, this has to be done by the caller */
        void print()
        {
            for (size_t i=0; i<stacks.size(); ++i)
            {
                std::cout << "Player " << i+1 << "'s deck: " ;
                bool first = true;
                for (auto& c: stacks[i])
                {
                    if (! first)
                    {
                        std::cout << ", ";
                    }
                    std::cout << c;
                    first = false;;
                }
                std::cout << std::endl;
            }
        }

        /* Calculate a (hopefully) unique crc of the current deck status */
        Crc64 getDeckCrc()
        {
            Crc64 ret;

            for (size_t i=0; i<stacks.size(); ++i)
            {
                ret << i+1 << stacks[i].size() << 0xffUL;
                for (auto c : stacks[i])
                {
                    ret << (uint64_t)c;
                }
                ret << 0xffUL;
            }

            return ret;
        }

    private:

        /* card stacks for each player */
        std::array<std::deque<size_t>, PLAYERS> stacks;

        /* Number of rounds we have played */
        size_t rounds;

        /* Which game we are at */
        int    game_index;

        /* variable that count the number of games played in this session */
        int    game_count_x;

        /* Points to game_count_x for the the first game in every session */
        int*   game_count;

        /* Whether we want to be verbose */
        bool   verbose;

        /* The index or the winner.  When equal to PLAYERS no winner is known yet */
        size_t _winner;

        /* Number of cards dealt in this game. */
        size_t n_cards;

        /* List of rounds that have been played, to detect loops. */
        std::set<Crc64> old_rounds;
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    Game game(false);

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_h("^Player\\s+([0-9]+):\\s*$");
        const std::regex matchrule_c("^\\s*([0-9]+)\\s*$");

        int player = 0;
        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_h))
            {
                player = std::stoi(match[1].str());
            }
            else if (std::regex_match(line, match, matchrule_c))
            {
                game.dealCard(player, std::stoi(match[1].str()));
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        return -1;
    }

    {
        std::cout << "Playing by normal rules" << std::endl;
        std::cout << "----------------------------------------------------------------------------" << std::endl;
        Game game1(game);
        game1.restart();
        game1.play();

        std::cout << "Score " << game1.score() << std::endl;
    }

    {
        std::cout << std::endl <<"Playing recursive" << std::endl;
        std::cout << "----------------------------------------------------------------------------" << std::endl;
        Game game1(game);
        game1.restart();
        game1.play(true);

        std::cout << "Score " << game1.score() << std::endl;

    }

    return 0;
}
