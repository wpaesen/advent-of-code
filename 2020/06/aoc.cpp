#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>

class Form
{
    public:

        Form() : m_NumPersons(0) {};

        void tallyAnswer(const char q)
        {
            m_Answers[q] += 1;
        }

        void tallyPerson()
        {
            m_NumPersons++;
        };

        unsigned getNumQuestionsAny()
        {
            return m_Answers.size();
        }

        unsigned getNumQuestionsAll()
        {
            unsigned ret = 0;
            for (auto& q: m_Answers)
            {
                if (q.second == m_NumPersons)
                {
                    ret += 1;
                }
            }

            return ret;
        }

        unsigned getNumPersons()
        {
            return m_NumPersons;;
        }

        bool hasAnswers()
        {
            return m_NumPersons > 0;
        }

    private:

        std::map<char, unsigned> m_Answers;
        unsigned m_NumPersons;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Form> records;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchfields("^([a-z]+)$");
        const std::regex emptyline("^[[:s:]]*$");

        Form rec;

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, emptyline))
            {
                if (rec.hasAnswers())
                {
                    records.emplace_back(rec);
                    rec = Form();
                }
            }
            else if (std::regex_match(line, match, matchfields))
            {
                rec.tallyPerson();
                for (auto c : match[1].str())
                {
                    rec.tallyAnswer(c);
                }
            }
        }

        if (rec.hasAnswers())
        {
            records.emplace_back(rec);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    unsigned sumany = 0;
    unsigned sumall = 0;

    std::cout << "Got " << records.size() << " records" <<std::endl;
    for (auto& rec : records)
    {
        sumany += rec.getNumQuestionsAny();
        sumall += rec.getNumQuestionsAll();
    }

    std::cout << "- Total any : " << sumany << std::endl;
    std::cout << "- Total all : " << sumall << std::endl;

    return 0;
}
