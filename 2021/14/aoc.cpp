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

    std::map<char, int64_t> subsitute(const std::string& poly, int iterations)
    {
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

        while (iterations-- > 0)
        {
            pairs = substitute(pairs);
        }

        std::map<char, int64_t> ret;

        ret[*poly.begin()] += 1;
        ret[*poly.rbegin()] += 1;

        for (auto& i : pairs)
        {
            ret[i.first->first] += i.second;
            ret[i.first->second] += i.second;
        }

        for (auto& i: ret)
        {
            i.second /= 2;
        }

        return ret;
    }

    int64_t countScore(const std::string& poly, int iterations)
    {
        auto tally = subsitute(poly, iterations);

        auto mm = std::minmax_element(tally.cbegin(), tally.cend(), [](auto& a, auto& b) {
            return a.second < b.second;
        });

        return mm.second->second - mm.first->second;
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

    std::cout << "10 iterations " << ruleset.countScore(basepoly, 10) << std::endl;
    std::cout << "40 iterations " << ruleset.countScore(basepoly, 40) << std::endl;

    return 0;
}
