#include <string>
#include <array>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <future>

class TokenString : public std::vector<std::pair<char, int64_t>>
{
    public:

        TokenString(const std::string& txt)
        {
            /* Tokenize */
            for (size_t i = 0; i < txt.length();)
            {
                if (isspace(txt[i]))
                {
                    /* Skip whitespace  */
                    i++;
                }
                else if (isdigit(txt[i]))
                {
                    /* Found digit, so we are at a number */
                    size_t j = i+1;
                    for (; (j < txt.length()) && isdigit(txt[j]); ++j);

                    if (j < txt.length())
                    {
                        this->emplace_back('#', std::stol(txt.substr(i, j-i)));
                        i = j;
                    }
                    else
                    {
                        this->emplace_back('#', std::stol(txt.substr(i)));
                        break;
                    }
                }
                else
                {
                    switch (txt[i])
                    {
                        case '*':
                        case '+':
                        case '(':
                        case ')':
                            emplace_back(txt[i], 0);
                            break;
                        default:
                            throw std::invalid_argument("Token string parsing failed;");
                            break;
                    }
                    i++;
                }
            }
        }
};

class Solver
{
    public:

        Solver(const TokenString& tokens, std::function<int(char)> fPrecedence)
        {
            /* Shunting yard algorithm.  \o/ all bow to EWD. */
            std::deque<char> opstack;

            for (auto& t: tokens)
            {
                if (t.first == '#')
                {
                    rpn.emplace_back('#', t.second);
                }
                else if ((t.first == '+') || (t.first == '*'))
                {
                    int new_prec = fPrecedence(t.first);

                    while (! opstack.empty())
                    {
                        if ( opstack.back() == '(' ) break;

                        int back_prec = fPrecedence(opstack.back());

                        if (back_prec < new_prec ) break;

                        rpn.emplace_back(opstack.back(), 0);
                        opstack.pop_back();
                    }

                    opstack.push_back(t.first);
                }
                else if (t.first == '(')
                {
                    opstack.push_back('(');
                }
                else if (t.first == ')')
                {
                    while ((! opstack.empty()) && (opstack.back() != '('))
                    {
                        rpn.emplace_back(opstack.back(), 0);
                        opstack.pop_back();
                    }

                    if ((! opstack.empty() ) && (opstack.back() == '('))
                    {
                        opstack.pop_back();
                    }
                }
            }

            while (! opstack.empty())
            {
                rpn.emplace_back(opstack.back(), 0);
                opstack.pop_back();
            }
        };

        std::string toString()
        {
            std::string ret;
            std::array<char, 64> buf;

            for (auto i = rpn.begin(); i != rpn.end(); ++ i)
            {
                switch(i->first)
                {
                    case '+':
                        ret += " + ";
                        break;
                    case '*':
                        ret += " * ";
                        break;
                    case '(':
                        ret += " ( ";
                        break;
                    case ')':
                        ret += " ) ";
                        break;
                    case '#':
                        ret += std::string(buf.data(), std::snprintf(buf.data(), buf.size(), " %ld ", i->second));
                        break;
                }
            }

            return ret;
        }

        int64_t calculate()
        {
            std::array<int64_t, 2> args;
            std::deque<int64_t> stack;

            for (auto& op : rpn)
            {
                switch (op.first)
                {
                    case '+':
                        if (stack.size() < 2) throw std::invalid_argument("Syntax error");
                        for (size_t i=0; i<2; ++i)
                        {
                            args[i] = stack.back();
                            stack.pop_back();
                        }

                        stack.push_back(args[0] + args[1]);
                        break;
                    case '*':
                        if (stack.size() < 2) throw std::invalid_argument("Syntax error");
                        for (size_t i=0; i<2; ++i)
                        {
                            args[i] = stack.back();
                            stack.pop_back();
                        }

                        stack.push_back(args[0] * args[1]);
                        break;
                    case '#':
                        stack.push_back(op.second);
                        break;
                    default:
                        throw std::invalid_argument("syntax error");
                        break;
                }
            }

            if (stack.size() != 1)
                throw std::invalid_argument("syntax error");

            return stack.back();
        }

    private:

        std::vector<std::pair<char,int64_t>> rpn;

    protected:

        virtual int operator_precedence(char t)
        {
            return 0;
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

    std::vector<TokenString> equations;
    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_w("^\\s*$");


        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (! std::regex_match(line, match, matchrule_w))
            {
                equations.emplace_back(line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }


    std::future<int64_t> suma = std::async(
        [equations](){
            int64_t ret = 0;

            for (auto& eq : equations)
            {
                Solver solver(eq, [](char c) { (void)c; return 0; });
                ret += solver.calculate();
            }

            return ret;
        }
    );

    std::future<int64_t> sumb = std::async(
        [equations](){
            int64_t ret = 0;

            for (auto& eq : equations)
            {
                Solver solver(eq, [](char c) {
                    switch (c)
                    {
                        case '(': return 10;
                        case ')': return 10;
                        case '+': return 9;
                        case '*': return 8;
                        default:  return 0;
                    }
                });

                ret += solver.calculate();
            }

            return ret;
        }
    );

    std::cout << "Sum without precedence : " << suma.get() << std::endl;
    std::cout << "Sum *with* precedence  : " << sumb.get() << std::endl;

    return 0;
}
