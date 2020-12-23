#include <string>
#include <array>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include <set>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <future>
#include <forward_list>
#include <list>

class Game
{
    template<typename T>
    auto wrap( const T& i, const T& first, const T& end )
    {
        if (i == end)
        {
            return first;
        }
        else
        {
            return i;
        }
    }

    template<typename T>
    auto nextwrap( const T& i, const T& first, const T& end)
    {
        return wrap(std::next(i), first, end);
    }

    public:
        Game(const std::string& input, bool extended = false) : nround(0)
        {
            max_value = 0;
            for (auto c: input)
            {
                std::string val_s;
                val_s += c;

                int val = std::stoi(val_s);
                cups.push_back(val);
                max_value = std::max(max_value, val);
            }

            if (extended)
            {
                int i = max_value;
                for (; i< 1000000; ++i)
                {
                    cups.push_back(i+1);
                }
                max_value = i;
            }

            values.resize(max_value+1);
            for (auto i = cups.begin(); i != cups.end(); ++i)
            {
                values[*i] = i;
            }

            current = cups.begin();
        }

        std::string result2()
        {
            std::string res;

            auto i = std::find(cups.begin(), cups.end(), 1);

            auto a1 = nextwrap(i, cups.begin(), cups.end());
            auto a2 = nextwrap(a1, cups.begin(), cups.end());

            int64_t v1 = *a1;
            int64_t v2 = *a2;

            res += "1st star: " + std::to_string(v1);
            res += ", 2nd star: " + std::to_string(v2);
            res += ", answer: " + std::to_string(v1*v2);

            return res;
        }

        std::string result()
        {
            std::string res;

            auto i = std::find(cups.begin(), cups.end(), 1);

            for (;;)
            {
                i = nextwrap(i, cups.begin(), cups.end() );

                if ((*i) == 1)
                    break;

                res += std::to_string(*i);
            }

            return res;
        }

        void print()
        {
            std::cout << "cups: ";
            for (auto x = cups.begin(); x != cups.end(); ++x)
            {
                if (x == current)
                {
                    std::cout << "(" << *x << ")";
                }
                else
                {
                    std::cout << " " << *x << " ";
                }
            }
            std::cout << std::endl;
        }


        std::list<int>::iterator find_dst(int current_value,const std::vector<int>& excludes)
        {
            bool found = false;
            for (;!found;)
            {
                current_value -= 1;
                if (current_value == 0)
                {
                    current_value = max_value;
                }
                found = (std::find(excludes.begin(), excludes.end(), current_value) == excludes.end());
            }

            return nextwrap(values[current_value], cups.begin(), cups.end());
        }

        size_t round()
        {
            ++nround;

            std::vector<int> pick;
            auto i = nextwrap(current, cups.begin(), cups.end());
            while (pick.size() < 3)
            {
                pick.push_back(*i);
                i = wrap(cups.erase(i), cups.begin(), cups.end());
            }

            auto k = cups.insert(find_dst(*current, pick), pick.begin(), pick.end());
            for (size_t i=0; i<3; ++i, ++k)
            {
                values[*k] = k;
            }

            current = nextwrap(current, cups.begin(), cups.end());

            return nround;
        }

    private:

        std::list<int> cups;
        std::list<int>::iterator current;
        std::vector<std::list<int>::iterator> values;

        size_t nround;
        int max_value;
};

int
main(int argc, char **argv)
{
    bool runexamples = false;

    if (argc >= 2)
    {
        if (std::string(argv[1]) == "--test")
        {
            std::cout << "Running example test cases" << std::endl;
            runexamples = true;
        }
    }


    /* Assignent a, Example  */
    if (runexamples)
    {
        std::cout << "Part a, example" << std::endl;
        Game game("389125467");   /* sample input */

        std::cout << "- Ready to start" << std::endl;

        while (game.round() < 10L);

        std::string res = game.result();
        std::cout << "Result after 10  : " << res;
        if (res != "92658374")
        {
            std::cout << " (FAIL) " << std::endl;
            return 0;
        }

        std::cout << " (OK)" << std::endl;

        while (game.round() < 100L);
        res = game.result();
        std::cout << "Result after 100 : " << res;
        if (res != "67384529")
        {
            std::cout << " (FAIL) " << std::endl;
            return 0;
        }

        std::cout << " (OK)" << std::endl;
    }
    else
    {
        std::cout << "Part a, assignment" << std::endl;

        Game game("487912365");
        while (game.round() < 100);

        std::cout << "Result after 100 : " << game.result() << std::endl;
    }

    if (runexamples)
    {
        std::cout << "Part b, example" << std::endl;

        Game game("389125467", true);   /* sample input */

        while (game.round() < 10000000L);

        std::string res = game.result2();
        std::string expected = "1st star: 934001, 2nd star: 159792, answer: 149245887792";

        std::cout << "Result : " << game.result2();
        if (res != expected)
        {
            std::cout << " (FAIL) " << std::endl;
            return 0;
        }


        std::cout << " (OK) " << std::endl;
    }
    else
    {
        std::cout << "Part b, assignment" << std::endl;

        Game game("487912365", true);

        while (game.round() < 10000000L);

        std::cout << "Result : " << game.result2() << std::endl;;
    }

    return 0;
}
