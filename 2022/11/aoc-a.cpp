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

class Monkey
{
public:

    Monkey(int a_id, const std::vector<int64_t>& a_items, std::function<int64_t(int64_t)> a_op, std::function<int(int64_t)> a_test)
        : id(a_id), inspections(0), op(a_op), items(a_items), test(a_test)
    {}

    std::vector<std::pair<int, int64_t>> round()
    {
        std::vector<std::pair<int, int64_t>> ret;

        if (do_debug) std::cout << "Monkey " << id << std::endl;
        for (auto& item : items)
        {
            if (do_debug) std::cout << "  Item with worry level " << item << std::endl;

            auto level = op(item);
            if (do_debug) std::cout << "  new worry level " << level << std::endl;

            level /= 3;
            if (do_debug) std::cout << "  bored worry level " << level << std::endl;

            auto dst = test(level);
            if (do_debug) std::cout << "  toss item with worry level " << level << " to monkey " << dst << std::endl;
            ret.emplace_back(dst, level);
        }

        inspections += items.size();
        items.clear();

        return ret;
    }

    void toss(int64_t item)
    {
        items.push_back(item);
    }

    void print(std::ostream& os)
    {
        os << "Monkey " << id << ": ";
        for (auto& i : items)
        {
            os << i <<", ";
        }
        os << " inspections " << inspections;
    }

    std::size_t countInspections() const { return inspections; };
    std::size_t countItems() const { return items.size(); };

private:

    int id;
    std::size_t inspections;
    std::vector<int64_t> items;
    std::function<int64_t(int64_t)> op;
    std::function<int(int64_t)> test;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Monkey> monkeys;

    if (std::string(argv[1]) == "test")
    {
        monkeys.emplace_back(0, std::vector<int64_t>({79, 98}), 
            [](const int64_t& old) { return old * 19; }, 
            [](const int64_t& value) {
                if ((value % 23) == 0)
                {
                    return 2;
                }
                return 3;
        });

        monkeys.emplace_back(1, std::vector<int64_t>({54, 65, 75, 74}), 
            [](const int64_t& old) { return old + 6; }, 
            [](const int64_t& value) {
                if ((value % 19) == 0)
                {
                    return 2;
                }
                return 0;
        });

        monkeys.emplace_back(2, std::vector<int64_t>({79, 60, 97}), 
            [](const int64_t& old) { return old * old; }, 
            [](const int64_t& value) {
                if ((value % 13) == 0)
                {
                    return 1;
                }
                return 3;
        });


        monkeys.emplace_back(3, std::vector<int64_t>({74}), 
            [](const int64_t& old) { return old + 3; }, 
            [](const int64_t& value) {
                if ((value % 17) == 0)
                {
                    return 0;
                }
                return 1;
        });
    }
    else
    {
        monkeys.emplace_back(0, std::vector<int64_t>({99, 67, 92, 61, 83, 64, 98}), 
            [](const int64_t& old) { return old * 17; }, 
            [](const int64_t& value) {
                if ((value % 3) == 0)
                {
                    return 4;
                }
                return 2;
        });

        monkeys.emplace_back(1, std::vector<int64_t>({78, 74, 88, 89, 50}), 
            [](const int64_t& old) { return old * 11; }, 
            [](const int64_t& value) {
                if ((value % 5) == 0)
                {
                    return 3;
                }
                return 5;
        });

        monkeys.emplace_back(2, std::vector<int64_t>({98, 91}), 
            [](const int64_t& old) { return old + 4; }, 
            [](const int64_t& value) {
                if ((value % 2) == 0)
                {
                    return 6;
                }
                return 4;
        });

        monkeys.emplace_back(3, std::vector<int64_t>({59, 72, 94, 91, 79, 88, 94, 51}), 
            [](const int64_t& old) { return old * old; }, 
            [](const int64_t& value) {
                if ((value % 13) == 0)
                {
                    return 0;
                }
                return 5;
        });
    
        monkeys.emplace_back(4, std::vector<int64_t>({95, 72, 78}), 
            [](const int64_t& old) { return old + 7; }, 
            [](const int64_t& value) {
                if ((value % 11) == 0)
                {
                    return 7;
                }
                return 6;
        });

        monkeys.emplace_back(5, std::vector<int64_t>({76}), 
            [](const int64_t& old) { return old + 8; }, 
            [](const int64_t& value) {
                if ((value % 17) == 0)
                {
                    return 0;
                }
                return 2;
        });

        monkeys.emplace_back(6, std::vector<int64_t>({69, 60, 53, 89, 71, 88}), 
            [](const int64_t& old) { return old + 5; }, 
            [](const int64_t& value) {
                if ((value % 19) == 0)
                {
                    return 7;
                }
                return 1;
        });

        monkeys.emplace_back(7, std::vector<int64_t>({72, 54, 63, 80}), 
            [](const int64_t& old) { return old + 3; }, 
            [](const int64_t& value) {
                if ((value % 7) == 0)
                {
                    return 1;
                }
                return 3;
        });
    }

    std::cout << "==== BEGIN " << std::endl;
    std::size_t total_items = 0;
    for (auto& monkey : monkeys)
    {
        monkey.print(std::cout);
        total_items += monkey.countItems();
        std::cout << std::endl;
    }

    std::cout << total_items << " items total " << std::endl;

    for (std::size_t i=0; i<20; ++i)
    {
        for (auto& monkey : monkeys)
        {
            auto ret = monkey.round();
            for (auto& r : ret)
            {
                monkeys[r.first].toss(r.second);
            }
        }

        if (do_debug)
        {
            for (auto& monkey : monkeys)
            {
                monkey.print(std::cout);
                std::cout << std::endl;
            }
        }
    }

    std::cout << "==== END " << std::endl;
    std::vector<std::size_t> inspections;
    total_items = 0;
    std::size_t total_inspections = 0;
    for (auto& monkey : monkeys)
    {
        monkey.print(std::cout);
        std::cout << std::endl;
        inspections.push_back(monkey.countInspections());
        total_items += monkey.countItems();
        total_inspections += monkey.countInspections();
    }
    std::cout << total_items << " items total " << std::endl;
    std::cout << total_inspections << " inspections total" << std::endl;

    std::sort(inspections.begin(), inspections.end());

    auto n1 = inspections.rbegin();
    auto n2 = std::next(n1);

    std::size_t result = *n1 * *n2;
    std::cout << "Monkey business level (" << *n1 << " * " << *n2 << ") " << result << std::endl;

    return 0;
}
