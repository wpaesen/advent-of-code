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
#include <list>

class MotherFish
{
public:
    MotherFish(int a_state = 0)
    {
        if ( state >= spawn.size())
        {
            throw std::invalid_argument("Invalid initial state");
        }

        spawn.fill(0);
        spawn[0] = 1;
        state = a_state;
        for (size_t i=0;i<9; ++i) children.push_back(0);
    }

    void addMother()
    {
        spawn[0] += 1;
    }

    void tick()
    {
        spawn[state] += children.front();
        children.pop_front();        
        children.push_back( spawn[state] );

        state -= 1;
        if (state < 0)
        {
            state = 6;
        }
    }

    int64_t amount()
    {
        int64_t ret = 0;
        for (auto &sp : spawn)
        {
            ret += sp;
        }
        for (auto &sp : children)
        {
            ret += sp;
        }
        return ret;
    }

private:

    std::array<int64_t, 7> spawn;
    std::list<int64_t> children;
    int state; 
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::map<int, MotherFish> mothers;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line, ','))
        {
            int value = std::stoi(line);

            auto j = mothers.find(value);
            if (j == mothers.end())
            {
                mothers[value] = MotherFish(value);
            }
            else
            {
                j->second.addMother();
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    if (mothers.empty())
    {
        std::cerr << "Reading data error : NO DATA" << std::endl;
    }

    for ( size_t i=0; i<256; ++i )
    {   
        for (auto& f : mothers)
        {
            f.second.tick();
        }

        if ( ( ( i+1 ) == 80) || ( ( i+1 ) == 256) )
        {
            int64_t amount = 0;
            for (auto& f : mothers)
            {
                amount += f.second.amount();
            }

            std::cout << "After " << i+1 << " days : " << amount << std::endl;
        }
    }

    return 0;
}
