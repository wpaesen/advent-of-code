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

class Stack
{
public:

    enum CraneModel 
    {
        CRATEMOVER_9000,
        CRATEMOVER_9001,
    };

    Stack() {};
    Stack(const Stack& other) : stacks(other.stacks) {};

    struct Instruction
    {
        Instruction() : src(0), dst(0), n(0) {};
        Instruction(const std::string& num, const std::string& from, const std::string& to)
        {
            src = std::stoul(from);
            dst = std::stoul(to);
            n = std::stoul(num);

            if ((src < 1) || (src > 9))
                throw std::invalid_argument("Invalid instruction source");
            if ((dst < 1) || (dst > 9))
                throw std::invalid_argument("Invalid instruction destination");
            if (src == dst)
                throw std::invalid_argument("Invalid instruction destination");                
        }

        std::size_t src;
        std::size_t dst;
        std::size_t n;
    };

    void initStack(const std::string& items)
    {
        std::vector<int> stackindexes;

        /* Each stack index is position on a fixed position.  There can only be 9 stacks */
        std::size_t n_stacks = 0;
        for (std::size_t i = 1; i < items.size(); i += 4)
        {
            int stack_id = (items[i] - '0');
            if ((stack_id < 1) || (stack_id > 9))
                throw std::invalid_argument("Invalid stack configuration");

            stackindexes.push_back(stack_id);
        }

        for (int i=0; i<stackindexes.size(); ++i)
        {
            if ((stackindexes[i] - 1) != i)
                throw std::invalid_argument("Invalid stack configuration");
        }

        stacks.clear();
        stacks.resize(stackindexes.size());
    }

    void addStack(const std::string& items)
    {
        for (std::size_t i=0; i<stacks.size(); ++i)
        {
            std::size_t contents = (i*4)+1;

            if (items.size() >= contents+1)
            {
                if ((items[contents-1] != ' ') || (items[contents] != ' ') || (items[contents+1] != ' '))
                {
                    if ((items[contents-1] != '[') || (items[contents+1] != ']'))
                        throw std::invalid_argument("Invalid stack contents");

                    stacks[i].push_back(items[contents]);
                }
            }
        }
    }

    void runInstruction(const Instruction& i, const CraneModel& model)
    {
        switch (model)
        {
        case CRATEMOVER_9000:
            runInstruction_9000(i);
            break;
        case CRATEMOVER_9001:
            runInstruction_9001(i);
            break;
        default:
            throw std::invalid_argument("Invalid mover type");
            break;
        }
    }

    std::size_t tallestStack() const
    {
        std::size_t tallest_stack = 0;
        for (auto& s : stacks)
        {
            if (tallest_stack < s.size())
                tallest_stack = s.size();
        }

        return tallest_stack;
    }

    std::string getMessage() const
    {
        std::string ret;

        for (auto& s : stacks)
        {
            if (s.empty())
                throw std::invalid_argument("Empty stacks don't produce messages");
            ret += s.back();
        }

        return ret;
    }

    std::ostream& printStack(std::ostream& os) const
    {
        std::size_t levels = tallestStack();

        for (std::size_t level = levels; level > 0; --level)
        {
            std::size_t idx = level - 1;

            std::string prefix = "";
            for (auto& s: stacks)
            {
                if (s.size() > idx)
                {
                    os << prefix << "[" << s[idx] << "]";
                }
                else
                {
                    os << prefix << "   ";
                }

                if (prefix.empty())
                {
                    prefix = " ";
                }
            }

            os << std::endl;
        }

        std::string prefix = "";
        for (std::size_t i=1; i<=stacks.size(); ++i)
        {
            os << prefix << " " << i << " ";
            if (prefix.empty())
                prefix = " ";
        }

        os << std::endl;

        return os;
    }


private:


    void runInstruction_9000(const Instruction& i)
    {
        if ((i.src > stacks.size()) || (i.dst > stacks.size()))
            throw std::invalid_argument("Invalid instruction, index");

        std::deque<char>& src = stacks[i.src-1];
        std::deque<char>& dst = stacks[i.dst-1];

        if (src.size() < i.n)
            throw std::invalid_argument("Invalid instruction, too large");

        for (std::size_t j=0; j<i.n; ++j)
        {
            dst.push_back(src.back());
            src.pop_back();
        }
    }

    void runInstruction_9001(const Instruction& i)
    {
        std::deque<char> tmp;

        if ((i.src > stacks.size()) || (i.dst > stacks.size()))
            throw std::invalid_argument("Invalid instruction, index");

        std::deque<char>& src = stacks[i.src-1];
        std::deque<char>& dst = stacks[i.dst-1];

        if (src.size() < i.n)
            throw std::invalid_argument("Invalid instruction, too large");

        for (std::size_t j=0; j<i.n; ++j)
        {
            tmp.push_front(src.back());
            src.pop_back();
        }

        while (! tmp.empty())
        {
            dst.push_back(tmp.front());
            tmp.pop_front();
        }
    }

    std::vector<std::deque<char>> stacks;
   
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    Stack stack;
    std::vector<Stack::Instruction> instructions;
    try
    {
        std::ifstream infile(argv[1]);

        std::regex instructionmatch{"^\\s*move\\s+([0-9]+)\\s+from\\s+([0-9]+)\\s+to\\s+([0-9]+)\\s*$"};

        std::vector<std::string> stacklines;
        bool stackcompleted = false;

        std::string line;
        while (std::getline(infile, line))
        {
            if (! stackcompleted)
            {
                if (line.empty())
                {
                    stackcompleted = true;
                    for (auto i = stacklines.rbegin(); i !=stacklines.rend(); i = std::next(i))
                    {
                        if (i == stacklines.rbegin())
                        {
                            stack.initStack(*i);
                        }
                        else
                        {
                            stack.addStack(*i);
                        }
                    }
                }
                else
                {
                    stacklines.push_back(line);
                }
            }
            else 
            {
                std::smatch match;
                if (std::regex_match(line, match, instructionmatch))
                {
                    instructions.emplace_back(match[1].str(), match[2].str(), match[3].str());
                }
                else
                {
                    throw std::invalid_argument("Invalid line :" + line);
                }
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    {
        Stack workstack(stack);
        
        std::cout << "===== INIT 9000 =====" << std::endl;
        workstack.printStack(std::cout);
        std::cout << "===== BEGIN 9000 =====" << std::endl;
        for (auto& i : instructions)
        {
            workstack.runInstruction(i, Stack::CRATEMOVER_9000);
        }
        std::cout << "===== END 9000=====" << std::endl;
        workstack.printStack(std::cout);

        std::cout << "MESSAGE 9000 : " << workstack.getMessage() << std::endl;
    }

    {
        Stack workstack(stack);
        
        std::cout << "===== INIT 9001 =====" << std::endl;
        workstack.printStack(std::cout);
        std::cout << "===== BEGIN 9001 =====" << std::endl;
        for (auto& i : instructions)
        {
            workstack.runInstruction(i, Stack::CRATEMOVER_9001);
        }
        std::cout << "===== END 9001=====" << std::endl;
        workstack.printStack(std::cout);

        std::cout << "MESSAGE 9001 : " << workstack.getMessage() << std::endl;
    }
}
