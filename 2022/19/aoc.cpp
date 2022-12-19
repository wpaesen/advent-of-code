#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
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

class Resources
{
public:
    enum RESOURCE : std::size_t {
        ORE = 0,
        CLAY = 1,
        OBSIDIAN = 2,
        GEODE = 3
    };

    Resources() {
        amounts.fill(0);
    };

    Resources operator+(const Resources& rhs) const
    {
        Resources ret;

        for (std::size_t i=0; i<amounts.size(); ++i)
        {
            ret.amounts[i] = amounts[i] + rhs.amounts[i];
        }

        return ret;
    }
   
    bool operator>=(const Resources& rhs) const
    {
        for (std::size_t i=0; i<amounts.size(); ++i)
        {
            if (amounts[i] < rhs.amounts[i]) return false;
        }

        return true;
    }

    Resources& operator-=(const Resources& rhs)
    {
        for (std::size_t i=0; i<amounts.size(); ++i)
        {
            if (rhs.amounts[i] > amounts[i])
                throw std::invalid_argument("Bank failed");

            amounts[i] -= rhs.amounts[i];
        }

        return *this;
    }

    Resources& operator+=(const Resources& rhs)
    {
        for (std::size_t i=0; i<amounts.size(); ++i)
        {
            amounts[i] += rhs.amounts[i];
        }

        return *this;
    }

    Resources& operator|=(const Resources& rhs)
    {
        for (std::size_t i=0; i<amounts.size(); ++i)
        {
            if (rhs.amounts[i] > amounts[i])
            {
                amounts[i] = rhs.amounts[i];
            }
        }

        return *this;    
    }

    bool operator<(const Resources& rhs) const
    {
        return amounts < rhs.amounts;
    }

    RESOURCE cheapest() const
    {
        RESOURCE cheapest = ORE;
        std::size_t cheapest_amount = amounts[cheapest];

        for (auto & r: all)
        {
            if ((amounts[r] > 0) && (amounts[r] < cheapest_amount))
            {
                cheapest = r;
                cheapest_amount = amounts[r];
            }
        }
       
        return cheapest;
    }

    bool empty() const
    {
        for (auto& r: amounts)
        {
            if (r > 0) return false;
        }

        return true;
    }

    std::size_t& operator[](RESOURCE r)
    {
        return amounts[r];
    }

    std::size_t operator[](RESOURCE r) const
    {
        return amounts[r];
    }

    void print(std::ostream& os) const
    {
        std::string sep = "";
        for (auto a : all)
        {
            if (amounts[a] > 0)
            {
                os << sep << amounts[a] << " " << asString(a);
                if (sep.empty())
                    sep = " and "; 
            }
        }
    }

    static constexpr std::array<RESOURCE, 4> all{ORE, CLAY, OBSIDIAN, GEODE};
    static constexpr std::array<RESOURCE, 3> nonores{CLAY, OBSIDIAN, GEODE};

    static RESOURCE parse(const std::string& string, std::size_t pos = 0)
    {
        if (string.find("ore") == pos)
            return ORE;
        if (string.find("clay") == pos)
            return CLAY;
        if (string.find("obsidian") == pos)
            return OBSIDIAN;
        if (string.find("geode") == pos)
            return GEODE;

        throw std::invalid_argument("Can't parse resource type");

        return ORE;
    }

    static std::string asString(RESOURCE r)
    {
        switch (r)
        {
        case ORE:
            return "ore";
        case CLAY:
            return "clay";
        case OBSIDIAN:
            return "obsidian";
        case GEODE:
            return "geode";
        default:
            break;
        }

        return "Unknown";
    }

private:
    std::array<std::size_t, 4> amounts;
};

std::ostream& operator<<(std::ostream& os, const Resources::RESOURCE& r)
{
    os << Resources::asString(r);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Resources& r)
{
    r.print(os);
    return os;
}

class BlueprintPart
{
public:
    BlueprintPart() : m_product(Resources::ORE) {};

    Resources::RESOURCE m_product;
    Resources m_cost;

    void print(std::ostream& os) const
    {
        os << "Each " << m_product << " robot costs ";
        std::string sep = "";

        for (auto& resource : Resources::all)
        {
            if (m_cost[resource] > 0)
            {
                os << sep << m_cost[resource] << " " << resource;
                if (sep.empty())
                    sep = " and ";
            }
        }

        os << ".";
    }

    void produce(Resources& amounts, Resources& robots) const
    {
        amounts -= m_cost;
        robots[m_product]++;
    }
};

std::ostream& operator<<(std::ostream& os, const BlueprintPart& p)
{
    p.print(os);
    return os;
}

class Blueprint
{
public:

    Blueprint() : m_id(0) {};

    Blueprint(const std::string& description) {
        auto i = description.find("Blueprint ");
        if (i == std::string::npos)
            throw std::invalid_argument("Blueprint not found in description");

        auto j = description.find(": ", i);
        if (j == std::string::npos)
            throw std::invalid_argument("Blueprint ID not found in description");

        m_id = std::stoul(description.substr(i+10));

        j = description.find("Each ", j);
        while (j != std::string::npos)
        {
            i = description.find("Each ", j+1);
            if (i == std::string::npos)
            {
                parse_cost(description.substr(j));
            }
            else
            {
                parse_cost(description.substr(j, i-j));                
            }

            j = i;
        }

        maxrobots[Resources::GEODE] = std::numeric_limits<std::size_t>::max();
    };

    std::size_t m_id;
    std::map<Resources::RESOURCE, BlueprintPart> m_products;
    Resources maxrobots;

    void print(std::ostream& os) const
    {
        os << "Blueprint " << m_id << ": ";
        
        std::string sep = "";
        for (auto &pr : m_products)
        {
            os << sep << pr.second;
            if (sep.empty())
                sep = " ";
        }
    }

    std::map<Resources::RESOURCE, const BlueprintPart*> getOptions(const Resources& amounts, const Resources& robots) const
    {
        std::map<Resources::RESOURCE, const BlueprintPart*> ret;

        /* List our options, based on the amount of resources available and the cost per robot.  Also, 
         * since it doesn't make sense to have more robots of each type than the maximum amount of resource per new robot 
         * don't list them as option once that has been reached.
         */
        for (auto &p : m_products)
        {
            if ((amounts >= p.second.m_cost) && (robots[p.first] < maxrobots[p.first]))
            {
                ret.emplace(p.first, &p.second);
            }
        }

        return ret;
    }

    static const Resources nullresource;

    const BlueprintPart* get(Resources::RESOURCE r) const
    {
        auto i = m_products.find(r);
        if (i == m_products.end())
            return nullptr;

        return &i->second;
    }

    const Resources& operator[](Resources::RESOURCE r) const
    {
        auto i = m_products.find(r);
        if (i == m_products.end())
            return nullresource;

        return i->second.m_cost;
    }

    bool hasMaxRobots(const Resources& robots) const
    {
        for (auto &r : { Resources::ORE, Resources::CLAY, Resources::OBSIDIAN })
        {
            if (robots[r] < maxrobots[r]) return false;
        }

        return true;
    }

    const Resources& getMaxRobots() const
    {
        return maxrobots;
    }

private:

    void parse_cost(const std::string& cost)
    {
        BlueprintPart part;

        part.m_product = Resources::parse(cost, 5);

        auto i = cost.find("costs ");
        if (i == std::string::npos)
            throw std::invalid_argument("Blueprint contains not cost");

        i += 6;
        while (i != std::string::npos)
        {
            auto j = cost.find("and ", i);

            auto parts = cost.substr(i);

            std::size_t k = 0;
            auto amount = std::stoul(parts, &k);
            auto resource = Resources::parse(parts, k+1);

            if (j == std::string::npos)
            {
                i = j;
            }
            else
            {
                i = j+4;
            }

            part.m_cost[resource] = amount;
        }

        if (part.m_cost.empty())
            throw std::invalid_argument("Blueprint has no cost");

        maxrobots |= part.m_cost;
        m_products.emplace(part.m_product, part);
    }
};

const Resources Blueprint::nullresource{};

std::ostream& operator<<(std::ostream& os, const Blueprint& p)
{
    p.print(os);
    return os;
}

class Scenario
{
public:

    Scenario(const Blueprint& a_blueprint, std::size_t a_maxtime) : blueprint(a_blueprint), maxtime(a_maxtime)
    {
        geodes = std::async(std::launch::async, [&](){
            auto result = findAmountOfGeodes();
            return result;
        });
    }

    std::array<uint8_t, 9> getKey(std::size_t time, const Resources& amounts, const Resources& robots) const
    {
        std::array<uint8_t, 9> ret;

        std::size_t i = 0;

        ret[i++] = time & 0xff;

        for (auto& r : Resources::all)
        {
            ret[i++] = std::min(0xffUL, amounts[r]);
            ret[i++] = robots[r] & 0xff;
        }
        
        return ret;
    }

    std::size_t qualityLevel()
    {
        return geodes.get() * blueprint.m_id;
    }

    std::size_t maxGeodes()
    {
        return geodes.get();
    }

    std::size_t findAmountOfGeodes() const
    {
        Resources amounts;
        Resources robots;

        robots[Resources::ORE] = 1;
        std::size_t maxgeodes = 0;

        std::map<std::array<uint8_t, 9>, unsigned> seen2;

        return findAmountOfGeodes(maxtime, amounts, robots, maxgeodes, seen2);
    }

private:

    std::size_t findAmountOfGeodes(std::size_t time, const Resources& amounts, const Resources& robots, std::size_t& seen, std::map<std::array<uint8_t, 9>, unsigned>& seen2) const
    {
        if (time == 0)
            return amounts[Resources::GEODE];
     
        /* Determine what options we have per the blueprint and per our amount of ores */
        auto options = blueprint.getOptions(amounts, robots);

        if (blueprint.hasMaxRobots(robots))
        {
            /* When the max amount of robots is reached, it should be possible 
             * to calculate the remaining yield. In that case we can produce 
             * a geode cracker every cycle for the remainder of our time.
             */

            std::size_t res = 0;
            std::size_t geoders = robots[Resources::GEODE];
           
            while (time > 0)
            {
                res += geoders * time;
                geoders += 1;
                time--;
            }
         
            return res;
        }

        auto key = getKey(time, amounts, robots);

        /* If for every remaining cycle we would be able to buy geoders and we don't exceed the max already seen, lets drop it */
        {
            std::size_t limit = amounts[Resources::GEODE];
            std::size_t geoders = robots[Resources::GEODE];
           
            std::size_t tr = time;
            while (tr > 0)
            {
                limit += geoders * tr;
                geoders += 1;
                tr--;
            }

            if (limit < seen)
            {
                seen2.emplace(key, 0);
                return 0;
            }
        }
        
        /* Checked if this pattern has been seen before */
        auto cached = seen2.find(key);
        if (cached != seen2.end())
        {
            return cached->second;
        }

        std::size_t best_result = 0;

        for (auto& option : options)
        {
            auto new_amounts = amounts + robots;
            auto new_robots(robots);

            option.second->produce(new_amounts, new_robots);

            auto result = findAmountOfGeodes(time-1, new_amounts, new_robots, seen, seen2);
            if (result > best_result)
                best_result = result;
        }

        {
            /* Also do a simulation where we buy nothing */
            auto new_amounts = amounts + robots;
           
            auto result = findAmountOfGeodes(time-1, new_amounts, robots, seen, seen2);
            if (result > best_result)
                best_result = result;
        }

        if (best_result > seen)
        {
            seen = best_result;
        }

        seen2.emplace(key, best_result);

        return best_result;
    }

    std::size_t maxgeodes;

    Blueprint blueprint;
    std::size_t maxtime;
    std::future<std::size_t> geodes;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    std::vector<Blueprint> blueprints;
    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            if (! line.empty())
            {
                blueprints.emplace_back(line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    std::vector<std::unique_ptr<Scenario>> scenarios;

    /* For part a, run all the scenario's in a separate thread */
    for (auto &bp: blueprints)
    {
        scenarios.emplace_back(std::make_unique<Scenario>(bp, 24));
    }

    std::size_t resulta = 0;
    for (auto &sc: scenarios)
    {
        resulta += sc->qualityLevel();
    }

    std::cout << "Part a " << resulta << std::endl;
    
    std::size_t resultb = 1;
    /* For part b, run all the scenario's 1-by-1 since the cache can
     * grow quite large.
     */
    for (std::size_t i=0; i<std::min(3UL, blueprints.size()); ++i)
    {
        auto sc = std::make_unique<Scenario>(blueprints[i], 32);
        resultb *= sc->maxGeodes();
    }

    std::cout << "Part b " << resultb << std::endl;

    return 0;
}
