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

template<std::size_t N>
class Streamer
{
public:

    Streamer() : iter(buffer.begin()) {};

    std::size_t findFirstSOT(const std::string& line)
    {
        for (std::size_t i=1; i<=line.size(); ++i)
        {
            push(line[i-1]);

            if (i < N) continue;
            if (isMarker()) return i;
        }

        return 0;
    }

private:

    bool isMarker() const
    {
        for (auto j = buffer.begin(); j != std::prev(buffer.end()); j = std::next(j))
        {
            for (auto k = std::next(j); k != buffer.end(); k = std::next(k))
            {
                if (*j == *k) return false;
            }
        }

        return true;
    }

    void push(char c)
    {
         *iter = c;
        iter = std::next(iter);
        if (iter == buffer.end())
            iter = buffer.begin();
    }

    std::array<char, N> buffer;
    typename std::array<char, N>::iterator iter;
};



int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::string transmission;
    try
    {
        std::ifstream infile(argv[1]);

        if (!std::getline(infile, transmission))
        {
             std::cerr << "Reading data error" << std::endl;
             std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    std::cout << "First marker at " << Streamer<4>().findFirstSOT(transmission) << std::endl;
    std::cout << "First packet at " << Streamer<14>().findFirstSOT(transmission) << std::endl;

    return 0;
}
