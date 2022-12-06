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

class Section
{
public:
    Section() : span({0, 0}) {};
    Section(std::size_t a_begin, std::size_t a_end) 
    {
        if (a_begin <= a_end)
        {
            span[0] = a_begin;
            span[1] = a_end;
        }
        else
        {
            span[0] = a_end;
            span[1] = a_begin;
        }
    } 

    std::string asString() const
    {
        return std::to_string(span[0]) + "-" + std::to_string(span[1]);
    }

    bool contains(const Section& rhs) const
    {
        return (span[0] <= rhs.span[0]) && (span[1] >= rhs.span[1]);
    }

    bool intersects(const Section& rhs) const
    {
        if (rhs.span[1] < span[0]) return false;
        if (rhs.span[0] > span[1]) return false;

        return true;
    }

private:
    std::array<std::size_t, 2> span;
};

class ElfPair
{
public:
    ElfPair(std::size_t l_begin, std::size_t l_end, std::size_t r_begin, std::size_t r_end) :
        m_Elves({Section(l_begin, l_end), Section(r_begin, r_end)})
    {}

    std::string asString() const
    {
        return m_Elves[0].asString() + ","  + m_Elves[1].asString();
    }

    bool hasContainment() const
    {
        return m_Elves[0].contains(m_Elves[1]) || m_Elves[1].contains(m_Elves[0]);
    }

    bool hasOverlap() const
    {
        return m_Elves[0].intersects(m_Elves[1]);
    }

private:

    std::array<Section ,2> m_Elves;
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<ElfPair> pairs;

    try
    {
        std::ifstream infile(argv[1]);

        std::regex matchrule{"^\\s*([0-9]+)-([0-9]+),([0-9]+)-([0-9]+)\\s*$"};

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule))
            {
                pairs.emplace_back( std::stoul(match[1].str()), std::stoul(match[2].str()), std::stoul(match[3].str()), std::stoul(match[4].str()));
            }
            else
            {
                throw std::invalid_argument("Invalid line :" + line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    std::size_t pairs_with_containment = 0;
    std::size_t pairs_with_overlap = 0;
    for (auto& p : pairs)
    {
        if (p.hasContainment())
            pairs_with_containment++;
        if (p.hasOverlap())
            pairs_with_overlap++;
    }

    std::cout << pairs_with_containment << " pairs with containment" << std::endl;
    std::cout << pairs_with_overlap << " pairs with overlap" << std::endl;

}
