#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <set>
#include <vector>
#include <cstdint>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <iomanip>
#include <list>
#include <chrono>
#include <cmath>
#include <string_view>
#include <tuple>

class Instruction
{
public:

    enum Opcode
    {
        INP,
        ADD,
        MUL,
        DIV,
        MOD,
        EQL,
    };

    enum Register
    {
        W,
        X,
        Y,
        Z
    };

    Instruction(Opcode a_opcode, char arg1, char arg2)
    {
        m_op = a_opcode;

        switch (arg1)
        {
            case 'w': m_arg1 = W; break;
            case 'x': m_arg1 = X; break;
            case 'y': m_arg1 = Y; break;
            case 'z': m_arg1 = Z; break;
            default:
                throw std::invalid_argument("Invalid register 1");
        }

        if (m_op != INP)
        {
            m_arg2_is_literal = false;
            switch (arg2)
            {
                case 'w': m_arg2 = W; break;
                case 'x': m_arg2 = X; break;
                case 'y': m_arg2 = Y; break;
                case 'z': m_arg2 = Z; break;
                default:
                    throw std::invalid_argument("Invalid register 2");
            }
        }

        m_arg2_literal = 0;
    }

    Instruction(Opcode a_opcode, char arg1, int64_t a_arg2)
    {
        if (a_opcode == INP)
        {
            throw std::invalid_argument("Invalid opcode");
        }

        m_op = a_opcode;

        switch (arg1)
        {
            case 'w': m_arg1 = W; break;
            case 'x': m_arg1 = X; break;
            case 'y': m_arg1 = Y; break;
            case 'z': m_arg1 = Z; break;
            default:
                throw std::invalid_argument("Invalid register 1");
        }

        m_arg2_is_literal = true;
        m_arg2_literal = a_arg2;
        m_arg2 = W;
    }

    Opcode   m_op;
    Register m_arg1;

    bool     m_arg2_is_literal;
    int64_t  m_arg2_literal;
    Register m_arg2;

    bool execute( std::array<int64_t, 4>& registers, std::list<int64_t>& input ) const
    {
        int64_t accu = 0;

        int64_t arg2 = m_arg2_literal;
        if (! m_arg2_is_literal)
        {
            arg2 = registers[m_arg2];
        }

        switch (m_op)
        {
        case INP:
            if (input.empty()) 
            {
                std::cerr << "No input" << std::endl;
                return false;
            }

            registers[m_arg1] = input.front();
            input.pop_front();

            break;
        case ADD:
            registers[m_arg1] += arg2;
            break;
        case MUL:
            registers[m_arg1] *= arg2;
            break;
        case DIV:
            if (arg2 == 0)
            {
                std::cerr << "Division by zero" << std::endl;
                return false;
            }

            if (arg2 != 1)                
                registers[m_arg1] /= arg2;
            break;
        case MOD:
            if (registers[m_arg1] < 0)
            {
                std::cerr << "Modulo by zero" << std::endl;
                return false;
            }

            if (arg2 <= 0)
            {
                std::cerr << "Negative modulo" << std::endl;
                return false;
            }

            registers[m_arg1] %= arg2;
            break;
        case EQL:
            if (registers[m_arg1] == arg2)
            {
                registers[m_arg1] = 1;
            }
            else
            {
                registers[m_arg1] = 0;
            }
            break;
        default:
            return false;
        }

        return true;
    }

    void print(std::ostream& s) const
    {
        switch (m_op)
        {
        case INP:
            s << "inp";
            break;
        case ADD:
            s << "add";
            break;
        case MUL:
            s << "mul";
            break;
        case DIV:
            s << "div";
            break;
        case MOD:
            s << "mod";
            break;
        case EQL:
            s << "eql";
            break;
        }

        switch (m_arg1)
        {
        case W: s << " w"; break;
        case X: s << " x"; break;
        case Y: s << " y"; break;
        case Z: s << " z"; break;
        }

        if (m_op != INP)
        {
            if (m_arg2_is_literal)
            {
                s << " " << m_arg2_literal;
            }
            else
            {
                switch (m_arg2)
                {
                case W: s << " w"; break;
                case X: s << " x"; break;
                case Y: s << " y"; break;
                case Z: s << " z"; break;
                }
            }
        }
    }
};

std::ostream& operator<<(std::ostream& s, const Instruction& t)
{
    t.print(s); return s;
}

class Alu
{
public:

    Alu( int64_t a_input )
    {
        registers.fill(0);
        input.push_back( a_input );
        error = false;
    }

    Alu( const std::list<int64_t>& a_input = std::list<int64_t>() )
        : input( a_input )
    {
        registers.fill(0);
        error = false;
    }

    Alu( int64_t a_input, int64_t z )
    {
        registers.fill(0);
        input.push_back( a_input );
        registers[Instruction::Z] = z;
        error = false;
    }

    Alu( int64_t a_input, const Alu& state )
        : registers(state.registers)
    {
        input.push_back( a_input );
        error = false;
    }

    int64_t getZ() const { return registers[Instruction::Z]; };

    bool valid() const { return registers[Instruction::Z] == 0; };
    int64_t getInput( ) const { return *registers.rbegin(); };

    bool execute( const std::vector<Instruction>& il )
    {
        for (auto &i: il)
        {
            error = !i.execute( registers, input );
            if (error) break;
        }
        return !error;
    }

    std::array<int64_t, 4> registers;
    std::list<int64_t> input;
    bool error;
};

class DigitSolver
{
public:

    DigitSolver() {};
    DigitSolver(const std::vector<Instruction>& il) : instructions(il) {};

    std::vector<Instruction> instructions;

    std::map<int64_t, std::set<std::pair<int64_t, int64_t>>> solutions;

    DigitSolver* prev;
    DigitSolver* next;

    int64_t factor;
    bool first;
    int pos;

    std::map<int64_t, std::set<int64_t>> output_universe_largest;
    std::map<int64_t, std::set<int64_t>> output_universe_smallest;


    void solve_for_largest()
    {
        if ((prev) && (prev->prev))
        {
            prev->prev->output_universe_largest.clear();
        }

        if (prev)
        {
            for ( auto d : prev->output_universe_largest )
            {
                for (int i = 1; i<10; ++i)
                {
                    Alu alu( i, d.first );

                    if (alu.execute(instructions))
                    {
                        if ((next) || alu.valid())
                        {
                            output_universe_largest[alu.getZ()].insert( ( i * factor ) + *d.second.rbegin() );
                        }
                    }
                }
            }

        }
        else
        {
            for (int i = 1; i<10; ++i)
            {
                Alu alu( i );

                if (alu.execute(instructions))
                {
                    if ((next) || alu.valid())
                    {
                        output_universe_largest[alu.getZ()].insert( ( i * factor ) );
                    }
                }
            }
        }

        if (output_universe_largest.empty())
        {
            throw std::logic_error("No solution for largest");
        }
    }

    void solve_for_smallest()
    {
        if ((prev) && (prev->prev))
        {
            prev->prev->output_universe_smallest.clear();
        }

        if (prev)
        {
            for ( auto d : prev->output_universe_smallest )
            {
                for (int i = 1; i<10; ++i)
                {
                    Alu alu( i, d.first );

                    if (alu.execute(instructions))
                    {
                        if ((next) || alu.valid())
                        {
                            output_universe_smallest[alu.getZ()].insert( ( i * factor ) + *d.second.begin() );
                        }
                    }
                }
            }

        }
        else
        {
            for (int i = 1; i<10; ++i)
            {
                Alu alu( i );

                if (alu.execute(instructions))
                {
                    if ((next) || alu.valid())
                    {
                        output_universe_smallest[alu.getZ()].insert( ( i * factor ) );
                    }
                }
            }
        }

        if (output_universe_smallest.empty())
        {
            throw std::logic_error("No solution for smallest");
        }
    }

    void clear()
    {
        output_universe_smallest.clear();
        output_universe_largest.clear();
    }

    int64_t getLargestSolution() const
    {
        int64_t largest = std::numeric_limits<int64_t>::min();

        for (auto &s: output_universe_largest)
        {
            if (! s.second.empty())
            {
                largest = std::max( largest, *s.second.rbegin());
            }
        }

        return largest;
    }

    int64_t getSmallestSolution() const
    {
        int64_t smallest = std::numeric_limits<int64_t>::max();

        for (auto &s: output_universe_smallest)
        {
            if (! s.second.empty())
            {
                smallest = std::min( smallest, *s.second.begin());
            }
        }

        return smallest;
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

    std::list<DigitSolver> monad;

    try
    {
        std::ifstream infile(argv[1]);

        const std::array<std::pair<std::pair<Instruction::Opcode, bool>, std::regex>, 11> matchrules{{
            { { Instruction::INP, false }, std::regex("^\\s*inp\\s+([wxyz])\\s*$") },
            { { Instruction::ADD, false }, std::regex("^\\s*add\\s+([wxyz])\\s+([wxyz])\\s*$") },
            { { Instruction::MUL, false }, std::regex("^\\s*mul\\s+([wxyz])\\s+([wxyz])\\s*$") },
            { { Instruction::DIV, false }, std::regex("^\\s*div\\s+([wxyz])\\s+([wxyz])\\s*$") },
            { { Instruction::MOD, false }, std::regex("^\\s*mod\\s+([wxyz])\\s+([wxyz])\\s*$") },
            { { Instruction::EQL, false }, std::regex("^\\s*eql\\s+([wxyz])\\s+([wxyz])\\s*$") },
            { { Instruction::ADD, true  }, std::regex("^\\s*add\\s+([wxyz])\\s+([-+]?[0-9]+)\\s*$") },
            { { Instruction::MUL, true  }, std::regex("^\\s*mul\\s+([wxyz])\\s+([-+]?[0-9]+)\\s*$") },
            { { Instruction::DIV, true  }, std::regex("^\\s*div\\s+([wxyz])\\s+([-+]?[0-9]+)\\s*$") },
            { { Instruction::MOD, true  }, std::regex("^\\s*mod\\s+([wxyz])\\s+([-+]?[0-9]+)\\s*$") },
            { { Instruction::EQL, true  }, std::regex("^\\s*eql\\s+([wxyz])\\s+([-+]?[0-9]+)\\s*$") }
        }};

        std::vector<Instruction> part;
        std::string line;
        while (std::getline(infile, line))
        {
            for ( auto &r: matchrules )
            {
                std::smatch match;
                if (std::regex_match( line, match, r.second))
                {
                    if (r.first.first == Instruction::INP )
                    {
                        if (! part.empty())
                        {
                            monad.push_back(part);
                            part.clear();
                        }

                        part.emplace_back( r.first.first, match[1].str()[0], 'X' );
                    }
                    else if ( r.first.second )
                    {
                        part.emplace_back( r.first.first, match[1].str()[0], (int64_t)std::stoll(match[2].str()));
                    }
                    else
                    {
                        part.emplace_back( r.first.first, match[1].str()[0], match[2].str()[0]);
                    }

                    break;
                }
            }
        }
        if (! part.empty())
        {
            monad.push_back(part);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    if (monad.size() != 14)
    {
        std::cerr << "Syntax error" << std::endl;
        exit(-1);
    }

    /* Print the instruction chart */
    size_t max_lines = 0;
    for (auto &il : monad)
    {
        max_lines = std::max(max_lines, il.instructions.size());
    }

    for (size_t line = 0; line < max_lines; ++line)
    {
        for (auto &il:monad)
        {
            std::string buf;

            if (il.instructions.size() > line)
            {
                std::stringstream sbuf;
                sbuf << il.instructions[line];
                buf = sbuf.str();
            }


            while (buf.length() < 15) buf.append(" ");

            std::cout << buf;
        }

        std::cout << std::endl;

    }

    /* Prepare the data structure linking */
    for (auto i = monad.begin(); i != monad.end(); i = std::next(i))
    {
        if (i == monad.begin())
        {
            i->first = true;
            i->prev = nullptr;
            i->pos = 1;
        }
        else
        {
            i->first = false;
            auto j = std::prev(i);
            i->prev = &*j;
            i->pos = j->pos+1;
        }

        {
            auto j = std::next(i);
            if (j != monad.end())
            {
                i->next = &*j;
            }
            else
            {
                i->next = nullptr;
            }
        }
    }

    int64_t factor = 1;
    for (auto i = monad.rbegin(); i != monad.rend(); i = std::next(i))
    {
        i->factor = factor;
        factor *= 10;
    }

    std::cout << "solving Smallest serial" << std::endl;
    for (auto i = monad.begin(); i != monad.end(); i = std::next(i))
    {
        std::cout << "Digit " << i->pos << std::endl;
        i->solve_for_smallest();
    }

    auto smallestSerial = monad.rbegin()->getSmallestSolution();

    {
        /* Validate */
        std::list<int64_t> input;
        auto x = smallestSerial;
        while (x > 0)
        {
            input.push_front(x%10);
            x = x/10;
        }

        Alu test(input);

        for (auto i = monad.begin(); i != monad.end(); i = std::next(i))
        {
            if (! test.execute(i->instructions))
            {
                throw std::logic_error("Invalid smallest solution 1");
            }
        }

        if (! test.valid())
        {
                throw std::logic_error("Invalid smallest solution 2");            
        }

        std::cout << "Smalles serial " << smallestSerial << std::endl;
    }

    for (auto &d : monad)
    {
        d.clear();
    }

    std::cout << "solving Largest serial" << std::endl;
    for (auto i = monad.begin(); i != monad.end(); i = std::next(i))
    {
        std::cout << "Digit " << i->pos << std::endl;
        i->solve_for_largest();
    }

    auto largestSerial = monad.rbegin()->getLargestSolution();

    {
        /* Validate */
        std::list<int64_t> input;
        auto x = largestSerial;
        while (x > 0)
        {
            input.push_front(x%10);
            x = x/10;
        }

        Alu test(input);

        for (auto i = monad.begin(); i != monad.end(); i = std::next(i))
        {
            if (! test.execute(i->instructions))
            {
                throw std::logic_error("Invalid largest solution 1");
            }
        }

        if (! test.valid())
        {
                throw std::logic_error("Invalid largest solution 2");            
        }

        std::cout << "Largest serial : " << largestSerial << std::endl;
    }

    return 0;
}