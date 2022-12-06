#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <vector>
#include <cstdint>
#include <fstream>
#include <algorithm>

class Elf
{
public:

    Elf() : sum(0)
    {
    } 

    void push(size_t value)
    {
        values.push_back(value);
        sum += value;
    }

    std::size_t total() const { return sum; };

    bool operator<(const Elf& rhs) const {
        return total() < rhs.total();
    }
private:

    std::vector<size_t> values;

    size_t sum;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Elf> elves;
    elves.push_back(Elf());

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            if (line.empty())
            {
                elves.push_back(Elf());
            }
            else
            {
                elves.back().push(std::stoull(line));
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    std::sort(elves.begin(), elves.end());

    std::cout << "Elf with most calories carries " << elves.back().total() << std::endl;

    std::size_t sum = 0;
    auto i = elves.rbegin();
    for (std::size_t n = 0; (n < 3) && (i != elves.rend()); ++n, i = std::next(i))
    {
        sum += i->total();
    }

    std::cout << "3 topmost carry " << su m<< std::endl;

    return 0;
}
