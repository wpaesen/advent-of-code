#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>

class Turn
{
    public:
        Turn() : last(-1), second_to_last(-1) {};

        void update(size_t turn)
        {
            second_to_last = last;
            last = turn;
        }

        int64_t next() {
            if (second_to_last == -1)
            {
                return 0;
            }
            else
            {
                return last - second_to_last;
            }

        };

        bool isFirst()
        {
            return second_to_last == -1;
        }

        size_t last;
        size_t second_to_last;
};

class Alu
{
    public:
        Alu(const std::vector<int64_t> initial)
        {
            for (size_t i=0; i<initial.size(); ++i)
            {
                m_Memory[initial[i]].update(i+1);
                m_Stack.emplace_back(initial[i]);
            }
        };

        void turn()
        {
            int64_t next = m_Memory[last()].next();

            m_Stack.emplace_back(next);
            m_Memory[next].update(m_Stack.size());
        }

        int64_t last()
        {
            return *m_Stack.rbegin();
        }

        std::vector<int64_t> m_Stack;
        std::map<int64_t, Turn> m_Memory;
};

int
main(int argc, char **argv)
{
    Alu alu({ 0, 14, 6, 20, 1, 4 });
    //Alu alu({ 0, 3, 6, });

    while (alu.m_Stack.size() < 2020)
    {
        alu.turn();
    }

    std::cout << "Last number said was " << alu.last() << std::endl;

    while (alu.m_Stack.size() < 30000000)
    {
        alu.turn();
    }

    std::cout << "Last number said was " << alu.last() << std::endl;

    return 0;
}
