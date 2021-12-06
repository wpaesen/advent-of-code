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
#include "bigint.h"

class MotherFish
{
public:

    using valueType = bigint;

    MotherFish()
    {
        spawn.fill(0);
        state = 0;
        for (size_t i=0;i<9; ++i) children.push_back(0);
    }

    void addMother(int initialstate)
    {
        int position = spawn.size() - initialstate - 1;
        if ( position >= spawn.size())
        {
            throw std::invalid_argument("Invalid initial state");
        }
        spawn[position] += 1;
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

    valueType amount()
    {
        valueType ret = 0;
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

    std::array<valueType, 7> spawn;
    std::list<valueType> children;
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

    MotherFish mothers;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line, ','))
        {
            int value = std::stoi(line);
            mothers.addMother(value);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    for ( size_t i=0; i<=10000; ++i )
    {   
        mothers.tick();

        if ( ( i== 80) || ( i == 256) || ( i == 1000 ) || ( i == 10000 ) )
        {
            std::cout << "After " << i << " days : " << mothers.amount() << std::endl;
        }
    }

    return 0;
}
