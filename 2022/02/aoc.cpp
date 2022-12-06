#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <vector>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <bitset>

class Draw
{
public:
    enum Item {
        ROCK,
        PAPER,
        SCISSORS
    };

    enum Outcome
    {
        LOSE,
        DRAW,
        WIN
    };

    Draw(const std::string& state) : line(state)
    {
        if (state.size() < 3)
        {
            throw std::invalid_argument("Invalid input size");
        }

        switch (state[0])
        {
        case 'A':
            items[0] = ROCK;
            break;
        case 'B':
            items[0] = PAPER;
            break;
        case 'C':
            items[0] = SCISSORS;
            break;
        default:
            throw std::invalid_argument("Invalid input left");
            break;
        }

        switch (state[2])
        {
        case 'X':
            items[1] = ROCK;
            outcome = LOSE;
            break;
        case 'Y':
            items[1] = PAPER;
            outcome = DRAW;
            break;
        case 'Z':
            items[1] = SCISSORS;
            outcome = WIN;
            break;
        default:
            throw std::invalid_argument("Invalid input left");
            break;
        }
    }

    std::array<std::size_t,2> getScoresA() const
    {
        return getScores(items[1]);
    }

    std::array<std::size_t,2> getScoresB() const
    {
        return getScores(getSelectedItemByOutCome());
    }

private:

    std::array<std::size_t,2> getScores(Item player2) const
    {
        auto ret = getRoundScores(player2);

        for (std::size_t i=0; i<2; ++i)
        {
            switch (player2)
            {
            case ROCK:
                ret[i] += 1;
                break;
            case PAPER:
                ret[i] += 2;
                break;
            case SCISSORS:
                ret[i] += 3;
                break;
            }
        }

        return ret;
    }

    std::array<std::size_t,2> getRoundScores(Item player2) const
    {
        switch (items[0])
        {
        case ROCK:
            switch (player2)
            {
                case ROCK:
                    return {3, 3};
                case PAPER:
                    return {0, 6};
                case SCISSORS:
                    return {6, 0};
            }
            break;
        case PAPER:
            switch (player2)
            {
                case ROCK:
                    return {6, 0};
                case PAPER:
                    return {3, 3};
                case SCISSORS:
                    return {0, 6};
            }
            break;
        case SCISSORS:
            switch (player2)
            {
                case ROCK:
                    return {0, 6};
                case PAPER:
                    return {6, 0};
                case SCISSORS:
                    return {3, 3};
            }
            break;
        }

        throw std::invalid_argument("BUG");

        return {0, 0};
    }

    Item getSelectedItemByOutCome() const
    {
        switch (outcome)
        {
        case LOSE:
            switch (items[0])
            {
                case ROCK:
                    return SCISSORS;
                case PAPER:
                    return ROCK;
                case SCISSORS:
                    return PAPER;
            }
            break;
        case DRAW:
            return items[0];
        case WIN:
            switch (items[0])
            {
                case ROCK:
                    return PAPER;
                case PAPER:
                    return SCISSORS;
                case SCISSORS:
                    return ROCK;
            }
            break;
        }

        throw std::invalid_argument("BUG");

        return ROCK;
    }

    std::array<Item, 2> items;
    std::string line;
    Outcome outcome;
};

template<typename T, std::size_t N>
std::array<T, N>& operator+=(std::array<T, N>& lhs, const std::array<T, N>& rhs)
{
    for (std::size_t i=0; i<N; ++i)
    {
        lhs[i] += rhs[i];
    }
    return lhs;
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Draw> draws;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            if (! line.empty())
            {
                draws.emplace_back(line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    std::array<std::size_t,2> scoresA{0, 0};
    std::array<std::size_t,2> scoresB{0, 0};

    for (auto& draw : draws)
    {
        scoresA += draw.getScoresA();
        scoresB += draw.getScoresB();
    }

    std::cout << "My score is (A) : " << scoresA[1] << std::endl;
    std::cout << "My score is (B) : " << scoresB[1] << std::endl;
}
