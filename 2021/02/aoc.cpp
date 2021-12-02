#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <vector>
#include <cstdint>
#include <fstream>
#include <regex>
#include <stdexcept>

class Instruction
{
public:

    enum Direction : char {
        FORWARD = 'f',
        DOWN = 'd',
        UP = 'u'
    };

    struct DirectionParser
    {
        constexpr static std::array<std::pair<char, Direction>, 3> map{{
            { 'f', FORWARD },
            { 'u', UP },
            { 'd', DOWN }
        }};

        DirectionParser( const std::string& s )
        {
            auto dir = std::find_if( map.cbegin(), map.cend(), [ s ]( const std::pair<char, Direction>& i) { return i.first == s[0]; });
            if (dir == map.cend() )
            {
                throw std::invalid_argument("Could not parse direction");
            }

            v = dir->second;
        }

        operator Direction () const { return v; }

        Direction v;
    };

    
    Instruction(const std::string& a_direction, unsigned a_distance)
        : distance(a_distance), direction( DirectionParser( a_direction ))
    {}

    Direction direction;
    unsigned distance;

};

class Diver
{
public:

    Diver() : x(0), y(0) {};

    void Run( const std::vector<Instruction>& listing ) 
    {
        for ( auto& i : listing )
        {
            RunSingle(i);
        }
    }

    int64_t Result() const { return x * -y; };

protected:
    
    virtual void RunSingle( const Instruction& i) = 0;

    int64_t x;
    int64_t y;
};

class DiverSimple : public Diver
{
public:
    DiverSimple() {};

protected:

    void RunSingle( const Instruction& i) override 
    {
        switch (i.direction)
        {
            case Instruction::FORWARD:
                x += i.distance;
                break;
            case Instruction::UP:
                y += i.distance;
                break;
            case Instruction::DOWN:
                y -= i.distance;
                break;
        }
    }
};

class DiverComplex : public Diver
{
public:

    DiverComplex() : aim(0) {};

protected:

    void RunSingle( const Instruction& i) override 
    {
        switch (i.direction)
        {
            case Instruction::FORWARD:
                x += i.distance;
                y += (i.distance * aim);
                break;
            case Instruction::UP:
                aim += i.distance;
                break;
            case Instruction::DOWN:
                aim -= i.distance;
                break;
        }
    }
 
    int64_t aim;
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Instruction> list;  

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^(up|down|forward)[ ]([0-9]+).*$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                list.emplace_back(match[1].str(), std::stoi(match[2].str()));
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    {
        DiverSimple b;

        b.Run( list );

        std::cout << "Simple Vehicle product " << b.Result() << std::endl;
    }

    {
        DiverComplex c;

        c.Run( list );

        std::cout << "Complex Vehicle product " << c.Result() << std::endl;
    }

    return 0;
}
