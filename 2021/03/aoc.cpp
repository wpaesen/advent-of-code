#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <vector>
#include <cstdint>
#include <fstream>
#include <regex>
#include <stdexcept>

class ParsedValue
{
public:

    ParsedValue(const std::string& raw)
    {
        bits = raw.length();
        value = 0;

        for ( auto c : raw )
        {
            bool bitval = (c=='1') ? 1 : 0;
            value = (value << 1) | bitval;
        }
    }

    std::uint64_t value;
    std::size_t   bits;
};

class CounterList
{
public:
    
    enum FilterCriterium : bool {
        MOST_COMMON = true,
        LEAST_COMMON = false
    };

    CounterList() {};

    CounterList( const CounterList& other )
        : values( other.values )
        , bitcount( other.bitcount )
    {
        setupBitMasks();
    }
   
    void push( ParsedValue v )
    {
        if ( bitcount.size() < v.bits )
        {
            bitcount.resize(v.bits);
        }

        values.emplace_back( std::move(v) );
    }

    std::pair<uint64_t, uint64_t> getGammaDelta()
    {
        setupBitMasks();

        for ( auto& v : values )
        {
            for (auto& b : bitcount )
            {
                if (v.value & b.second)
                {
                    b.first += 1;
                }
            }
        }

        std::uint64_t gamma = 0;
        std::uint64_t mask = 0;

        for (auto &b : bitcount)
        {
            gamma = gamma << 1;
            mask = (mask << 1) | 1;

            if ( (b.first*2) >= values.size() )
            {
                gamma |= 1;
            }
        }

        return std::make_pair( gamma, (~gamma) & mask );
    }
   
    ParsedValue filterOxygen()
    {
        return filter( MOST_COMMON );
    }

    ParsedValue filterCO2()
    {
        return filter( LEAST_COMMON );
    }

private:

    std::vector<ParsedValue> values;
    std::vector<std::pair<uint64_t, uint64_t>> bitcount;

    std::size_t countBits( const std::pair<uint64_t, uint64_t>& mask )
    {
        std::size_t count = 0;
        for ( auto& v : values )
        {
            if (v.value & mask.second)
            {
                count += 1;
            }
        }
        return count;
    }


    ParsedValue filter( FilterCriterium criterium )
    {
        for ( auto& b : bitcount )
        {
            if (values.size() == 1) break;

            filter( b, (!criterium) ^ ( ( 2*countBits(b) ) >= values.size() ) );
        }

        if (values.size() != 1)
        {
            throw std::invalid_argument("Could not filter");
        }

        return values[0];
    }

    void filter( const std::pair<uint64_t, uint64_t>& b, bool keepValue )
    {
        values.erase( std::remove_if( values.begin(), values.end(), [ b, keepValue ]( const ParsedValue& v ) {
            return ((v.value & b.second) > 0) ^ keepValue;
        }), values.end() );
    }

    void setupBitMasks()
    {
        uint64_t mask = 1;
        for ( auto i = bitcount.rbegin(); i != bitcount.rend(); ++i )
        {
            i->second = mask;
            i->first = 0;
            mask = mask << 1;
        }
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

    CounterList values;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^([01]+)$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                values.push( std::move( ParsedValue( match[1].str() ) ) );
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    auto result1 = values.getGammaDelta();

    std::cout << "       𝛄 = " << result1.first << std::endl;
    std::cout << "       𝛆 = " << result1.second << std::endl;
    std::cout << "Result A = " << (result1.first * result1.second) << std::endl;

    auto oxygen = CounterList(values).filterOxygen();
    auto co2 = CounterList(values).filterCO2();

    std::cout << "  Oxygen = " << oxygen.value << std::endl;
    std::cout << "     CO₂ = " << co2.value << std::endl;
    std::cout << "Result B = " << (oxygen.value * co2.value) << std::endl;

    return 0;
}
