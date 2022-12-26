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
#include <thread>
#include <atomic>
#include <list>
#include <future>
#include <functional>
#include <string_view>

static constexpr bool do_debug(false);

class Number
{
public:
    Number(const std::string& _snafu_value) : snafu_value(_snafu_value), decimal_value(0)
    {
        int64_t factor = 1;
        for (auto iter = _snafu_value.rbegin(); iter != _snafu_value.rend(); iter = std::next(iter), factor *= 5)
        {
            int64_t digit_value = 0;
            switch (*iter)
            {
            case '2':
                digit_value = 2;
                break;
            case '1':
                digit_value = 1;
                break;
            case '0':
                digit_value = 0;
                break;
            case '-':
                digit_value = -1;
                break;
            case '=':
                digit_value = -2;
                break;
            default:
                throw std::invalid_argument("Can't recognize symbol " + std::string(1, *iter));
            }

            decimal_value += factor * digit_value;
        }
    };

    Number(int64_t _decimal_value) : snafu_value(""), decimal_value(_decimal_value)
    {
        if (_decimal_value < -2)
            throw std::invalid_argument("Can't represent values below -2 as SNAFU");

        if (_decimal_value == -2)
        {
            snafu_value = "=";
        }
        else if (_decimal_value == -1)
        {
            snafu_value = "-";
        }
        else
        {
            while (_decimal_value > 0)
            {
                int64_t digit = _decimal_value % 5;
                _decimal_value /= 5;

                switch (digit%5)
                {
                case 0:
                    snafu_value.push_back('0');
                    break;
                case 1:
                    snafu_value.push_back('1');
                    break;
                case 2:
                    snafu_value.push_back('2');
                    break;
                case 3:
                    _decimal_value += 1;
                    snafu_value.push_back('=');
                    break;
                case 4:
                    _decimal_value += 1;
                    snafu_value.push_back('-');
                    break;
                }
            } 
        }

        std::reverse(snafu_value.begin(), snafu_value.end());
    }

    const std::string& getSnafuValue() const
    {
        return snafu_value;
    }

    int64_t getDecimalValue() const
    {
        return decimal_value;
    }

private:

    std::string snafu_value;
    int64_t     decimal_value;
};

using namespace std::literals;
static constexpr std::array<std::pair<int64_t, std::string_view>, 15> tests{{
    {         1,              "1"sv },
    {         2,              "2"sv },
    {         3,             "1="sv },
    {         4,             "1-"sv },
    {         5,             "10"sv },
    {         6,             "11"sv },
    {         7,             "12"sv },
    {         8,             "2="sv },
    {         9,             "2-"sv },
    {        10,             "20"sv },
    {        15,            "1=0"sv },
    {        20,            "1-0"sv },
    {      2022,         "1=11-2"sv },
    {     12345,        "1-0---0"sv },
    { 314159265,  "1121-1110-1=0"sv }
}};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    /* run some simple tests */
    for (auto& test : tests)
    {
        Number parsed(std::string(test.second));
        if (parsed.getDecimalValue() != test.first)
        {
            std::cerr << "Could not parse value " << test.second << ", should be " << test.first << " got " << parsed.getDecimalValue() << std::endl;
        }

        Number produced(test.first);
        if (produced.getSnafuValue() != test.second)
        {
            std::cerr << "Could not produce value " << test.first << ", should be " << test.second << " got " << parsed.getSnafuValue() << std::endl;            
        }
    }

    std::vector<Number> numbers;
    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line)) 
        {
            if (line.empty())
                continue;

            numbers.emplace_back(line);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    int64_t sum = 0;
    for (auto& n : numbers)
    {
        {
            Number check(n.getDecimalValue());
            if (check.getSnafuValue() != n.getSnafuValue())
            {
                std::cerr << "Reverse translation validation failed, check your implementation !" << std::endl;
                std::exit(-1);                
            }
        }
        sum += n.getDecimalValue();
    }
   
    Number sumfu(sum);
    { 
        Number sumfutest(sumfu.getSnafuValue());
    
        if (sumfutest.getDecimalValue() != sumfu.getDecimalValue())
        {
            std::cerr << "Reverse translation validation failed, check your implementation !" << std::endl;
            std::exit(-1);  
        }
    }

    std::cout << "sum is " << std::endl;
    std::cout << " - " << sum << std::endl; 
    std::cout << " - " << sumfu.getSnafuValue() <<std::endl;

    return 0;
}
