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

class CrabBlaster
{
public:
    CrabBlaster() {};

    void add(int position)
    {
        crabs.emplace_back(position);
    }

    bool prepare()
    {
        if (crabs.empty())
        {
            throw std::invalid_argument("Invalid initial state");
        }

        fuelcosts.clear();        

        std::sort( crabs.begin(), crabs.end() );

        int last = 0;
        for (int i=0; i<= (*crabs.crbegin() - *crabs.cbegin()); ++i)
        {
            last += i;
            fuelcosts.emplace_back(last);
        }

        return true;
    }

    int lowestFuelCost()
    {        
        int cost = std::numeric_limits<int>::max();

        for (auto i = *crabs.cbegin(); i <= *crabs.crbegin(); ++i)
        {
            int tc = fuelCostLinear(i);
            cost = std::min(tc, cost);
        }

        return cost;
    }

    int lowestFuelCostCrablogic()
    {
        int cost = std::numeric_limits<int>::max();

        for (auto i = *crabs.cbegin(); i <= *crabs.crbegin(); ++i)
        {
            int tc = fuelCostCrablogic(i);
            cost = std::min(tc, cost);
        }

        return cost;
    }

private:
    
    std::vector<int> crabs;    
    std::vector<int> fuelcosts;

    int fuelCostLinear(int position)
    {
        int ret = 0;
        for (auto& c : crabs)
        {
            ret += std::abs(position - c);
        }

        return ret;
    }

    int fuelCostCrablogic(int position)
    {
        int ret = 0;
        for (auto& c : crabs)
        {
            ret += fuelcosts.at(std::abs(position - c));
        }

        return ret;
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

    CrabBlaster blaster;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line, ','))
        {
            blaster.add(std::stoi(line));
        }

        blaster.prepare();
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    std::cout << "Lowest fuel cost a " << blaster.lowestFuelCost() << std::endl;
    std::cout << "Lowest fuel cost b " << blaster.lowestFuelCostCrablogic() << std::endl;

    return 0;
}
