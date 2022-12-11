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
#include <cmath>

constexpr bool do_debug{false};

class Monkey
{
public:

    Monkey(int a_id, const std::vector<int64_t>& a_items, std::function<int64_t(int64_t)> a_op, std::function<int(int64_t)> a_test, int64_t a_limiter)
        : id(a_id), inspections(0), op(a_op), items(a_items), test(a_test), limiter(a_limiter)
    {}

    std::vector<std::pair<int, int64_t>> round()
    {
        std::vector<std::pair<int, int64_t>> ret;

        if (do_debug) std::cout << "Monkey " << id << std::endl;
        for (auto& item : items)
        {
            if (do_debug) std::cout << "  Item with worry level " << item << std::endl;

            auto level = op(item) % limiter;
            if (do_debug) std::cout << "  new worry level " << level << std::endl;

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
        os << "Monkey " << id << " inspected items "<< inspections << " times.";
    }

    std::size_t countInspections() const { return inspections; };
    std::size_t countItems() const { return items.size(); };

private:

    int id;
    std::size_t inspections;
    std::vector<int64_t> items;
    std::function<int64_t(int64_t)> op;
    std::function<int(int64_t)> test;
    int64_t limiter;
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

    /* Looking at the test function of each monkey, each monkey performs a modulo test.
     * If we want to keep track of the level in a way that each modulo test still works,
     * we can just keep the number for a modulo that is the multiplilcation of all 
     * module tests for all monkeys.   
     *
     * Eg we want to track the number so we can test for modulo 23 or module 19, then we 
     * can keep track of the value number % (23*19).
     */
    if (std::string(argv[1]) == "test")
    {
        const int64_t limiter = 23*19*13*17;

        monkeys.emplace_back(0, std::vector<int64_t>({79, 98}), 
            [](const int64_t& old) { return old * 19; }, 
            [](const int64_t& value) {
                if ((value % 23) == 0)
                {
                    return 2;
                }
                return 3;
        }, limiter);

        monkeys.emplace_back(1, std::vector<int64_t>({54, 65, 75, 74}), 
            [](const int64_t& old) { return old + 6; }, 
            [](const int64_t& value) {
                if ((value % 19) == 0)
                {
                    return 2;
                }
                return 0;
        }, limiter);

        monkeys.emplace_back(2, std::vector<int64_t>({79, 60, 97}), 
            [](const int64_t& old) { return old * old; }, 
            [](const int64_t& value) {
                if ((value % 13) == 0)
                {
                    return 1;
                }
                return 3;
         }, limiter);


        monkeys.emplace_back(3, std::vector<int64_t>({74}), 
            [](const int64_t& old) { return old + 3; }, 
            [](const int64_t& value) {
                if ((value % 17) == 0)
                {
                    return 0;
                }
                return 1;
        }, limiter);
    }
    else
    {
        const int64_t limiter = 3*5*2*13*11*17*19*7;

        monkeys.emplace_back(0, std::vector<int64_t>({99, 67, 92, 61, 83, 64, 98}), 
            [](const int64_t& old) { return old * 17; }, 
            [](const int64_t& value) {
                if ((value % 3) == 0)
                {
                    return 4;
                }
                return 2;
        }, limiter);

        monkeys.emplace_back(1, std::vector<int64_t>({78, 74, 88, 89, 50}), 
            [](const int64_t& old) { return old * 11; }, 
            [](const int64_t& value) {
                if ((value % 5) == 0)
                {
                    return 3;
                }
                return 5;
        }, limiter);

        monkeys.emplace_back(2, std::vector<int64_t>({98, 91}), 
            [](const int64_t& old) { return old + 4; }, 
            [](const int64_t& value) {
                if ((value % 2) == 0)
                {
                    return 6;
                }
                return 4;
        }, limiter);

        monkeys.emplace_back(3, std::vector<int64_t>({59, 72, 94, 91, 79, 88, 94, 51}), 
            [](const int64_t& old) { return old * old; }, 
            [](const int64_t& value) {
                if ((value % 13) == 0)
                {
                    return 0;
                }
                return 5;
        }, limiter);
    
        monkeys.emplace_back(4, std::vector<int64_t>({95, 72, 78}), 
            [](const int64_t& old) { return old + 7; }, 
            [](const int64_t& value) {
                if ((value % 11) == 0)
                {
                    return 7;
                }
                return 6;
        }, limiter);

        monkeys.emplace_back(5, std::vector<int64_t>({76}), 
            [](const int64_t& old) { return old + 8; }, 
            [](const int64_t& value) {
                if ((value % 17) == 0)
                {
                    return 0;
                }
                return 2;
        }, limiter);

        monkeys.emplace_back(6, std::vector<int64_t>({69, 60, 53, 89, 71, 88}), 
            [](const int64_t& old) { return old + 5; }, 
            [](const int64_t& value) {
                if ((value % 19) == 0)
                {
                    return 7;
                }
                return 1;
        }, limiter);

        monkeys.emplace_back(7, std::vector<int64_t>({72, 54, 63, 80}), 
            [](const int64_t& old) { return old + 3; }, 
            [](const int64_t& value) {
                if ((value % 7) == 0)
                {
                    return 1;
                }
                return 3;
        }, limiter);
    }

    for (std::size_t i=1; i<=10000; ++i)
    {
        for (auto& monkey : monkeys)
        {
            auto ret = monkey.round();
            for (auto& r : ret)
            {
                monkeys[r.first].toss(r.second);
            }
        }

        if ((i == 1) || (i==20) || ((i % 1000) == 0))
        {
            std::cout << "After round " << i << std::endl;
            for (auto& monkey : monkeys)
            {
                monkey.print(std::cout);
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    }

    std::vector<std::size_t> inspections;
    for (auto& monkey : monkeys)
    {
        inspections.push_back(monkey.countInspections());
    }
  
    std::sort(inspections.begin(), inspections.end());

    auto n1 = inspections.rbegin();
    auto n2 = std::next(n1);

    std::size_t result = *n1 * *n2;
    std::cout << std::endl;
    std::cout << "Monkey business level (" << *n1 << " * " << *n2 << ") = " << result << std::endl;

    return 0;
}
