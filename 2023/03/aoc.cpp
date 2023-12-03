#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
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
#include <thread>
#include <atomic>
#include <list>
#include <future>
#include <functional>
#include <optional>

class Schematic
{
public:
    Schematic() : lines(0), finalized(false){};

    struct Number {
        Number() : value(0), x_min(std::numeric_limits<int>::max()), x_max(std::numeric_limits<int>::min()), y(std::numeric_limits<int>::min())
        {
        }

        std::size_t value;
        int x_min;
        int x_max;
        int y;

        void addDigit(int x, int a_y, char c)
        {
            x_min = std::min(x_min, x);
            x_max = std::max(x_max, x);
            y = a_y;

            value = (value * 10) + (std::size_t)(c - '0');
        }

        std::vector<std::pair<int, int>> box() const
        {
            std::vector<std::pair<int, int>> ret;

            for (int x = x_min - 1; x <= x_max + 1; ++x) {
                ret.emplace_back(std::make_pair(x, y - 1));
                ret.emplace_back(std::make_pair(x, y + 1));
            }
            ret.emplace_back(std::make_pair(x_min - 1, y));
            ret.emplace_back(std::make_pair(x_max + 1, y));

            return ret;
        }
    };

    struct Symbol {
        Symbol(char a_symbol) : symbol(a_symbol)
        {
        }
        Symbol() : symbol('.')
        {
        }

        char symbol;
        std::vector<std::size_t> adjacent_numbers;

        void reset()
        {
            adjacent_numbers.clear();
        }

        void record_number(const Number& n)
        {
            if ((symbol == '*') && (adjacent_numbers.size() < 3)) {
                adjacent_numbers.emplace_back(n.value);
            }
        }

        std::size_t getGearRatio() const
        {
            if (adjacent_numbers.size() == 2) {
                return adjacent_numbers[0] * adjacent_numbers[1];
            }
            return 0;
        }
    };

    void addLine(const std::string& line)
    {
        if (finalized)
            throw std::invalid_argument("Can't add lines after finalized");

        std::optional<Number> number;

        for (int x = 0; x < line.length(); ++x) {
            if (isdigit(line[x])) {
                if (!number.has_value()) {
                    number = Number();
                }
                number.value().addDigit(x, lines, line[x]);

                continue;
            }

            if (number.has_value()) {
                map.emplace_back(number.value());
                number.reset();
            }

            if (line[x] == '.')
                continue;

            symbols.emplace(std::make_pair(x, lines), Symbol(line[x]));
        }

        if (number) {
            map.emplace_back(number.value());
            number.reset();
        }

        lines++;
    }

    std::size_t getValue_a()
    {
        std::size_t ret = 0;
        finalize();
        for (auto& number : map) {
            ret += number.value;
        }

        return ret;
    }

    std::size_t getValue_b()
    {
        std::size_t ret = 0;
        finalize();
        for (auto& s : symbols) {
            ret += s.second.getGearRatio();
        }

        return ret;
    }

private:
    void finalize()
    {
        if (not finalized) {
            /* Remove all numbers that don't have an adjacent symbol */

            for (auto i = map.begin(); i != map.end();) {
                std::vector<decltype(symbols)::iterator> matches;
                for (auto crd : i->box()) {
                    auto loc = symbols.find(crd);
                    if (loc != symbols.end()) {
                        matches.emplace_back(loc);
                    }
                }

                if (!matches.empty()) {
                    for (auto& loc : matches) {
                        loc->second.record_number(*i);
                    }
                    i = std::next(i);
                } else {
                    i = map.erase(i);
                }
            }
            finalized = true;
        }
    }

    std::map<std::pair<int, int>, Symbol> symbols;
    std::vector<Number> map;
    int lines;
    bool finalized;
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    Schematic schematic;
    try {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line)) {
            if (line.empty())
                break;
            schematic.addLine(line);
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    std::cout << "Number sum A " << schematic.getValue_a() << std::endl;
    std::cout << "Ratio sum B " << schematic.getValue_b() << std::endl;

    return 0;
}
