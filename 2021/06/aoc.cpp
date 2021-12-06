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

    for ( size_t i=0; i<256; ++i )
    {   
        mothers.tick();

        if ( ( ( i+1 ) == 80) || ( ( i+1 ) == 256) )
        {
            std::cout << "After " << i+1 << " days : " << mothers.amount() << std::endl;
        }
    }

    return 0;
}
