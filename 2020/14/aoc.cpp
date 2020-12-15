#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>

struct Op
{
    Op() : mask_set(0), mask_unset(0), mask_x(0), address(0), value(0) {};

    static Op create_mask(const std::string& mask)
    {
        Op ret;

        ret.code = MASK;

        for (auto c : mask)
        {
            ret.mask_set <<= 1;
            ret.mask_unset <<= 1;
            ret.mask_x = ret.mask_x <<= 1;

            switch (c)
            {
                case '0':
                    ret.mask_unset.set(0);
                    break;
                case '1':
                    ret.mask_set.set(0);
                    break;
                case 'X':
                    ret.mask_x.set(0);
                    break;
                default:
                    throw std::out_of_range("Input failed");
            }
        }

        for (size_t i=0; i<ret.mask_x.size(); ++i)
        {
            if (ret.mask_x.test(i))
            {
                ret.floatbits.push_back(i);
            }
        }

        return ret;
    }

    static Op create_write(const std::string& location, const std::string& value)
    {
        Op ret;

        ret.code = WRITE;

        ret.address = std::stol(location);
        ret.value = std::stol(value);
        return ret;
    }

    enum Code {
       WRITE,
       MASK
    };

    Code code;

    std::bitset<36> mask_set;
    std::bitset<36> mask_unset;
    std::bitset<36> mask_x;

    uint64_t address;
    uint64_t value;

    std::vector<size_t> floatbits;

    std::vector<uint64_t> permute(uint64_t value)
    {
        /* set all the bits that are in the set mask to 1 */
        value |= mask_set.to_ullong();

        /* set all the bits that are floating to 0 */
        value &= ~mask_x.to_ullong();

        std::vector<uint64_t> ret;

        ret.push_back(value);

        for (size_t i=1; i < (1<<floatbits.size()); ++i)
        {
            uint64_t n = value;

            for (size_t j=0; j<floatbits.size(); ++j)
            {
                if ((i & (1<<j)) != 0)
                {
                    n |= (1UL<<floatbits[j]);
                }
            }

            ret.push_back(n);
        }

        return ret;
    }
};

class Alu
{
    public:
        Alu() {};

        void reset()
        {
            m_Mask = Op();
            m_Memory.clear();
        }

        void exec(const Op& op)
        {
            switch (op.code)
            {
                case Op::WRITE:
                    m_Memory[op.address] = ( op.value | m_Mask.mask_set.to_ullong() ) & ~m_Mask.mask_unset.to_ullong();
                    break;
                case Op::MASK:
                    m_Mask = op;
                    break;
            }

        };

        void exec2(const Op& op)
        {
            switch (op.code)
            {
                case Op::WRITE:
                    for (auto a : m_Mask.permute(op.address))
                    {
                        m_Memory[a] = op.value;
                    }
                    break;
                case Op::MASK:
                    m_Mask = op;
                    break;
            }
        }

        uint64_t sumall()
        {
            uint64_t ret = 0;

            for ( auto& m : m_Memory )
            {
                ret += m.second;
            }

            return ret;
        }

        Op m_Mask;

        std::vector<Op> m_Listing;
        std::map<uint64_t,uint64_t> m_Memory;
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    Alu alu;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_mask("^mask\\s+=\\s+([X01]+)\\s*$");
        const std::regex matchrule_op("^mem\\[([0-9]+)\\]\\s+=\\s+([0-9]+)\\s*$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                alu.m_Listing.emplace_back(Op::create_write(match[1].str(), match[2].str()));
            }
            else if (std::regex_match(line, match, matchrule_mask))
            {
                alu.m_Listing.emplace_back(Op::create_mask(match[1].str()));
            }
            else
            {
                throw std::out_of_range("Input failed 2");
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    alu.reset();
    for (auto& c: alu.m_Listing)
    {
        alu.exec(c);
    }

    std::cout << "1: Sum of all memory is " << alu.sumall() << std::endl;

    alu.reset();
    for (auto& c: alu.m_Listing)
    {
        alu.exec2(c);
    }

    std::cout << "2: Sum of all memory is " << alu.sumall() << std::endl;

    return 0;
}
