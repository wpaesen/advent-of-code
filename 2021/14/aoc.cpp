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

class Rule
{
public:
    Rule(char a_first, char a_second, char a_insertion = '-')
        : first( a_first )
        , second( a_second )
        , insertion( a_insertion )
    {
        key = (a_first * 256) + a_second;
        newpairs.fill(nullptr);
    }

    char first;
    char second;
    char insertion;

    unsigned key;

    std::array<Rule*, 2> newpairs;

    int operator< (const Rule& other) const
    {
        return key < other.key;
    }

    bool operator== (const Rule& other) const
    {
        return key == other.key;
    }

    void print() const
    {
        std::cout << first << second << " -> " << insertion << std::endl;
    }

    Rule* ptr() { return this; };

};

class Ruleset
{
public:
    Ruleset()
    {
    }

    std::vector<Rule> rules;

    void addRule(char first, char second, char insert)
    {
        rules.emplace_back(first, second, insert);
    }

    void resolve()
    {
        std::sort(rules.begin(), rules.end());

        for (auto &r : rules)
        {
            auto i = std::find(rules.begin(), rules.end(), Rule( r.first, r.insertion ) );
            if (i != rules.end())
            {
                r.newpairs[0] = &(*i);
            }

            i = std::find(rules.begin(), rules.end(), Rule( r.insertion, r.second ) );
            if (i != rules.end())
            {
                r.newpairs[1] = &(*i);
            }
        }
    }

    std::vector<std::pair<int, int64_t>> countScores(const std::string& poly, const std::vector<int>& a_iterations)
    {
        std::vector<std::pair<int, int64_t>> ret;

        std::vector<int> iterations( a_iterations );
        std::sort(iterations.begin(), iterations.end());

        std::map<Rule*, int64_t> pairs;

        /* Find all pairs */
        for (auto i = poly.cbegin(); (i+1) != poly.cend(); ++i)
        {
            auto k = std::find(rules.begin(), rules.end(), Rule( *i, *(i+1)));
            if (k != rules.end())
            {
                pairs[k->ptr()] += 1;
            }
        }

        int i = 0;
        for (auto& step : iterations )
        {
            while ( i < step )
            {
                pairs = substitute(pairs);
                i++;
            }

            std::map<char, int64_t> tally;

            tally[*poly.begin()] += 1;
            tally[*poly.rbegin()] += 1;

            for (auto& j : pairs)
            {
                tally[j.first->first] += j.second;
                tally[j.first->second] += j.second;
            }

            auto mm = std::minmax_element(tally.cbegin(), tally.cend(), [](auto& a, auto& b) {
                return a.second < b.second;
            });

            ret.emplace_back(i, (mm.second->second - mm.first->second)/2);
        }

        return ret;
    }

private:

    std::map<Rule*, int64_t> substitute(std::map<Rule*, int64_t> pairs)
    {
        std::map<Rule*, int64_t> ret;

        for (auto& c : pairs)
        {
            for (auto& p : c.first->newpairs)
            {
                ret[p] += c.second; 
            }
        }

        return ret;
    }
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::string basepoly;
    Ruleset ruleset;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_tpl("^\\s*([A-Z]+)\\s*$");
        const std::regex matchrule_rule("^\\s*([A-Z])([A-Z])\\s+[-][>]\\s+([A-Z])\\s*$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_tpl))
            {
                basepoly = match[1].str();
            }
            else if (std::regex_match(line, match, matchrule_rule))
            {
                ruleset.addRule(match[1].str()[0], match[2].str()[0], match[3].str()[0]);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    ruleset.resolve();

    auto rets = ruleset.countScores(basepoly, {10, 40});

    for (auto& r : rets)
    {
        std::cout << r.first << " iterations " << r.second << std::endl;
    }

    return 0;
}
