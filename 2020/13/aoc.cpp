#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<std::pair<char,int>> sequence;

    int64_t offset = 0;
    std::vector<int64_t> busses;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        if  (!(std::getline(infile, line)))
        {
            throw std::out_of_range("No offset");
        }

        offset = std::stol(line);

        if  (!(std::getline(infile, line)))
        {
            throw std::out_of_range("No table");
        }

        size_t pos = 0;
        while ((pos = line.find(",")) != std::string::npos )
        {
            std::string token = line.substr(0, pos);
            line.erase(0, pos + 1);

            if (token != "x")
            {
                busses.emplace_back( std::stol(token) );
            }
            else
            {
                busses.emplace_back( 0 );
            }
        }
        if (line != "x")
        {
            busses.emplace_back( std::stol(line) );
        }
        else
        {
            busses.emplace_back( 0 );
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    int64_t earliest = -1;
    int64_t earliest_time = -1;

    std::cout << "Time offset : " << offset << std::endl;
    std::cout << "Buses : " << std::endl;
    for (auto b : busses)
    {
        if (b != 0)
        {
            std::array<char, 64> pbuf;

            std::snprintf(pbuf.data(), pbuf.size(), "% 4ld", b);

            std::cout << " - " << pbuf.data() << " next at ";

            int64_t nexttime = offset - (offset % b) + b;
            std::cout << nexttime << std::endl;

            if ((nexttime < earliest_time) || (earliest_time == -1))
            {
                earliest = b;
                earliest_time = nexttime;
            }
        }
    }

    if (earliest == -1)
    {
        throw std::out_of_range("No solution");
    }

    std::cout << std::endl << "First bus is " << earliest << " at " << earliest_time << std::endl;

    int64_t answer = (earliest_time - offset) * earliest;

    std::cout << "Answer 1 is " << answer << std::endl << std::endl;

    /* Create an array of busses in the list that exist, coupled with their position in the
     * list
     */
    std::vector<std::pair<int64_t, int64_t>> busses2;
    for ( size_t i = 0; i <busses.size(); ++i )
    {
        if (busses[i] != 0)
        {
            busses2.emplace_back(busses[i], i);
        }
    }

    /* Sort the array on high to low bus id. */
    std::sort( busses2.begin(), busses2.end(),
            [](const std::pair<int64_t, int64_t>&a, const std::pair<int64_t, int64_t>&b){
                    return a.first > b.first;
            }
    );

    /* Adjust the offsets based on the position of the highest bus id in the list.
     * Keep the offset because we need it at the end to correct the calculated time.
     */
    int64_t offset2 = busses2[0].second;
    for (auto& b : busses2)
    {
        b.second -= offset2;
    }

    /* Offset for the calculation based on the hint given in the example. */
    constexpr int64_t thefuture = 100000000000000;

    /* Find the latest time smaller then the time offset that matches the longest tour */
    int64_t scan = thefuture - (thefuture % busses2[0].first);

    /* Build an array of bus Id's based on the offset2 list (in the high to low order)
     * We seek the factors bas */
    std::vector<int64_t> factors;
    for (auto& b: busses2)
    {
        factors.emplace_back(b.first);
    }

    int64_t factor = factors[0];
    factors.erase(factors.begin());

    bool mismatch = false;
    do
    {
        mismatch = false;
        size_t n = 0;
        scan += factor;

        for (auto& b : busses2)
        {
            if (++n == 1) continue;

            if (((scan + b.second) % b.first) != 0)
            {
                mismatch = true;
                break;
            }

            if (n > (busses2.size() - factors.size()))
            {
                factor *= factors[0];
                factors.erase(factors.begin());

                std::cout << n << " matches at " << (scan - offset2) << std::endl;
            }
        }

    } while (mismatch);

    /* Undo the offsetting we did at the beginning. */
    int64_t answer2 = scan - offset2;

    std::cout << std::endl <<  "Answer 2 is " << answer2 << std::endl;

    return 0;
}
