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

class DisplayValue
{
public:
    DisplayValue(const std::string& a_segments)
    {
        segments.reset();
        for (auto &s: a_segments)
        {
            int k = s - 'a';
            segments.set(k);
        }
    }

    DisplayValue() {};

    std::bitset<7> segments;

    bool isEasyDigit() const
    {
        constexpr std::array<int,4> set{{2, 3, 4, 7}};
        return std::any_of(set.cbegin(), set.cend(), [&](int value) { return value == segments.count();} );
    }

    std::size_t length() const 
    { 
        return segments.count(); 
    }

    bool operator<( const DisplayValue& other ) const
    {
        return segments.to_ulong() < other.segments.to_ulong();
    }

    bool operator==( const DisplayValue& other ) const
    {
        return other.segments == segments;
    }
};

class Event
{
public:
    Event(const std::string& line)
    {
        bool in_outputs = false;
        
        std::vector<DisplayValue> combinations;
        std::map<int, DisplayValue> digits;

        std::istringstream values{ line };
        std::string item;
        while (std::getline(values, item, ' '))
        {
            if (item[0] == '|')
            {
                in_outputs = true;
            }
            else if (in_outputs)
            {
                outputs.emplace_back(item);
            }
            else
            {
                DisplayValue  value(item);

                if ( value.length() == 2 )
                {
                    digits[1] = value;
                }
                else if ( value.length() == 3)
                {
                    digits[7] = value;
                }
                else if ( value.length() == 4)
                {
                    digits[4] = value;
                }
                else if ( value.length() == 7 )
                {
                    digits[8] = value;
                }
                else
                {
                    combinations.push_back(value);
                }
            }
        }

        /* Deduce the remaining digit mapping 
              0: 6    1: 2    2: 5    3: 5    4: 4
               aaaa    ....    aaaa    aaaa    ....
              b    c  .    c  .    c  .    c  b    c
              b    c  .    c  .    c  .    c  b    c
               ....    ....    dddd    dddd    dddd
              e    f  .    f  e    .  .    f  .    f
              e    f  .    f  e    .  .    f  .    f
               gggg    ....    gggg    gggg    ....

              5: 5    6: 6    7: 3    8: 7    9: 6
               aaaa    aaaa    aaaa    aaaa    aaaa
              b    .  b    .  .    c  b    c  b    c
              b    .  b    .  .    c  b    c  b    c
               dddd    dddd    ....    dddd    dddd
              .    f  e    f  .    f  e    f  .    f
              .    f  e    f  .    f  e    f  .    f
               gggg    gggg    ....    gggg    gggg

              2 segments on : 1
              3 segments on : 7
              4 segments on : 4
              5 segments on : 2, 3, 5
              6 segments on : 0, 6, 9
              7 segments on : 8

        */

        /* A 5 segment rendering which has the same segments on as 1 is digit 3 */
        {
            auto i = std::find_if( combinations.begin(), combinations.end(), [ & ] ( const DisplayValue& v ) {
                return ( v.length() == 5 ) && (( v.segments & digits[1].segments) == digits[1].segments);
            });

            if ( i == combinations.end() )
            {
                throw std::invalid_argument("No solution for 3");
            }

            digits[3] = *i;
            combinations.erase(i);
        }

        /* A 6 segment rendering which doesn't have the same segements on as 1 is 6 */
        {
            auto i = std::find_if( combinations.begin(), combinations.end(), [ & ] ( const DisplayValue& v ) {
                return ( v.length() == 6 ) && (( v.segments & digits[1].segments) != digits[1].segments);
            });

            if ( i == combinations.end() )
            {
                throw std::invalid_argument("No solution for 6");
            }

            digits[6] = *i;
            combinations.erase(i);
        }

        /* A 6 segment rendering which has the same segments on as 4 is 9 */
        {
            auto i = std::find_if( combinations.begin(), combinations.end(), [ & ] ( const DisplayValue& v ) {
                return ( v.length() == 6 ) && (( v.segments & digits[4].segments) == digits[4].segments);
            });

            if ( i == combinations.end() )
            {
                throw std::invalid_argument("No solution for 9");
            }

            digits[9] = *i;
            combinations.erase(i);
        }

        /* The remaining 6 segment rendering is 0 */
        {
            auto i = std::find_if( combinations.begin(), combinations.end(), [ & ] ( const DisplayValue& v ) {
                return ( v.length() == 6 );
            });

            if ( i == combinations.end() )
            {
                throw std::invalid_argument("No solution for 0");
            }

            digits[0] = *i;
            combinations.erase(i);
        }

        /* The 5 segment rendering which has the same segments on as 6 is 5 */
        {
            auto i = std::find_if( combinations.begin(), combinations.end(), [ & ] ( const DisplayValue& v ) {
                return ( v.length() == 5 ) && (( v.segments & digits[6].segments) == v.segments);
            });

            if ( i == combinations.end() )
            {
                throw std::invalid_argument("No solution for 5");
            }

            digits[5] = *i;
            combinations.erase(i);
        }

        /* The remaining segment is 2 */
        if (combinations.size() != 1)
        {
            throw std::invalid_argument("No solution for 2");
        }

        digits[2] = *combinations.begin();

        for (auto& c : digits)
        {
            translation[c.second] = c.first;
        }
    }

    Event()
    {}

    
    std::vector<DisplayValue>   outputs;

    
    std::map<DisplayValue, int> translation;

    int getEasyOutputs()
    {
        int ret = 0;
        for (auto& c : outputs)
        {   
            ret += c.isEasyDigit() ? 1 : 0;
        }
        return ret;
    }

    int getOutputValue()
    {
        int result = 0;
        for (auto& i : outputs)
        {
            result = (result* 10) + translation[i];
        }

        return result;
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

    std::vector<Event> events;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            events.emplace_back(line);
        }        
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    int easyOutputs = 0;
    for (auto& e : events)
    {
        easyOutputs += e.getEasyOutputs();
    }

    std::cout << "Number of easy digits in output : " << easyOutputs << std::endl;

    int64_t sum = 0;
    for (auto& e : events)
    {
        sum += e.getOutputValue();
    }

    std::cout << "Sum of all outputs : " << sum << std::endl;

    return 0;
}
