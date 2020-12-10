#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>
#include <algorithm>

#if 0
class ACCU
{
    public:
        ACCU(size_t a_size) : size(a_size), pos(0), accu(0)
        {
            parts.reserve(a_size);
        };

        bool push(unsigned v)
        {
            if (parts.size() < size)
            {
                accu += v;
                parts.push_back(v);
            }
            else
            {
                accu -= parts[pos];
                accu += v;
                parts[pos] = v;

                pos = (pos+1) % parts.size();
            }

            return (parts.size() == size);
        }

        uint64_t GetAccu() { return accu; };

        bool FindCombination(std::function<bool(unsigned, unsigned)> cf)
        {
            for (size_t i=0; i<parts.size()-1; ++i)
            {
                for (size_t j=i+1; j<parts.size(); ++j)
                {
                    if ( cf(parts[i], parts[j]) ) return true;
                }
            }

            return false;
        }

        std::pair<unsigned, unsigned> GetMinMax()
        {
            auto mm = std::minmax_element(std::begin(parts), std::end(parts));
            return std::make_pair(*mm.first, *mm.second);
        }

    private:

        size_t                size;
        size_t                pos;
        uint64_t              accu;

        std::vector<unsigned> parts;
};
#endif

struct SWIP
{
    SWIP(unsigned len, unsigned a_match)
        : pos(0)
        , accu(0)
        , match(a_match)
    {
        parts.assign(len, 0);
    };

    void print_weakness()
    {
        auto mm = std::minmax_element(std::begin(parts), std::end(parts));

        uint64_t sum = *mm.first + *mm.second;

        std::cout << "WEAKNESS = " << *mm.first << "," << *mm.second << " = " << sum  << std::endl;
    }

    bool process(const std::vector<unsigned>& listing)
    {
        size_t i = 0;
        for (; i<parts.size(); ++i)
        {
            accu += listing.at(i);
            parts[i] = listing.at(i);
        }

        if (accu == match)
        {
            print_weakness();
            return true;
        }

        pos = 0;
        for (; i<listing.size(); ++i)
        {
            accu -= parts[pos];
            parts[pos] = listing.at(i);
            accu += parts[pos];
            pos = (pos+1) % parts.size();

            if (accu == match)
            {
                print_weakness();
                return true;
            }
        }

        return false;
    }

    size_t pos;
    std::vector<unsigned> parts;
    uint64_t accu;
    uint64_t match;
};

struct XMAS
{
    XMAS() : stream_error(false), last_value(0), preamble(0) {};

    bool push(unsigned val)
    {
        listing.emplace_back(val);

        if (stream_error)
        {
            return true;
        }

        last_value = val;

        if (preamble >= kernel.size())
        {
            if (! validate(val))
            {
                stream_error = true;
                return false;
            }
        }

        kernel[preamble % kernel.size()] = val;
        preamble++;

        return true;
    }

    bool validate(unsigned val)
    {
        for (size_t i=0; i<kernel.size()-1; ++i)
        {
            for (size_t j=i+1; j<kernel.size(); ++j)
            {
                if ((kernel[i] + kernel[j]) == val) return true;
            }
        }

        return false;
    }

    bool find_set()
    {
        for (size_t i=2; i<listing.size(); ++i)
        {
            SWIP swip(i, last_value);

            if (swip.process(listing))
            {
                return true;
            }
        }

        return false;
    }

    bool stream_error;
    unsigned last_value;
    size_t preamble;

    std::array<unsigned, 25> kernel;

    std::vector<unsigned> listing;
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    XMAS coder;

    try
    {
        std::ifstream infile(argv[1]);

        /* skip empty lines to avoid stray 0 */
        const std::regex matchrule_op("^\\s*$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (! std::regex_match(line, match, matchrule_op))
            {
                if (! coder.push(std::stol(line)))
                {
                    std::cout << "Stream error : " << line << std::endl;
                }
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    coder.find_set();

    return 0;
}
