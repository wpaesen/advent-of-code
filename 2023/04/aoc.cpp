#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <numeric>
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
#include <optional>

class Card
{
public:
    Card(const std::string& line) : m_NumInstances(1)
    {
        std::istringstream buf(line);

        std::string part;

        if (!std::getline(buf, part, ':'))
            throw std::invalid_argument("Syntax error");

        static const std::string l_card{"Card "};

        if (part.find(l_card) != 0)
            throw std::invalid_argument("Syntax error 1");

        m_CardNumber = std::stoull(part.substr(l_card.size()));

        if (!std::getline(buf, part, '|'))
            throw std::invalid_argument("Syntax error 2");

        m_Winning_Numbers = parseNumberList(part);

        if (!std::getline(buf, part))
            throw std::invalid_argument("Syntax error 3");

        m_Lot_Numbers = parseNumberList(part);
    }

    void print() const
    {
        std::cout << "Card " << m_CardNumber << ": ";

        for (std::size_t i = 0; i < 256; ++i) {
            if (m_Winning_Numbers.test(i)) {
                std::cout << i << " ";
            }
        }

        std::cout << "| ";
        for (std::size_t i = 0; i < 256; ++i) {
            if (m_Lot_Numbers.test(i)) {
                std::cout << i << " ";
            }
        }
        std::cout << " ";
        std::cout << getWinningNumberCount();
        std::cout << " Winning Numbers, ";
        std::cout << getWinningPoints();

        std::cout << " Points";

        std::cout << std::endl;
    }

    std::size_t getWinningNumberCount() const
    {
        auto result = m_Winning_Numbers & m_Lot_Numbers;

        return result.count();
    }

    std::size_t getWinningPoints() const
    {
        auto n_cards = getWinningNumberCount();
        if (n_cards > 0) {
            return 1 << (getWinningNumberCount() - 1);
        }
        return 0;
    }

    int getCardNumber() const
    {
        return m_CardNumber;
    }

    std::size_t getNumInstances() const
    {
        return m_NumInstances;
    }

    void addInstances(std::size_t n)
    {
        m_NumInstances += n;
    }

private:
    static std::bitset<256> parseNumberList(const std::string& s)
    {
        std::bitset<256> ret;

        ret.reset();

        for (std::size_t i = 0; i < s.length(); i += 3) {
            auto part = s.substr(i, 3);
            if (part.length() == 3) {
                auto val = std::stoul(part);
                ret.set(val);
            }
        }

        return ret;
    }

    std::bitset<256> m_Winning_Numbers;
    std::bitset<256> m_Lot_Numbers;

    int m_CardNumber;
    std::size_t m_NumInstances;
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    std::vector<Card> cards;
    try {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line)) {
            if (line.empty())
                break;
            cards.emplace_back(line);
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    auto score
        = std::accumulate(cards.begin(), cards.end(), (std::size_t)0, [](const std::size_t& acc, const Card& card) { return acc + card.getWinningPoints(); });

    std::cout << "A: Pile is worth " << score << " points" << std::endl;

    for (auto& c : cards) {
        auto winning_numbers = c.getWinningNumberCount();

        for (auto i = 0; i < winning_numbers; ++i) {
            cards.at(c.getCardNumber() + i).addInstances(c.getNumInstances());
        }
    }

    auto n_cards
        = std::accumulate(cards.begin(), cards.end(), (std::size_t)0, [](const std::size_t& acc, const Card& card) { return acc + card.getNumInstances(); });

    std::cout << "B: Total number of cards " << n_cards << std::endl;

#if 0 
    for (auto& c : cards) {
        c.print();
    }
#endif

    // std::cout << "Number sum A " << schematic.getValue_a() << std::endl;
    // std::cout << "Ratio sum B " << schematic.getValue_b() << std::endl;

    return 0;
}
