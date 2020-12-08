#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>


struct Operation
{
    Operation(const std::string& code, const std::string& argument)
    {
        if (code == "acc")
        {
            oc = OP_ACC;
        }
        else if (code == "jmp")
        {
            oc = OP_JMP;
        }
        else if (code == "nop")
        {
            oc = OP_NOP;
        }
        else
        {
            throw std::out_of_range("Invalid opcode");
        }

        arg = std::stoi(argument);
    }

    enum Opcode {
      OP_ACC,
      OP_JMP,
      OP_NOP
    };

    Opcode oc;
    int    arg;

    static const std::string opcodeName(Opcode oc)
    {
        switch (oc)
        {
            case OP_ACC: return "ACC";
            case OP_JMP: return "JMP";
            case OP_NOP: return "NOP";
            default:     throw std::out_of_range("Invalid opcode");
        }
    }

    static const std::map<Opcode, std::string> opcodes;

    const std::string asString() const
    {
        std::array<char, 512> buf;

        return std::string(buf.data(), std::snprintf(buf.data(), buf.size(), "%s %d", opcodeName(oc).c_str(), arg));
    }
};

struct Interp
{
    Interp() : accu(0) {};

    void reset()
    {
        accu = 0;
    }

    bool run(const std::vector<Operation>& listing)
    {
        std::vector<bool> heatmap(listing.size(), false);

        for (size_t i = 0; i<listing.size();)
        {
            const Operation& op = listing[i];

            if (heatmap[i])
            {
                return false;
            }

            heatmap[i] = true;

            switch(op.oc)
            {
                case Operation::OP_ACC:
                    accu += op.arg;
                    i++;
                    break;
                case Operation::OP_JMP:
                    i += op.arg;
                    break;
                case Operation::OP_NOP:
                    i++;
                    break;
            };

        }

        return true;

    };

    int accu;
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Operation> listing;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^(\\w+)\\s* ([-+]?[0-9]+)\\s*$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                listing.emplace_back(match[1].str(), match[2].str());
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    Interp cpu;
    if (! cpu.run(listing))
    {
        std::cout << "Loop detected, accu value " << cpu.accu << std::endl;
    }
    else
    {
        std::cerr << "No loop detected, accu value " << cpu.accu << std::endl;
        return 0;
    }


    for (auto &o : listing)
    {
        cpu.reset();
        switch (o.oc)
        {
            case Operation::OP_JMP:
                /* Change to oc to OC_NOP and run the program */
                o.oc = Operation::OP_NOP;
                if (cpu.run(listing))
                {
                    std::cout << "Fix accu = " << cpu.accu << std::endl;
                }
                o.oc = Operation::OP_JMP;
                break;

            case Operation::OP_NOP:
                o.oc = Operation::OP_JMP;
                if (cpu.run(listing))
                {
                    std::cout << "Fix accu = " << cpu.accu << std::endl;
                }
                o.oc = Operation::OP_NOP;
                break;
            default:
                break;
        }
    }

    return 0;
}
