#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <numeric>
#include <regex>
#include <vector>
#include <cstdint>
#include <fstream>
#include <algorithm>

class CalibrationLine
{
public:
    CalibrationLine(const std::string& a_line) : line(a_line)
    {
    }

    std::size_t getValuePart1() const
    {
        return getValue(line);
    }

    std::size_t getValuePart2() const
    {
        std::string line2(line);

        for (std::string::size_type i = 0; i < line2.size(); ++i) {
            for (auto& d : dict) {
                if (line2.compare(i, d.second.length(), d.second) == 0) {
                    line2[i] = d.first;
                }
            }
        }

        return getValue(line2);
    }

private:
    std::string line;

    std::size_t getValue(const std::string& s) const
    {
        std::array<char, 3> value;
        value.fill('\0');

        for (auto& c : s) {
            if (isdigit(c)) {
                if (value[0] == '\0') {
                    value[0] = c;
                    value[1] = c;
                } else {
                    value[1] = c;
                }
            }
        }

        return std::atoi(value.data());
    }

    static std::array<std::pair<char, std::string>, 9> dict;
};

std::array<std::pair<char, std::string>, 9> CalibrationLine::dict{{
    {'1', "one"},
    {'2', "two"},
    {'3', "three"},
    {'4', "four"},
    {'5', "five"},
    {'6', "six"},
    {'7', "seven"},
    {'8', "eight"},
    {'9', "nine"},
}};

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<CalibrationLine> lines;

    try {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line)) {
            if (!line.empty()) {
                lines.emplace_back(line);
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    static constexpr std::size_t zero{0};

    std::size_t value
        = std::accumulate(lines.begin(), lines.end(), zero, [](std::size_t acc, CalibrationLine& l) -> std::size_t { return acc + l.getValuePart1(); });

    std::cout << "Calibration sum 1 : " << value << std::endl;

    std::size_t value2
        = std::accumulate(lines.begin(), lines.end(), zero, [](std::size_t acc, CalibrationLine& l) -> std::size_t { return acc + l.getValuePart2(); });

    std::cout << "Calibration sum 2 : " << value2 << std::endl;

    return 0;
}
