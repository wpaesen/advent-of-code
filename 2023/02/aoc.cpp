#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
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

class Game
{
public:
    enum Color : std::size_t {
        RED = 0,
        GREEN = 1,
        BLUE = 2,
    };

    static constexpr std::array<std::size_t, 3> COLORS{{RED, GREEN, BLUE}};

    Game(const std::string& state)
    {
        std::istringstream input(state);
        std::string line;

        if (!std::getline(input, line, ':'))
            throw std::invalid_argument("Syntax error 1");

        static const std::string s_game{"Game "};
        if (line.compare(0, s_game.length(), s_game) != 0)
            throw std::invalid_argument("Syntax error 2");

        m_Id = std::stoull(line.substr(s_game.length()));

        while (std::getline(input, line, ';')) {
            parseHand(line);
        }
    }

    bool isPossible(const std::array<std::size_t, COLORS.size()>& limits)
    {
        for (auto& hand : m_Hands) {
            for (auto& c : COLORS) {
                if (hand[c] > limits[c])
                    return false;
            }
        }

        return true;
    }

    std::size_t getPower()
    {
        std::array<std::size_t, COLORS.size()> required;
        required.fill(0);

        for (auto& hand : m_Hands) {
            for (auto& c : COLORS) {
                if (required[c] < hand[c])
                    required[c] = hand[c];
            }
        }

        std::size_t power = 1;
        for (auto& v : required) {
            power *= v;
        }

        return power;
    }

    std::size_t m_Id;
    std::vector<std::array<std::size_t, COLORS.size()>> m_Hands;

private:
    void parseHand(const std::string& state)
    {
        std::array<std::size_t, COLORS.size()> hand;
        hand.fill(0);

        std::istringstream input(state);
        for (std::string part; std::getline(input, part, ',');) {
            std::size_t value = std::stoull(part);
            if (part.find("red") != std::string::npos) {
                hand[Color::RED] += value;
            } else if (part.find("green") != std::string::npos) {
                hand[Color::GREEN] += value;
            } else if (part.find("blue") != std::string::npos) {
                hand[Color::BLUE] += value;
            } else {
                throw std::invalid_argument("Syntax error 3");
            }
        }

        m_Hands.emplace_back(hand);
    }
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    std::vector<Game> games;
    try {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line)) {
            if (!line.empty()) {
                games.emplace_back(line);
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    std::array<std::size_t, 3> stones{{12, 13, 14}};
    std::size_t possible_games = 0;

    std::size_t power_sum = 0;
    for (auto& game : games) {
        if (game.isPossible(stones)) {
            possible_games += game.m_Id;
        }
        power_sum += game.getPower();
    }

    std::cout << "Sum of possible games " << possible_games << std::endl;
    std::cout << "Power sum " << power_sum << std::endl;

    return 0;
}
