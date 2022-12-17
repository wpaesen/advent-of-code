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

class Rock
{
public:

    Rock(std::size_t a_y = 0 ) : width(0), y(a_y), lastpush(' ') {}

    std::vector<std::pair<std::size_t, std::size_t>> points;
    std::bitset<7> view;

    std::size_t width;
    std::size_t height;
    std::size_t y;

    char lastpush;

    void push(char c)
    {
        if (c == '<')
        {
            if (! view.test(6))
            {
                lastpush = c;
                view <<= 1;
                for (auto &p : points)
                {
                    p.first -= 1;
                }
            }
        }
        else if (c == '>')
        {
            if (! view.test(0))
            {
                lastpush = c;
                view >>=1 ;
                for (auto &p : points)
                {
                    p.first += 1;
                }
            }
        }
    }

    void unpush()
    {
        if (lastpush == '<')
        {
            push('>');
        }
        else if (lastpush == '>')
        {
            push('<');
        }
        lastpush = ' ';
    }

    bool fall()
    {
        if (y > 0)
        {
            y--;
        
            for (auto &p : points)
            {
                p.second -= 1;
            }
        }

        return y>0;
    }

    static const std::bitset<7> norocks;

    std::size_t getMaxHeight() const
    {
        return y + height;
    }

    std::pair<std::size_t, std::size_t> getLineRange() const
    {
        return std::make_pair(y, y+height-1);
    }

    bool collides(const std::array<std::set<std::size_t>, 7>& cave, int offset = 0) const
    {
        for (auto &p: points)
        {
            if (cave[p.first].find(p.second+offset) != cave[p.first].end())
            {
                return true;
            }
        }
        return false;
    }

    static std::unique_ptr<Rock> createShape(std::size_t i, std::size_t y)
    {
        auto ret = std::make_unique<Rock>(y);

        switch (i%5)
        {
        case 0:
            ret->width = 4;
            ret->height = 1;
            ret->points.emplace_back(2, y);
            ret->points.emplace_back(3, y);
            ret->points.emplace_back(4, y);
            ret->points.emplace_back(5, y);
            break;
        case 1:
            ret->width = 3;
            ret->height = 3;
            ret->points.emplace_back(3, y);
            ret->points.emplace_back(2, y+1);
            ret->points.emplace_back(3, y+1);
            ret->points.emplace_back(4, y+1);
            ret->points.emplace_back(3, y+2);
            break;
        case 2:
            ret->width = 3;
            ret->height = 3;
            ret->points.emplace_back(2, y);
            ret->points.emplace_back(3, y);
            ret->points.emplace_back(4, y);
            ret->points.emplace_back(4, y+1);
            ret->points.emplace_back(4, y+2);
            break;
        case 3:
            ret->width = 1;
            ret->height = 4;
            ret->points.emplace_back(2, y);
            ret->points.emplace_back(2, y+1);
            ret->points.emplace_back(2, y+2);
            ret->points.emplace_back(2, y+3);
            break;
        case 4:
            ret->width = 2;
            ret->height = 2;
            ret->points.emplace_back(2, y);
            ret->points.emplace_back(3, y);
            ret->points.emplace_back(2, y+1);
            ret->points.emplace_back(3, y+1);       
            break;
        default:
            break;
        }

        for (auto &l : ret->points)
        {
            ret->view.set(6-l.first);
        }

        return ret;
    };
};

class Cave
{
public:
    Cave(const std::string& a_jets, std::size_t a_max_rocks) : jets(a_jets), jet_i(0), shape_iter(0), n_rocks(0), height_offset(0), cave_top(0), max_rocks(a_max_rocks), warped(false)
    {    
        cave_max.fill(0);
        for (auto& c : cave)
        {
            c.insert(0);
        }
    };

    char nextJet()
    {
        return jets[(jet_i++)%jets.size()];
    }

    void embedrock()
    {
        if (rock)
        {
            for (auto& p : rock->points)
            {
                cave[p.first].insert(p.second);
                if (cave_max[p.first] < p.second)
                {
                    cave_max[p.first] = p.second;
                }
            }

            n_rocks++;
            rock.reset();

            auto cave_top_elm = std::max_element(cave_max.begin(), cave_max.end());
            cave_top = *cave_top_elm;

            if ((cave_top > 30) && (! warped) && (n_rocks > 2022))
            {  
                /* check if we see a repetition */
                std::tuple<unsigned, unsigned, std::set<std::pair<unsigned, std::size_t>>> sig;
                std::get<0>(sig) = jet_i % jets.size();
                std::get<1>(sig) = shape_iter % 5;

                std::size_t min_height = 0;
                if (cave_top > 30)
                {
                    min_height = cave_top - 30;
                }

                for (std::size_t y = cave_top; y > cave_top-21; --y)
                {
                    for (unsigned x = 0; x<7; ++x)
                    {
                        if (cave[x].find(y) != cave[x].end())
                        {
                            std::get<2>(sig).insert({x, cave_top - y});
                        }
                    }
                }

                auto& loop = loops[sig];
                if (loop.empty())
                {
                    loop.push_back({n_rocks, cave_top});
                }
                else
                {
                    std::size_t delta_rocks = n_rocks - loop.back().first;
                    std::size_t delta_height = cave_top - loop.back().second;

                    loop.push_back({n_rocks, cave_top});

                    // std::cout << "Repetition at " << n_rocks << ", delta_rox=" << delta_rocks << ", delta_height=" << delta_height << std::endl;     

                    /* Figure what the cycle is, and warp the cycle until just before we reach the max_rocks. The number of cycles we have will be added 
                     * as the height offset.
                     */

                    std::size_t needed = max_rocks - n_rocks;
                    std::size_t skip_cycles = needed / delta_rocks;
                    // std::cout << "Skipping " << skip_cycles << " cycles" << std::endl;
                    n_rocks += (skip_cycles * delta_rocks);
                    height_offset += (skip_cycles * delta_height);
                    warped = true;
                }
            }
        }
    }

    void tick()
    {
        if (! rock)
        {
            /* Instead of positioning the rock at 3 positions offset from stack top, 
             * start it at 0 position offset, and handle 3 of the jets at once 
             */
            rock = Rock::createShape(shape_iter++, 1 + cave_top);
            for (std::size_t i=0; i<3; ++i)
                rock->push(nextJet());
        }

        /* Handle jet stream */
        rock->push(nextJet());

        /* Check if the push resulted in horizontal collission.  If so, unpush */
        if (rock->collides(cave))
        {
            rock->unpush();
        }

        /* Check of any of the rock parts hit something
         * that's already in the cave 
         */
        if (rock->collides(cave, -1))
        {
            embedrock();
        }
        else
        {
            /* Fall down */
            rock->fall();
        }
    }

    std::size_t getRocks() const
    {
        return n_rocks;
    }

    std::size_t getPileHeight() const
    {
        return cave_top + height_offset;;
    }

    std::size_t getMaxRocks() const
    {
        return max_rocks;
    }

private:

    std::unique_ptr<Rock> rock;

    std::string jets;
    std::size_t jet_i;

    std::array<std::set<std::size_t>, 7> cave;
    std::array<std::size_t, 7> cave_max;
    std::size_t cave_top;

    std::size_t shape_iter;
    std::size_t n_rocks;
    std::size_t height_offset;

    std::map<std::tuple<unsigned, unsigned, std::set<std::pair<unsigned, std::size_t>>>, std::vector<std::pair<std::size_t, std::size_t>>> loops;

    std::size_t max_rocks;
    bool warped;
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    std::string jets;
    try
    {
        std::ifstream infile(argv[1]);

        std::regex matchrule{"^\\s*([<>]+)\\s*$"};

        std::string line;
        if (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule))
            {
                jets = match[1].str();
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

    if (jets.empty())
    {
        std::cerr << "Syntax error in inputs" << std::endl;
        std::exit(-1);
    }

    Cave cave(jets, 1000000000000UL);

    while (cave.getRocks() < 2022)
    {
        cave.tick();
    }

    std::cout << "Part A : " << std::endl;
    std::cout << "  rocks fallen " << cave.getRocks() << std::endl;
    std::cout << "  pile height  " << cave.getPileHeight() << std::endl;

    while (cave.getRocks() <  cave.getMaxRocks())
    {
        cave.tick();
    }

    std::cout << "Part B : " << std::endl;
    std::cout << "  rocks fallen " << cave.getRocks() << std::endl;
    std::cout << "  pile height  " << cave.getPileHeight() << std::endl;

    return 0;
}
