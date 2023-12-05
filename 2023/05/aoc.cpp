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

enum IdentifierType : std::size_t { SEED = 0, SOIL, FERTILIZER, WATER, LIGHT, TEMPERATURE, HUMIDITY, LOCATION, UNKNOWN };

// clang-format off
static const std::array<std::pair<std::string, IdentifierType>, 8> identifierNames{{
    { "seed", SEED },
    { "soil", SOIL },
    { "fertilizer", FERTILIZER },
    { "water", WATER },
    { "light", LIGHT },
    { "temperature", TEMPERATURE },
    { "humidity", HUMIDITY },
    { "location", LOCATION }  
}};
// clang-format on

class TransformType
{
public:
    TransformType() : m_From(UNKNOWN), m_To(UNKNOWN){};
    TransformType(IdentifierType a_from, IdentifierType a_to) : m_From(a_from), m_To(a_to){};

    IdentifierType from() const
    {
        return m_From;
    }
    IdentifierType to() const
    {
        return m_To;
    }

private:
    IdentifierType m_From;
    IdentifierType m_To;
};

namespace std
{
    string to_string(const IdentifierType& id)
    {
        return identifierNames.at((std::size_t)id).first;
    }

    string to_string(const TransformType& id)
    {
        return std::to_string(id.from()) + "-to-" + std::to_string(id.to());
    }

    ostream& operator<<(ostream& os, const IdentifierType& id)
    {
        os << std::to_string(id);
        return os;
    }

    ostream& operator<<(ostream& os, const TransformType& id)
    {
        os << std::to_string(id);
        return os;
    }
} // namespace std

template <typename T> struct Identifier {
    using value_type = T;

    Identifier() : value(-1), type(UNKNOWN){};
    Identifier(const T& v, IdentifierType t) : value(v), type(t){};

    bool operator<(const Identifier& rhs) const
    {
        if (type < rhs.type)
            return true;
        if (type > rhs.type)
            return false;

        return value < rhs.value;
    }

    T value;
    IdentifierType type;
};

template <typename T> class MapRange;

template <typename T> struct IdentifierRange {
    using value_type = T;

    IdentifierRange() : first(-1), last(-1), type(UNKNOWN), range(nullptr){};
    IdentifierRange(const T& a_first, IdentifierType t) : first(a_first), last(a_first), type(t), range(nullptr){};
    IdentifierRange(const T& a_first, const T& a_last, IdentifierType t)
     : first(std::min(a_first, a_last)), last(std::max(a_first, a_last)), type(t), range(nullptr){};

    bool operator<(const IdentifierRange& rhs)
    {
        return first < rhs.first;
    }

    void write(std::ostream& os) const
    {
    }

    T first;
    T last;

    IdentifierType type;

    MapRange<value_type> const* range;
};

namespace std
{
    template <typename T> string to_string(const IdentifierRange<T>& range)
    {
        return "[" + std::to_string(range.first) + "-" + std::to_string(range.last) + "](" + std::to_string(range.type) + ")";
    }

    template <typename T> ostream& operator<<(ostream& os, const IdentifierRange<T>& range)
    {
        os << std::to_string(range);
        return os;
    }
} // namespace std

template <typename T> class MapRange
{
public:
    using value_type = T;

    MapRange() : m_Start(std::numeric_limits<value_type>::min()), m_End(std::numeric_limits<value_type>::max()), m_Delta(0){};
    MapRange(value_type a_start, value_type a_end, value_type a_delta) : m_Start(a_start), m_End(a_end), m_Delta(a_delta){};

    bool operator<(const MapRange& rhs) const
    {
        return m_Start < rhs.m_Start;
    }

    bool contains(const Identifier<value_type>& id) const
    {
        return (id.value >= m_Start) && (id.value < m_End);
    }

    bool contains(const IdentifierRange<value_type>& range) const
    {
        return (range.first >= m_Start) && (range.last < m_End);
    }

    value_type xfrm(const value_type& val) const
    {
        return val + m_Delta;
    }

    void write(std::ostream& os) const
    {
        if (m_Start == std::numeric_limits<value_type>::min()) {
            os << "]-∞";
        } else {
            os << "[" << m_Start;
        }

        os << "-";

        if (m_End == std::numeric_limits<value_type>::max()) {
            os << "∞[";
        } else {
            os << m_End << "[";
        }
    }

    std::pair<std::vector<IdentifierRange<value_type>>, std::vector<IdentifierRange<value_type>>> xfrm(IdentifierRange<value_type> value) const
    {
        if (contains(value.first)) {
        }
    }

    const value_type& start() const
    {
        return m_Start;
    }

    const value_type& end() const
    {
        return m_End;
    }

    const value_type& delta() const
    {
        return m_Delta;
    }

    MapRange* ptr()
    {
        return this;
    }

    const MapRange* ptr() const
    {
        return this;
    }

private:
    value_type m_Start;
    value_type m_End;
    value_type m_Delta;
};

namespace std
{
    template <typename T> string to_string(const MapRange<T>& range)
    {
        std::ostringstream buf;
        range.write(buf);
        return buf.str();
    }

    template <typename T> ostream& operator<<(ostream& os, const MapRange<T>& id)
    {
        id.write(os);
        return os;
    }

} // namespace std

template <typename T> class Map
{
public:
    using value_type = T;

    Map(const TransformType& a_xfrm) : xfrm(a_xfrm){};

    Map() : xfrm(TransformType()){};

    IdentifierType getSourceType() const
    {
        return xfrm.from();
    }
    IdentifierType getDestinationType() const
    {
        return xfrm.to();
    }

    Identifier<value_type> operator[](const Identifier<value_type>& value) const
    {
        if (value.type != xfrm.from())
            throw std::invalid_argument("Invalid source type");

        for (auto& r : ranges) {
            if (r.contains(value)) {
                return Identifier<value_type>(r.xfrm(value.value), xfrm.to());
            }
        }

        throw std::invalid_argument("should not reach");

        return Identifier<value_type>(value.value, xfrm.to());
    }

    std::vector<IdentifierRange<value_type>> operator[](const IdentifierRange<value_type>& value) const
    {
        /* First break up the value range into subranges that each only span a single transform range */
        std::vector<IdentifierRange<value_type>> slices;

        std::optional<IdentifierRange<value_type>> current(value);
        for (auto& r : ranges) {
            if (!current.has_value())
                break;

            if (r.start() > current.value().last) {
                slices.emplace_back(current.value());
                current.reset();
            } else if ((current.value().first < r.start()) && (current.value().last >= r.start())) {
                /* Range left boundary intersects value slice */
                slices.emplace_back(current.value().first, r.start() - 1, value.type);
                current.value().first = r.start();
            }
        }

        if (current.has_value()) {
            slices.emplace_back(current.value());
        }

        std::vector<IdentifierRange<value_type>> ret;

        for (auto& slice : slices) {
            for (auto& r : ranges) {
                if (r.contains(slice)) {
                    ret.emplace_back(r.xfrm(slice.first), r.xfrm(slice.last), xfrm.to());
                    break;
                }
            }
        }

        if (ret.size() != slices.size()) {
            throw std::invalid_argument("Slicing and transforming failed");
        }

        return ret;
    }

    void addRange(value_type in, value_type out, value_type n)
    {
        ranges.emplace_back(in, in + n, out - in);
    }

    void print() const
    {
        std::cout << xfrm.from() << "-to-" << xfrm.to() << " map:" << std::endl;
        for (auto& r : ranges) {
            std::cout << " - ";
            r.write(std::cout);
            std::cout << " => " << r.delta() << std::endl;
        }
    }

    void validate()
    {
        if (ranges.empty())
            throw std::invalid_argument("No transforms");

        std::sort(ranges.begin(), ranges.end());

        for (auto i = ranges.begin(); i != ranges.end(); i = std::next(i)) {
            auto j = std::next(i);

            if (j != ranges.end()) {
                if (i->end() > j->start())
                    throw std::invalid_argument("Overlapping zones");

                if (i->end() < j->start()) {
                    /* insert a 0 shift range */
                    ranges.emplace(j, i->end(), j->start(), 0);
                }
            }
        }

        /* add a start sentinel range and an end sentinel range, reduces checking later on */
        ranges.emplace(ranges.begin(), std::numeric_limits<value_type>::min(), ranges.begin()->start(), 0);
        ranges.emplace_back(ranges.rbegin()->end(), std::numeric_limits<value_type>::max(), 0);
    }

private:
    std::vector<MapRange<value_type>> ranges;
    TransformType xfrm;
};

class Almanac
{
public:
    using value_type = int64_t;

    Almanac(){};

    Identifier<value_type> operator[](const Identifier<value_type>& value) const
    {
        auto map = maps.find(value.type);
        if (map == maps.end())
            throw std::invalid_argument("Unmapped source type");

        return map->second[value];
    }

    std::vector<Identifier<value_type>> operator[](const std::vector<Identifier<value_type>>& values) const
    {
        std::vector<Identifier<value_type>> ret;

        for (auto& value : values)
            ret.emplace_back((*this)[value]);

        return ret;
    }

    std::vector<IdentifierRange<value_type>> operator[](const IdentifierRange<value_type>& value) const
    {
        auto map = maps.find(value.type);
        if (map == maps.end())
            throw std::invalid_argument("Unmapped source type");

        return map->second[value];
    }

    std::vector<IdentifierRange<value_type>> operator[](const std::vector<IdentifierRange<value_type>>& values) const
    {
        std::vector<IdentifierRange<value_type>> ret;

        for (auto& v : values) {
            auto slices = (*this)[v];
            ret.insert(ret.end(), slices.begin(), slices.end());
        }

        std::sort(ret.begin(), ret.end());

        return ret;
    }

    void addRange(const TransformType& xfrm, std::size_t in, std::size_t out, std::size_t n)
    {
        auto map_iter = maps.emplace(xfrm.from(), xfrm);
        auto& map = map_iter.first->second;

        if (map.getDestinationType() != xfrm.to()) {
            throw std::invalid_argument("Invalid desintation type");
        }

        map.addRange(in, out, n);
    }

    void validate()
    {
        for (auto& map : maps) {
            map.second.validate();
        }
    }

    void print() const
    {
        for (auto& m : maps) {
            m.second.print();
        }
    }

private:
    std::unordered_map<IdentifierType, Map<value_type>> maps;
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    Almanac almanac;
    std::vector<Identifier<Almanac::value_type>> seeds;

    const std::regex re_seedlist{"\\s*\\s([0-9]+)"};
    const std::regex re_mapdef{"^\\s*([a-z]+)-to-([a-z]+)\\s+map:\\s*$"};
    const std::regex re_rangedef{"^\\s*([0-9]+)\\s+([0-9]+)\\s+([0-9]+)\\s*$"};

    try {
        std::ifstream infile(argv[1]);

        std::string line;
        TransformType current_xfrm(UNKNOWN, UNKNOWN);

        while (std::getline(infile, line)) {
            if (line.find("seeds:") == 0) {
                for (auto i = std::sregex_iterator(line.begin(), line.end(), re_seedlist); i != std::sregex_iterator(); i = std::next(i)) {
                    std::smatch match = *i;
                    seeds.emplace_back(std::stoul(match.str()), SEED);
                }
                continue;
            }

            std::smatch match;
            if (std::regex_match(line, match, re_rangedef)) {
                almanac.addRange(current_xfrm, std::stoul(match[2].str()), std::stoul(match[1].str()), std::stoul(match[3].str()));
                continue;
            }

            if (std::regex_match(line, match, re_mapdef)) {
                auto from_elm = std::find_if(identifierNames.begin(), identifierNames.end(), [&match](const decltype(identifierNames)::value_type& value) {
                    return value.first == match[1].str();
                });
                if (from_elm == identifierNames.end())
                    throw std::invalid_argument("Wrong from identifier name " + match[1].str());

                auto to_elm = std::find_if(identifierNames.begin(), identifierNames.end(), [&match](const decltype(identifierNames)::value_type& value) {
                    return value.first == match[2].str();
                });
                if (to_elm == identifierNames.end())
                    throw std::invalid_argument("Wrong to identifier name " + match[2].str());

                current_xfrm = TransformType(from_elm->second, to_elm->second);

                continue;
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    almanac.validate();

    std::vector<Identifier<Almanac::value_type>> results(seeds);
    if (results.empty()) {
        std::cerr << "Got no seeds to start with" << std::endl;
        std::exit(-2);
    }

    while (!results.empty() && (results[0].type != LOCATION)) {
        results = almanac[results];
    }

    // almanac.print();

    auto best_elm = std::min_element(results.begin(), results.end());

    std::cout << "Result A " << best_elm->value << std::endl;

    /* Interpret seeds as ranges */
    std::vector<IdentifierRange<Almanac::value_type>> ranges;

    for (auto i = seeds.begin(); i != seeds.end();) {
        auto first = *i;
        i = std::next(i);

        if (i != seeds.end()) {
            ranges.emplace_back(first.value, first.value + i->value - 1, SEED);
            i = std::next(i);
        } else {
            ranges.emplace_back(first.value, first.value, SEED);
        }
    }

    std::sort(ranges.begin(), ranges.end());

    while (!ranges.empty() && (ranges[0].type != LOCATION)) {
        ranges = almanac[ranges];
    }

    std::sort(ranges.begin(), ranges.end());

    std::cout << "Result B " << ranges.begin()->first << std::endl;

    return 0;
}
