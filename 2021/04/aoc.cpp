#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <vector>
#include <cstdint>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <iomanip>

class Board
{
public:

    static constexpr int COLUMNS = 5;
    static constexpr int ROWS = 5;

    static std::vector<std::bitset<COLUMNS*ROWS>> hitlist;

    Board(const std::vector<std::array<int, COLUMNS>>& values) 
        : hitmap()
        , value(0)
    {
        if ( values.size() != map.size())
        {
            throw std::invalid_argument("Invalid map size");
        }

        std::copy( values.cbegin(), values.cend(), map.begin() );
        for ( auto& r : map )
        {
            for (auto &v : r )
            {
                value += v;
            }
        }
    };

    bool draw(int value)
    {
        if (hitmap.all())
        {
            /* Don't process boards were already completed */
            return false;
        }

        auto coords = locate(value);
        if (coords.first == -1)
        {
            /* Number not in board */
            return false;
        }

        hit(coords);

        if (check())
        {
            hitmap.set();
        }

        return hitmap.all();
    }

     int getBoardValue() const { return value; };

private:

    std::array<std::array<int,COLUMNS>,ROWS> map;
    std::bitset<ROWS*COLUMNS> hitmap;
    
    int value;
    
    std::pair<int,int> locate( int value )
    {
        for (int y=0; y<ROWS; ++y)
        {
            for (int x=0; x<COLUMNS; ++x)
            {
                if ( map[y][x] == value)
                {
                    return std::make_pair(y, x);
                }
            }
        }

        return std::make_pair(-1,-1);
    }    

    void hit( const std::pair<int, int>& c)
    {
        int idx = (c.first * COLUMNS ) + c.second;
        hitmap.set(idx);
        value -= map[c.first][c.second];
    }

    bool check()
    {
        if (hitlist.empty())
        {
            /* Initialise hitlist */
            for (int i = 0; i<ROWS; ++i)
            {
                std::bitset<COLUMNS*ROWS> tpl;
                tpl.reset();
                for (int j=0; j<COLUMNS; ++j)
                {
                    tpl.set((i*COLUMNS)+j);
                }
                hitlist.emplace_back(tpl);
            }

            for (int i=0; i<COLUMNS; ++i)
            {
                std::bitset<COLUMNS*ROWS> tpl;
                tpl.reset();
                for (int j=0; j<ROWS; ++j)
                {
                    tpl.set(i+(j*COLUMNS));
                }
                hitlist.emplace_back(tpl);
            }
        }

        for (auto &l : hitlist)
        {
            if ( ( hitmap & l ) == l)
            {
                return true;
            }
        }

        return false;
    }
};

std::vector<std::bitset<Board::COLUMNS*Board::ROWS>> Board::hitlist;

class Game
{
public:
    Game()
    {}

    std::vector<Board> boards;
    std::vector<int> draws;

    std::vector<std::pair<int, int>> winners;

    void setDraw(const std::string& line)
    {
        std::istringstream values{ line };
        std::string item;
        while (std::getline(values, item, ','))
        {
            draws.emplace_back(std::stoi(item));
        }
    }

    void Run()
    {
        for (auto& drawValue : draws)
        {
            iterate(drawValue);
        }
    }

    std::vector<std::pair<std::string, decltype(winners)::const_iterator>> getWinners()
    {
        std::vector<std::pair<std::string, decltype(winners)::const_iterator>> ret;

        if ( ! winners.empty())
        {
            ret.emplace_back( "First winner", winners.cbegin() );
            ret.emplace_back( " Last winner", (winners.crbegin() + 1).base() );
        }

        return ret;
    }

private:

    void iterate(int drawValue )
    {
        boards.erase(
            std::remove_if( boards.begin(), boards.end(), [ &, drawValue ] ( Board& b ) {
                bool ret = b.draw(drawValue);
                if (ret)
                {
                    winners.emplace_back( drawValue,  b.getBoardValue());
                }
                return ret;
            }),
            boards.end()
        );
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

    Game game;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^\\s*([0-9]+)\\s+([0-9]+)\\s+([0-9]+)\\s+([0-9]+)\\s+([0-9]+)\\s*$");
        bool first = true;

        std::vector<std::array<int, 5>> board;

        std::string line;
        while (std::getline(infile, line))
        {
            if (first)
            {
                game.setDraw( line );
                first = false;
            }
            else
            {
                std::smatch match;
                if (std::regex_match(line, match, matchrule_op))
                {
                    std::array<int, 5> row;
                    for (size_t i=0; i<5; ++i)
                    {
                        row[i] = std::stoi(match[i+1].str());
                    }

                    board.push_back( std::move(row));

                    if (board.size() == 5)
                    {
                        game.boards.emplace_back(board);
                        board.clear();
                    }
                }
            }
        }

        if (! board.empty())
        {
            throw std::invalid_argument("Data parsing, incomplete board");
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    game.Run();

    auto winners = game.getWinners();

    if ( winners.empty() )
    {
        std::cout << "NO WINNERS, BAD CODER" << std::endl;
    }
    else
    {
        for (auto &w : winners)
        {
            std::cout << w.first << " : draw=" << w.second->first << ", value=" << w.second->second << ", score=" << w.second->first * w.second->second << std::endl; 
        } 
    }

    return 0;
}
