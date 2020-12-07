#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>

struct ColorMap
{
    ColorMap() : lastcolor(0) {};

    typedef unsigned Color;

    Color operator[] (const std::string& name)
    {
        auto& r = map[name];
        if (r == 0)
        {
            r = ++lastcolor;
        }

        return r;
    }

    const std::string& operator[] (Color c)
    {
        for (auto i = map.begin(); i != map.end(); ++i)
        {
            if (i->second == c)
            {
                return i->first;
            }
        }

        throw std::logic_error("unknown color");
    }

    std::map<std::string, Color> map;
    Color lastcolor;
};


struct Bag
{
    Bag() {};
    Bag(ColorMap::Color a_color) : color(a_color) {};

    bool canContain(ColorMap::Color c)
    {
        return contents.count(c) > 0;
    }

    ColorMap::Color color;
    std::map<ColorMap::Color, int> contents;

    unsigned getNumBags(std::map<ColorMap::Color, Bag>& rules) const
    {
        unsigned ret = 0;

        for (auto& c : contents)
        {
            const auto &b = rules[c.first];
            ret += c.second * (1 + b.getNumBags(rules));
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

    ColorMap colors;

    auto myColor = colors["shiny gold"];
    std::cout << "My color " << colors[myColor] << std::endl;

    std::map<ColorMap::Color, Bag> rules;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_begin("^(\\w+ \\w+) bags contain (.*)[.]+\\s*$");
        const std::regex matchrule_amount("([0-9]+) (\\w+ \\w+) bag[s]?");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_begin))
            {
                Bag bag(colors[match[1].str()]);

                if ( match[2].str() == std::string("no other bags") )
                {
                    rules[bag.color] = bag;
                }
                else
                {
                    std::string amount = match[2].str();
                    std::smatch amatch;
                    while (std::regex_search(amount, amatch, matchrule_amount))
                    {
                        bag.contents[colors[amatch[2].str()]] = std::stoi(amatch[1].str());
                        amount = amatch.suffix();
                    }
                    rules[bag.color] = bag;
                }
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    std::map<ColorMap::Color, unsigned> colorOptions;

    std::vector<Bag> smuggleoptions;

    /* count direct  */
    for (auto i = rules.begin(); i != rules.end(); i++)
    {
        if ( i->second.canContain( myColor ) )
        {
            smuggleoptions.emplace_back(i->second);
            colorOptions[i->second.color] = 1;
        }
    }

    while (! smuggleoptions.empty())
    {
        std::vector<Bag> newoptions;
        std::map<ColorMap::Color, int> dup;

        for (auto& o : smuggleoptions)
        {
            for ( auto i = rules.begin(); i != rules.end(); i++)
            {
                if ( i->second.canContain( o.color ) )
                {
                    if (dup[i->second.color] == 0)
                    {
                        newoptions.emplace_back(i->second);
                        dup[i->second.color] = 1;
                    }
                }
            }
        }

        for (auto& i : dup)
        {
            colorOptions[i.first] = 1;
        }

        smuggleoptions = newoptions;
    }

    std::cout << "Smuggling has " << colorOptions.size() << " options " << std::endl;

    std::cout << "We need to buy " << rules[myColor].getNumBags(rules) << " bags" << std::endl;

    return 0;
}
