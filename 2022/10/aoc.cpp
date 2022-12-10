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

constexpr bool do_debug{false};

struct Instruction {

    enum Opcode {
        ADDX,
        NOOP
    };

    Instruction() : opcode(NOOP), argument(0) {};
    Instruction(const std::string& line) 
    {
        if (line.find("addx ") == 0)
        {
            opcode = ADDX;
            argument = std::stol(line.substr(5));
        }
        else if (line.find("noop") == 0)
        {
            opcode = NOOP;
            argument = 0;
        }
        else
        {
            throw std::invalid_argument("syntax error");
        }
    }

    Opcode opcode;
    int64_t argument;
};

class Cpu
{
public:
    Cpu() : accu(1), cycle(1), ss_accu(0), sample_cycle(20) {
        std::string emptyscreen(40, ' ');
        screen.fill(emptyscreen);
    };

    int64_t run(const std::vector<Instruction>& il)
    {
        for (auto& i : il)
        {   
            run(i);
        }

        return ss_accu;
    }

    void print_screen() const
    {
        for (auto& l : screen)
        {
            std::cout << l << std::endl;
        }
    }
    
private:

    void sample()
    {
        if (cycle == sample_cycle)
        {
            if (do_debug)
            {
                std::cout << "cycle=" << cycle << " : X=" << accu << " : signal_strength=" << (accu*cycle) << std::endl;
            }
            ss_accu += (accu * cycle);

            sample_cycle += 40;
        }

        int crt_x = (cycle-1) % 40;
        int crt_y = (cycle-1) / 40;

        if ((accu >= crt_x-1) && (accu <= crt_x + 1))
        {
            screen.at(crt_y).at(crt_x) = '#';
        }
    }

    void run(const Instruction& i)
    {
        sample();

        switch (i.opcode)
        {
        case Instruction::ADDX:
            cycle += 1;
            sample();
            cycle += 1;
            accu += i.argument;
            break;
        case Instruction::NOOP:
            cycle += 1;
            break;
        }
    }

    int64_t accu;
    int64_t cycle;
    int64_t ss_accu;
    int64_t sample_cycle;

    std::array<std::string, 6> screen;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Instruction> program;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            program.emplace_back(line);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    Cpu cpu1;
    std::cout << "Result " << cpu1.run(program) << std::endl;
    cpu1.print_screen();

    return 0;
}
