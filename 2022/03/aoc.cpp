#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <vector>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <bitset>

class ItemHelper
{
public:
    ItemHelper() {};
    virtual ~ItemHelper() {};

protected:

    std::size_t calcItemPrio(const char item) const
    {
        if ((item >= 'a') && (item <= 'z'))
        {
            return 1 + (item - 'a');
        }

        return 27 + (item - 'A');
    }

    std::size_t getLowestPrio(const std::bitset<52>& mask) const
    {
        for (std::size_t i=0; i< mask.size(); ++i)
        {
            if (mask.test(i)) return i+1;
        }

        return 0;
    }
};

class Rucksack : public ItemHelper
{
public:

    Rucksack(const std::string& state)
    {
        if ((state.size() % 2) == 1)
            throw std::invalid_argument("State needs even number of items");

        for (auto& p: pockets)
        {
            p.reset();
        }

        std::size_t i=0;
        for (;i<state.size()/2; ++i)
        {
            addItem(0, state[i]);
        }

        for (;i<state.size(); ++i)
        {
            addItem(1, state[i]);
        }
    }

    std::size_t getMatchingItemPrio() const
    {
        return getLowestPrio(pockets[0] & pockets[1]);
    }

    std::bitset<52> getItemList() const
    {
        return pockets[0] | pockets[1];
    }

private:

    void addItem(std::size_t pocket, const char item)
    {
        pockets[pocket].set(calcItemPrio(item)-1);
    }

    std::array<std::bitset<52>, 2> pockets;
};

class Group : public ItemHelper
{
public:

    Group() { matchlist.set(); };

    void addRucksack(const Rucksack& s)
    {
        matchlist &= s.getItemList();
    }

    std::size_t getMatchingItemPrio() const
    {
        return getLowestPrio(matchlist);
    }

private:

    std::bitset<52> matchlist;
};


template<typename T, std::size_t N>
std::array<T, N>& operator+=(std::array<T, N>& lhs, const std::array<T, N>& rhs)
{
    for (std::size_t i=0; i<N; ++i)
    {
        lhs[i] += rhs[i];
    }
    return lhs;
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Rucksack> rucksacks;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            if (! line.empty())
            {
                rucksacks.emplace_back(line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    std::size_t priosum = 0;
    for (auto& r : rucksacks)
    {
        priosum += r.getMatchingItemPrio();
    }
    std::cout << "Priority sum of matching items " << priosum << std::endl;

    std::vector<Group> groups;
    auto rsi = rucksacks.begin();
    while (rsi != rucksacks.end())
    {
        Group g;

        for (std::size_t i=0; (i<3) && (rsi != rucksacks.end()); ++i, rsi = std::next(rsi))
        {
            g.addRucksack(*rsi);
        }

        groups.emplace_back(std::move(g));
    }

    std::size_t priosum2 = 0;
    for (auto& g : groups)
    {
        priosum2 += g.getMatchingItemPrio();
    }

    std::cout << "Priority sum of grouped items " << priosum2 << std::endl;
}
