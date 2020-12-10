#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<std::pair<unsigned, uint64_t>> adapters;

    adapters.emplace_back(0, 0);

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^\\s*([0-9]+)\\s*$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                adapters.emplace_back(std::stoi(match[1].str()), 0);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    std::sort(adapters.begin(), adapters.end());

    adapters.emplace_back((adapters.rbegin()->first) + 3, 1);

    auto i = adapters.begin();
    auto j = i + 1;

    unsigned n_one = 0;
    unsigned n_three = 0;

    for (; j != adapters.end(); j = (++i)+1)
    {
        unsigned delta = j->first - i->first;

        switch (delta)
        {
            case 1:
                n_one++;
                break;
            case 2:
                break;
            case 3:
                n_three++;
                break;
            default:
                std::cout << "Chain doesn't work" << std::endl;
                return 0;
        }
    }

    std::cout << "one-j jumps : " << n_one << std::endl;
    std::cout << "three-j jumps : " << n_three << std::endl;

    std::cout << "answer : " << (n_one * n_three) << std::endl;

    for (auto i = adapters.rbegin(); i != adapters.rend(); ++i)
    {
        for (size_t j=1; ((i+j) != adapters.rend()) && ((i->first - (i+j)->first) <= 3); ++j)
        {
            (i+j)->second += i->second;
        }
    }

    std::cout << "combinations : " << adapters.begin()->second << std::endl;

    return 0;
}
