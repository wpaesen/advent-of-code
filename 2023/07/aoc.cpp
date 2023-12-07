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
#include <string_view>

static constexpr bool with_debug = false;

using namespace std::string_view_literals;

class HandBase
{
public:
    enum Type {
        HIGH_CARD = 0,
        ONE_PAIR,
        TWO_PAIR,
        THREE_OF_A_KIND,
        FULL_HOUSE,
        FOUR_OF_A_KIND,
        FIVE_OF_A_KIND,
    };

    HandBase(std::size_t bid) : m_Bid(bid), m_Type(HIGH_CARD){};

    const Type& getType() const
    {
        return m_Type;
    }

    std::size_t getWinnings(std::size_t rank) const
    {
        return rank * m_Bid;
    }

    virtual std::string getCardFaces() const = 0;

    std::ostream& write(std::ostream& s) const
    {
        s << this->getCardFaces() << " " << m_Bid << " ";

        switch (m_Type) {
            case HIGH_CARD:
                s << "high-card";
                break;
            case ONE_PAIR:
                s << "one-pair";
                break;
            case TWO_PAIR:
                s << "two-pair";
                break;
            case THREE_OF_A_KIND:
                s << "three-of-a-kind";
                break;
            case FULL_HOUSE:
                s << "full-house";
                break;
            case FOUR_OF_A_KIND:
                s << "four-of-a-kind";
                break;
            case FIVE_OF_A_KIND:
                s << "five-of-a-kind";
                break;
        }

        return s;
    }

    bool operator<(const HandBase& rhs) const
    {
        if (m_Type < rhs.m_Type)
            return true;

        if (m_Type > rhs.m_Type)
            return false;

        for (auto i = 0; i < m_Cards.size(); ++i) {
            if (m_Cards[i] < rhs.m_Cards[i])
                return true;

            if (m_Cards[i] > rhs.m_Cards[i])
                return false;
        }

        return false;
    }

protected:
    std::array<std::size_t, 5> m_Cards;
    Type m_Type;
    std::size_t m_Bid;
};

class HandStandard : public HandBase
{
public:
    static constexpr std::string_view card_faces = "23456789TJQKA"sv;

    HandStandard(const std::string& cards, std::size_t bid) : HandBase(bid)
    {
        if (cards.size() != m_Cards.size())
            throw std::invalid_argument("Hand not size 5");

        m_Tally.fill(0);

        for (std::size_t i = 0; i < m_Cards.size(); ++i) {
            auto idx = card_faces.find_first_of(cards[i]);

            if (idx == std::string_view::npos) {
                throw std::invalid_argument("Unknown card face");
            }

            m_Cards[i] = idx;
            m_Tally[idx]++;
        }

        auto max_tally = std::max_element(m_Tally.begin(), m_Tally.end());
        if (*max_tally == 5) {
            m_Type = FIVE_OF_A_KIND;
        } else if (*max_tally == 4) {
            m_Type = FOUR_OF_A_KIND;
        } else if (*max_tally == 3) {
            auto has_2 = std::find(m_Tally.begin(), m_Tally.end(), 2);
            if (has_2 != m_Tally.end()) {
                m_Type = FULL_HOUSE;
            } else {
                m_Type = THREE_OF_A_KIND;
            }
        } else {
            unsigned pairs = 0;
            for (auto& t : m_Tally) {
                if (t == 2)
                    pairs++;
            }

            if (pairs == 2) {
                m_Type = TWO_PAIR;
            } else if (pairs == 1) {
                m_Type = ONE_PAIR;
            }
        }
    }

    std::string getCardFaces() const override
    {
        std::string ret;
        for (auto& c : m_Cards) {
            ret.push_back(card_faces[c]);
        }
        return ret;
    }

private:
    std::array<unsigned, card_faces.size()> m_Tally;
};

class HandWithJokers : public HandBase
{
public:
    static constexpr std::string_view card_faces = "J23456789TQKA"sv;

    HandWithJokers(const std::string& cards, std::size_t bid) : HandBase(bid)
    {
        if (cards.size() != m_Cards.size())
            throw std::invalid_argument("Hand not size 5");

        m_Tally.fill(0);

        for (std::size_t i = 0; i < m_Cards.size(); ++i) {
            auto idx = card_faces.find_first_of(cards[i]);

            if (idx == std::string_view::npos) {
                throw std::invalid_argument("Unknown card face");
            }

            m_Cards[i] = idx;
            m_Tally[idx]++;
        }

        auto max_tally = std::max_element(std::next(m_Tally.begin()), m_Tally.end());
        auto& jokers = m_Tally[0];

        if ((*max_tally == 5) || (jokers == 5)) {
            m_Type = FIVE_OF_A_KIND;
        } else if (*max_tally == 4) {
            if (jokers == 1) {
                m_Type = FIVE_OF_A_KIND;
            } else {
                m_Type = FOUR_OF_A_KIND;
            }
        } else if (*max_tally == 3) {
            if (jokers == 2) {
                m_Type = FIVE_OF_A_KIND;
            } else if (jokers == 1) {
                m_Type = FOUR_OF_A_KIND;
            } else {
                auto has_2 = std::find(std::next(m_Tally.begin()), m_Tally.end(), 2);
                if (has_2 != m_Tally.end()) {
                    m_Type = FULL_HOUSE;
                } else {
                    m_Type = THREE_OF_A_KIND;
                }
            }
        } else {
            unsigned pairs = 0;
            for (auto i = std::next(m_Tally.begin()); i != m_Tally.end(); i = std::next(i)) {
                if ((*i) == 2)
                    pairs++;
            }

            if (pairs == 2) {
                if (jokers == 1) {
                    m_Type = FULL_HOUSE;
                } else {
                    m_Type = TWO_PAIR;
                }
            } else if (pairs == 1) {
                if (jokers == 3) {
                    m_Type = FIVE_OF_A_KIND;
                } else if (jokers == 2) {
                    m_Type = FOUR_OF_A_KIND;
                } else if (jokers == 1) {
                    m_Type = THREE_OF_A_KIND;
                } else {
                    m_Type = ONE_PAIR;
                }
            } else {
                if (jokers == 4) {
                    m_Type = FIVE_OF_A_KIND;
                } else if (jokers == 3) {
                    m_Type = FOUR_OF_A_KIND;
                } else if (jokers == 2) {
                    m_Type = THREE_OF_A_KIND;
                } else if (jokers == 1) {
                    m_Type = ONE_PAIR;
                }
            }
        }
    }

    std::string getCardFaces() const override
    {
        std::string ret;
        for (auto& c : m_Cards) {
            ret.push_back(card_faces[c]);
        }
        return ret;
    }

private:
    std::array<unsigned, card_faces.size()> m_Tally;
};

namespace std
{
    string to_string(const HandBase& range)
    {
        std::ostringstream buf;
        range.write(buf);
        return buf.str();
    }

    ostream& operator<<(ostream& os, const HandBase& range)
    {
        range.write(os);
        return os;
    }
} // namespace std

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    std::vector<HandStandard> hands_standard;
    std::vector<HandWithJokers> hands_with_joker;

    const std::regex re_carddesc{"\\s*([" + std::string(HandStandard::card_faces) + "]{5})\\s+([0-9]+)"};

    try {
        std::ifstream infile(argv[1]);

        std::string line;

        while (std::getline(infile, line)) {
            std::smatch match;
            if (std::regex_match(line, match, re_carddesc)) {
                hands_standard.emplace_back(match[1].str(), std::stoul(match[2].str()));
                hands_with_joker.emplace_back(match[1].str(), std::stoul(match[2].str()));
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    std::sort(hands_standard.begin(), hands_standard.end());
    std::sort(hands_with_joker.begin(), hands_with_joker.end());
    if (with_debug) {
        std::cout << "Standard game :" << std::endl;
    }
    std::size_t standard_winnings = 0;
    for (std::size_t i = 0; i < hands_standard.size(); ++i) {
        if (with_debug) {
            std::cout << "Hand rank " << (i + 1) << " " << hands_standard[i] << std::endl;
        }
        standard_winnings += hands_standard[i].getWinnings(i + 1);
    }

    if (with_debug) {
        std::cout << "Joker game :" << std::endl;
    }
    std::size_t joker_winnings = 0;
    for (std::size_t i = 0; i < hands_with_joker.size(); ++i) {
        if (with_debug) {
            std::cout << "Hand rank " << (i + 1) << " " << hands_with_joker[i] << std::endl;
        }
        joker_winnings += hands_with_joker[i].getWinnings(i + 1);
    }

    std::cout << "Standard Winnigs    " << standard_winnings << std::endl;
    std::cout << "With joker winnings " << joker_winnings << std::endl;
    return 0;
}
