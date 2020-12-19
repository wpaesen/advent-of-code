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

class Rule : public std::enable_shared_from_this<Rule>
{
    public:
        const size_t HASMATCH = 999999;

        Rule(const std::string& a_Desc)
        {
            size_t colon =  a_Desc.find(":");
            if (colon == std::string::npos)
            {
                throw std::invalid_argument("No rule id");
            }

            id = std::stoi(a_Desc.substr(0, colon));

            size_t start = a_Desc.find_first_of(" \t");
            if (start == std::string::npos)
            {
                throw std::invalid_argument("Empty rule");
            }

            start = a_Desc.find_first_not_of(" \t", start);
            if (start == std::string::npos)
            {
                throw std::invalid_argument("Empty rule");
            }

            desc = a_Desc.substr(start);
        }

        void print()
        {
            std::cout << "Rule " << id;

            if (matchlist.empty())
            {
                std::cout << " = " << token << std::endl;
            }
            else
            {
                std::cout << " => ";

                for (auto &s: matchlist)
                {
                    for (auto &r: s)
                    {
                        std::cout << " " << r->getId();
                    }

                    std::cout << " | ";
                }

                std::cout << std::endl;
            }
        }

        void resolve( std::unordered_map<int, std::shared_ptr<Rule>>& a_Rules )
        {
            if (desc.empty())
            {
                throw std::domain_error("Bad use");
            }

            if (desc[0] == '"')
            {
                token = desc[1];
            }
            else
            {
                std::vector<std::shared_ptr<Rule>> option;
                size_t j = 0;

                for (size_t i=0; i<desc.length(); i++)
                {
                    if (isspace(desc[i]))
                    {
                        if (j<i)
                        {
                            std::string tok = desc.substr(j, i-j);
                            if (tok[0] == '|')
                            {
                                if (! option.empty())
                                {
                                    matchlist.emplace_back(option);
                                    option.clear();
                                }
                            }
                            else
                            {
                                int rid = std::stoi(tok);
                                auto r = a_Rules[rid];
                                if (! r)
                                {
                                    throw std::invalid_argument("Rule syntax failure");
                                }
                                option.emplace_back(r);
                            }
                        }
                        j = i;
                    } else if (isspace(desc[j]))
                    {
                        j = i;
                    }
                }

                if (j != std::string::npos)
                {
                    std::string tok = desc.substr(j);
                    int rid = std::stoi(tok);
                    auto r = a_Rules[rid];
                    if (! r)
                    {
                        throw std::invalid_argument("Rule syntax failure");
                    }
                    option.emplace_back(r);
                }

                if (! option.empty())
                {
                    matchlist.emplace_back(option);
                }
            }
        }

        bool matches(const std::string& data)
        {
            return (validate(data, 0) == data.length());
        }

        int getId()
        {
            return id;
        }

    private:

        size_t validate( const std::string& data, size_t pos = 0 )
        {
            if ( matchlist.empty() )
            {
                return (data[pos] == token) ? 1 : 0;
            }

            for (auto &s: matchlist)
            {
                size_t k = _subvalidate( data, pos, s);
                if (k != 0)
                {
                    return k;
                }
            }

            return 0;
        }

        size_t _subvalidate( const std::string& data, size_t pos, std::vector<std::shared_ptr<Rule>>& sub )
        {
            size_t j = 0;
            for (auto &r: sub)
            {
                size_t k = r->validate(data, pos + j);
                if (k == 0)
                {
                    return 0;
                }

                j += k;

                if (id == r->getId())
                {
                    if (pos+j == data.length())
                    {
                        return j;
                    }
                }
            }

            return j;
        }

        std::vector<std::vector<std::shared_ptr<Rule>>> matchlist;
        char token;

        int id;
        std::string desc;
};



int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::unordered_map<int, std::shared_ptr<Rule>> rules;
    std::vector<std::string> messages;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_w("^\\s*$");

        bool parsingRules = true;
        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_w))
            {
                parsingRules = false;
            }
            else if (parsingRules)
            {
                std::shared_ptr<Rule> r = std::make_shared<Rule>(line);
                rules[r->getId()] = r;
            }
            else if (line.length() > 0)
            {
                line.erase(line.find_last_not_of(" \n\r\t")+1);
                messages.emplace_back(line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    /* setup rule tree */
    for (auto& r: rules)
    {
        r.second->resolve(rules);
    }

    std::cout << "Parsed" << std::endl;
    std::cout << " - " << rules.size() << " rules" << std::endl;
    std::cout << " - " << messages.size() << " messages" << std::endl << std::endl;

    /* check for matches in the input data */
    int n_matches = 0;
    for (auto &l: messages)
    {
        if (rules[0]->matches(l))
        {
            n_matches++;
        }
    }

    std::cout << n_matches << " messages are valid." << std::endl;


    return 0;
}
