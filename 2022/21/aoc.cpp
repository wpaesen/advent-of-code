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

class Node
{
public:

    enum HumanState : std::size_t {
        OPERAND_A = 0,
        OPERAND_B = 1,
        ME,
        NOT_ME,
        UNKNOWN
    };

    Node(const std::string& a_name, const std::string& a_operand_a, char a_operation, const std::string& a_operand_b)
        : name(a_name), key(makeKey(a_name)), operation(a_operation), solved(false), value(0), operand_names({a_operand_a, a_operand_b}), human(UNKNOWN)
    {
        switch (a_operation)
        {
        case '-':
        case '*':
        case '/':
        case '+':
            break;
        default:
            throw std::invalid_argument("Invalid operation");
        }

        for (std::size_t i=0; i<operand_names.size(); ++i)
        {
            operands[i].first = makeKey(operand_names[i]);
            operands[i].second = nullptr;
        }

    }

    Node(const std::string& a_name, int64_t value)
        : name(a_name), key(makeKey(a_name)), operation('='), solved(true), value(value), human(NOT_ME)
    {
        operand_names[0] = "";
        operand_names[1] = "";

        operands[0].first = 0;
        operands[0].second = nullptr;
        operands[1].first = 0;
        operands[1].second = nullptr;

        if (a_name == "humn")
        {
            human = ME;
        }
    }

    uint32_t getKey() const { return key; }
    const std::string& getName() const { return name; }

    bool isSolved() const { return solved; }
    bool isLinked() const { 
        return (operation == '=') || ((operands[0].second != nullptr) && (operands[1].second != nullptr));
    }

    bool isHumanKnown() const {
        return human != UNKNOWN;
    }

    bool leadsToHuman() const {
        return (human == ME) || (human == OPERAND_A) || (human == OPERAND_B);
    }

    bool tryLink(std::map<uint32_t, std::shared_ptr<Node>>& resolved_nodes)
    {
        bool changed = false;
        for (auto &operand : operands)
        {
            if (operand.second == nullptr)
            {
                auto r = resolved_nodes.find(operand.first);
                if (r != resolved_nodes.end())
                {
                    operand.second = r->second.get();
                    changed = true;
                }
            }
        }

        return isLinked();
    }

    bool trySolve()
    {
        if (! isSolved())
        {
            std::array<int64_t, 2> values;

            for (std::size_t i=0; i<operands.size(); ++i)
            {
                if (! operands[i].second->isSolved()) return false;
                values[i] = operands[i].second->value;
            }

            switch (operation)
            {
            case '+':
                value = values[0] + values[1];
                solved = true;
                break;
            case '/':
                value = values[0] / values[1];
                solved = true;
                break;
            case '-':
                value = values[0] - values[1];
                solved = true;
                break;
            case '*':
                value = values[0] * values[1];
                solved = true;
                break;
            default:
                break;
            }
        }

        return isSolved();
    }

    bool tryFindHuman()
    {
        if ((! isHumanKnown()) && isSolved())
        {
            if (operands[0].second->isHumanKnown() && operands[0].second->leadsToHuman())
            {
                human = OPERAND_A;
            }

            if (operands[1].second->isHumanKnown() && operands[1].second->leadsToHuman())
            {
                human = OPERAND_B;
            }
        }

        return isHumanKnown();
    }

    void Print(std::ostream& os) const
    {
        os << name << ": ";

        if (operation == '=')
        {
            os << value;
            if (human == ME)
            {
                os << " H ";
            }
        } else 
        {
            os << operand_names[0];
            if (operands[0].second)
            {
                if (operands[0].second->leadsToHuman())
                {
                    os << "[H]";
                } else
                {
                    os << "[*]";
                }
            }
            else
            {
                os << "   ";
            }
        
            os << " " << operation << " ";
            os << operand_names[1];
            if (operands[1].second)
            {
                if (operands[1].second->leadsToHuman())
                {
                   os << "[H]";
                } else
                {
                    os << "[*]";
                }
            }
            else
            {
                os << "   ";
            }

            if (isSolved())
            {
                os << " = " << value;
            }
        }
    }

    int64_t getValue() const
    {
        return value;
    }

    int64_t getHumanValue() const
    {
        if ( human == ME )
        {
            return value;
        }
        else if ( human == OPERAND_A )
        {
            return operands[0].second->getHumanValue( operands[1].second->getValue() );
        }
        else if ( human == OPERAND_B )
        {
            return operands[1].second->getHumanValue( operands[0].second->getValue() );
        }

        throw std::runtime_error("Can't find human value in path which doesn't lead to human");

        return -1;
    }

private:

    std::string name;
    uint32_t key;
    char operation;

    bool solved;
    int64_t value;
    
    std::array<std::pair<uint32_t, Node*>, 2> operands;
    std::array<std::string, 2> operand_names;

    HumanState human;

    static uint32_t makeKey(const std::string& name)
    {
        if (name.size() != 4)
            throw std::invalid_argument("Can not encode name");

        return *reinterpret_cast<const uint32_t *>(name.data());
    }

    int64_t getHumanValue(int64_t value) const
    {
        if ( human == ME )
        {
            return value;
        }

        if ( human == OPERAND_A )
        {
            switch (operation)
            {
            case '+':
                // value = a + b -> a = value - b
                return operands[0].second->getHumanValue( value - operands[1].second->getValue() );
            case '-':
                // value = a - b  -> a = value + b
                return operands[0].second->getHumanValue( value + operands[1].second->getValue() );
            case '*':
                // value = a * b -> a = value / b
                return operands[0].second->getHumanValue( value / operands[1].second->getValue() );                
            case '/':
                // value = a / b -> a = value * b
                return operands[0].second->getHumanValue( value * operands[1].second->getValue() );                
            default:
                break;
            }
        } else if ( human == OPERAND_B )
        {
            switch (operation)
            {
            case '+':
                // value = a + b -> b = value - a;
                return operands[1].second->getHumanValue( value - operands[0].second->getValue() );
            case '-':
                // value = a - b -> b = a - value;
                return operands[1].second->getHumanValue( operands[0].second->getValue() - value );
            case '*':
                // value = a * b -> b = value / a
                return operands[1].second->getHumanValue( value / operands[0].second->getValue() );
            case '/':
                // value = a / b -> b = a / value
                return operands[1].second->getHumanValue( operands[0].second->getValue() / value );
            default:
                break;
            }
        } 

        throw std::runtime_error("Can't find human value in path which doesn't lead to human");

        return -1;
    }
};

class Work
{
public:
    Work() {};

    void addMonkey(const std::string& line)
    {
        {
            std::smatch match;
            if (std::regex_match(line, match, operation_rule))
            {
                auto monkey = std::make_shared<Node>(
                    match[1].str(),
                    match[2].str(),
                    match[3].str()[0],
                    match[4].str()
                );

                unresolved_monkeys.push_back(monkey);
                monkey_order.push_back(monkey);

                if (monkey->getName() == "root")
                {
                    root = monkey;
                }
            }
            else if (std::regex_match(line, match, value_rule))
            {
                auto monkey = std::make_shared<Node>(
                    match[1].str(),
                    std::stoi(match[2].str())
                );

                resolved_monkeys.emplace(monkey->getKey(), monkey);
                monkey_order.push_back(monkey);

                /* Unlikely but hey */
                if (monkey->getName() == "root")
                {
                    root = monkey;
                }
            }
        }
    }

    void reduce()
    {   
        std::size_t i=0;

        while (! unresolved_monkeys.empty())
        {
            for (auto &i : unresolved_monkeys)
            {
                if (i->tryLink(resolved_monkeys))
                {
                    resolved_monkeys.emplace(i->getKey(), i);
                    i.reset();
                }
            }

            unresolved_monkeys.erase(
                std::remove_if(unresolved_monkeys.begin(), unresolved_monkeys.end(), [](const std::shared_ptr<Node>& m) {
                    return !(bool)m;
                }),
                unresolved_monkeys.end()
            );
        }
    }

    void solve()
    {
        while (! root->isSolved() )
        {
            for (auto& i : monkey_order)
            {
                i->trySolve();
            }
        }
    }

    void Print(std::ostream& os)
    {
        for (auto& m : monkey_order)
        {
            m->Print(os);
            os << std::endl;
        }
    }

    void mapHuman()
    {
        while (! root->isHumanKnown())
        {
            for (auto& m : monkey_order)
            {
                m->tryFindHuman();
            }
        }
    }

    int64_t getRootNumber() const
    {
        return root->getValue();
    }

    int64_t getHumanNumber() const
    {
        return root->getHumanValue();
    }

private:

    static const std::regex operation_rule;
    static const std::regex value_rule;

    std::vector<std::shared_ptr<Node>> unresolved_monkeys;
    std::map<uint32_t, std::shared_ptr<Node>> resolved_monkeys;
    std::vector<std::shared_ptr<Node>> monkey_order;

    std::shared_ptr<Node> root;
};

const std::regex Work::operation_rule{"^\\s*([a-z]{4})\\s*:\\s*([a-z]{4})\\s*([+-/*]{1})\\s*([a-z]{4})\\s*$"};
const std::regex Work::value_rule{"^\\s*([a-z]{4})\\s*:\\s*([0-9]+)\\s*$"};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    Work work;
    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            if (! line.empty())
            {
                work.addMonkey(line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    work.reduce();
    work.solve();

    std::cout << "Root has value " << work.getRootNumber() << std::endl;

    work.mapHuman();

    std::cout << "Human has value " << work.getHumanNumber() << std::endl;

    return 0;
}
