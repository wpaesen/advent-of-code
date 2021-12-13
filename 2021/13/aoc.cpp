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
#include <chrono>
#include <cmath>

class Paper
{
public:
    Paper() {};

    enum FoldDirection
    {
        X,
        Y
    };

    void addDot(int x, int y)
    {
        dots[std::make_pair(x, y)] = true;
    }

    void print()
    {
        /* Find max x and y */
        int max_x = 0;
        int max_y = 0;

        for (auto& p: dots)
        {
            if (p.first.first > max_x) max_x = p.first.first;
            if (p.first.second > max_y) max_y = p.first.second; 
        }

        for (int y=0; y<=max_y; ++y )
        {
            for (int x=0; x<=max_x; ++x)
            {
                auto p = dots.find(std::make_pair(x, y));
                if (p != dots.end())
                {
                    std::cout << "#";
                }
                else
                {
                    std::cout << ".";
                }
            }

            std::cout << std::endl;
        }
    }

    Paper foldAlong(FoldDirection dir, int coord)
    {
        Paper ret;

        for (auto& p : dots)
        {
            int new_x = p.first.first;
            int new_y = p.first.second;

            switch (dir)
            {
            case Y:
                if (new_y > coord)
                {   
                    new_y = (2*coord) - new_y;
                }
                break;
            case X:
                if (new_x > coord)
                {   
                    new_x = (2*coord) - new_x;
                }
                break;
            };

            ret.dots[std::make_pair(new_x, new_y)] = true;
        }

        return ret;
    }

    std::size_t visibleDots() const
    {
        return dots.size();
    }

private:

    

    std::map<std::pair<int, int>, bool> dots;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    Paper paper;
    std::vector<std::pair<Paper::FoldDirection, int>> folds;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_dot("^\\s*([0-9]+),([0-9]+)\\s*$");
        const std::regex matchrule_op("^\\s*fold\\s+along\\s+([xy])=([0-9]+)\\s*$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_dot))
            {
                paper.addDot( std::stoi(match[1].str()), std::stoi(match[2].str()));
            }
            else if (std::regex_match(line, match, matchrule_op))
            {
                if (match[1].str() == "x")
                {
                    folds.emplace_back(Paper::X, std::stoi(match[2].str()));
                }
                else if (match[1].str() == "y")
                {
                    folds.emplace_back(Paper::Y, std::stoi(match[2].str()));
                }
                else
                {
                    throw std::invalid_argument("Unknown fold instruction");
                }
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    bool firstfold = true;
    for (auto l : folds)
    {
        paper = paper.foldAlong(l.first, l.second);

        if (firstfold)
        {        
            std::cout << paper.visibleDots() << " dots visible after first fold" << std::endl;
            firstfold = false;
        }
    }

    std::cout << "Code : " << std::endl;
    paper.print();
    
    return 0;
}
