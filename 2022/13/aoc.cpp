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
#include <iomanip>

constexpr bool do_debug{false};

class Item
{
public:
    enum Kind
    {
        VALUE,
        LIST
    };

    Item() : kind(LIST), value(0) {}
    Item(int v) : kind(VALUE), value(v) {};

    void print(std::ostream& os) const;

    enum Decision
    {
        NO_DECISION,
        IN_ORDER,
        REVERSE_ORDER
    };

    Decision compare(const std::unique_ptr<Item>& rhs);

    Kind kind;
    int value;
    std::vector<std::unique_ptr<Item>> list;

    static std::unique_ptr<Item> parse(const std::string& line);
};

Item::Decision Item::compare(const std::unique_ptr<Item>& rhs)
{
    if ((kind == VALUE) && (rhs->kind == VALUE))
    {
        if (value < rhs->value)
        {
            return IN_ORDER;
        }
        else if (value > rhs->value)
        {
            return REVERSE_ORDER;
        }
        else
        {
            return NO_DECISION;
        }
    } else if ((kind == LIST) && (rhs->kind == VALUE))
    {
        /* left list runs out first */
        if (list.empty())
        {
            return IN_ORDER;
        }

        auto cmp1 = list[0]->compare(rhs);
        if (cmp1 != NO_DECISION)
        {
            return cmp1;
        }

        /* right list runs out */
        if (list.size() > 1)
        {
            return REVERSE_ORDER;
        }

        return NO_DECISION;

    } else if ((kind == VALUE) && (rhs->kind == LIST))
    {
        /* right list runs out first */
        if (rhs->list.empty())
        {
            return REVERSE_ORDER;
        }

        auto cmp1 = compare(rhs->list[0]);
        if (cmp1 != NO_DECISION)
        {
            return cmp1;
        }    

        /* left list runs out */
        if (rhs->list.size() > 1)
        {
            return IN_ORDER;
        }

        return NO_DECISION;
    }
    else
    {
        std::size_t i=0;
        for (;i<std::min(list.size(), rhs->list.size()); ++i)
        {
            auto cmp1 = list[i]->compare(rhs->list[i]);
            if (cmp1 != NO_DECISION)
            {
                return cmp1;
            }
        }

        if (list.size() < rhs->list.size())
        {
            return IN_ORDER;
        }
        if (list.size() > rhs->list.size())
        {
            return REVERSE_ORDER;
        }
    }

    return NO_DECISION;
}

std::unique_ptr<Item> Item::parse(const std::string& line)
{
    std::unique_ptr<Item> ret;
    std::deque<Item*> stack;

    if (line.empty())
        throw std::invalid_argument("can't parse empty line");

    if ((line[0] != '[') || ((*line.rbegin()) != ']'))
        throw std::invalid_argument("Unbalanced outer list");

    ret = std::make_unique<Item>();
    stack.push_back(ret.get());

    auto begin = std::next(line.begin());
    auto end = line.end();

    std::string accu;
    while (begin != end)
    {
        if (*begin == '[')
        {
            stack.back()->list.emplace_back(std::make_unique<Item>());
            stack.push_back(stack.back()->list.back().get());
            accu.clear();
        }
        else if (*begin == ']')
        {
            if (stack.size() == 0)
                throw std::invalid_argument("Unbalanced '[' and ']'");

            if (! accu.empty())
            {
                stack.back()->list.emplace_back(std::make_unique<Item>(std::stoi(accu)));
            }

            stack.pop_back();
            accu.clear();
        }
        else if (*begin == ',') 
        {
            if (! accu.empty())
            {
                stack.back()->list.emplace_back(std::make_unique<Item>(std::stoi(accu)));
            }
            accu.clear();
        }
        else
        {
            accu.push_back(*begin);
        }
        begin = std::next(begin);
    }


    return ret;
}

void Item::print(std::ostream& os) const
{
    if (kind == VALUE)
    {
        os << value;
    }
    else
    {
        os << "[";

        std::string sep = "";
        for (auto& i : list)
        {
            os << sep;
            i->print(os);
            if (sep.empty()) sep=",";
        }

        os << "]";
    }
}

std::ostream& operator<<(std::ostream& os, const std::unique_ptr<Item>& item)
{
    item->print(os);
    return os;
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<std::pair<std::unique_ptr<Item>, std::unique_ptr<Item>>> pairs;
    try
    {
        std::ifstream infile(argv[1]);

        std::vector<std::string> lines;

        std::string line;
        while (std::getline(infile, line))
        {
            if (line.empty())
            {
                if (lines.size() == 2)
                {
                    pairs.emplace_back(Item::parse(lines[0]), Item::parse(lines[1]));
                }
                lines.clear();
            }
            else
            {
                lines.push_back(line);
            }
        }

        if (lines.size() == 2)
        {
            pairs.emplace_back(Item::parse(lines[0]), Item::parse(lines[1]));
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    std::size_t idx = 1;
    std::size_t order_sum = 0;
    for (auto& p: pairs)
    {
        auto order = p.first->compare(p.second);

        if (do_debug)
        {
            std::cout << p.first << std::endl;
            std::cout << p.second << std::endl;
            std::cout << std::endl;
        }

        if (order == Item::NO_DECISION)
        {
            std::cout << "no decision" << std::endl;
            std::exit(-1);
        }

        if (order == Item::IN_ORDER)
        {
            order_sum += idx;
        }

        idx++;        
    }

    std::cout << "Order sum " << order_sum << std::endl;

    std::vector<std::unique_ptr<Item>> list;
    for (auto& p : pairs)
    {
        list.emplace_back(std::unique_ptr<Item>(p.first.release()));
        list.emplace_back(std::unique_ptr<Item>(p.second.release()));
    }


    std::vector<const Item*> dividers;

    dividers.push_back(list.insert(list.end(), Item::parse("[[2]]"))->get());
    dividers.push_back(list.insert(list.end(), Item::parse("[[6]]"))->get());

    std::sort(list.begin(), list.end(), [](const std::unique_ptr<Item>& a, const std::unique_ptr<Item>& b) {
        return a->compare(b) == Item::IN_ORDER;
    });

    std::size_t divider = 1;
    idx = 1;
    for (auto &i : list)
    {
        if (do_debug)
        {
            std::cout << i << std::endl;
        }

        if (std::find(dividers.begin(), dividers.end(), i.get()) != dividers.end())
        {
            divider *= idx;
        }

        idx++;
    }

    std::cout << "Divider sum " << divider << std::endl;

    return 0;
}
