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

class Parser
{
public:

    Parser() : m_firstIllegal('*'), m_completionScore(0) {}
    Parser(const std::string& line) : m_firstIllegal('*'), m_tokens(line), m_completionScore(0) {}

    bool isValid() const { return m_firstIllegal =='*'; };

    int64_t getCompletionScore() const { return m_completionScore; };

    int getInvalidSyntaxScore()
    {
        switch( m_firstIllegal )
        {
            case ')': return 3;
            case ']': return 57;
            case '}': return 1197;
            case '>': return 25137;
            default:
                break;
        }

        return 0;
    }

    bool validate()
    {
        m_stack.clear();
        m_completionScore = 0;

        std::any_of( m_tokens.begin(), m_tokens.end() , [&](char c) {
            switch(c)
            {
                case '{':
                case '[':
                case '(':
                case '<':
                    m_stack.emplace_back(c);
                    break;
                case '}':
                case ']':
                case '>':
                case ')':
                    if (m_stack.empty())
                    {
                        m_firstIllegal = c;
                        return true;
                    }
                    if (*m_stack.rbegin() != swap(c))
                    {
                        m_firstIllegal = c;
                        return true;
                    }
                    m_stack.pop_back();
                    break;
                default:
                    m_firstIllegal = c;
                    return true;
            }

            return false;
        });

        if ( isValid() )
        {
            /* calculate the completion score */

            for (auto i = m_stack.crbegin(); i != m_stack.crend(); ++ i)
            {
                switch(*i)
                {
                    case '(':
                        m_completionScore = ( m_completionScore * 5 ) + 1;
                        break;
                    case '[':
                        m_completionScore = ( m_completionScore * 5 ) + 2;
                        break;
                    case '{':
                        m_completionScore = ( m_completionScore * 5 ) + 3;
                        break;
                    case '<':
                        m_completionScore = ( m_completionScore * 5 ) + 4;
                        break;

                    default:
                        break;
                }
            }
        }

        return isValid();
    }

    bool operator<( const Parser& other )
    {
        return m_completionScore < other.m_completionScore;
    }

private:

    char m_firstIllegal;
    int64_t m_completionScore;

    std::string m_tokens;
    std::vector<char> m_stack;

    static char swap(char c)
    {
        switch(c)
        {
            case '{': 
            case '[': 
            case '(': 
            case '<': 
                return c;
            case '}': return '{';
            case ']': return '[';
            case '>': return '<';
            case ')': return '(';
            default:
                return '*';
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

    std::vector<Parser> lines;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            if (! line.empty())
            {
                lines.emplace_back(line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    int syntaxScore = 0;
    for (auto& l : lines)
    {
        if (! l.validate())
        {
            syntaxScore += l.getInvalidSyntaxScore();
        }
    }

    std::cout << "Invalid syntax score " << syntaxScore << std::endl;

    lines.erase(
        std::remove_if( lines.begin(), lines.end(), [](Parser& c) { return ! c.isValid(); }),
        lines.end()
    );
   
    std::sort( lines.begin(), lines.end() );

    std::cout << "Median completion score " << lines[((lines.size()-1)/2)].getCompletionScore() << std::endl;

    return 0;
}
