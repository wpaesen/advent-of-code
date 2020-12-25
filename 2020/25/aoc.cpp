#include <string>
#include <array>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include <set>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <future>
#include <forward_list>
#include <list>

uint64_t findLoopSize(uint64_t subject, uint64_t value)
{
    uint64_t res = 1;
    for (uint64_t i = 1; ; ++i)
    {
        res = (res*subject) % 20201227;
        if (res == value) return i;
    }
}

uint64_t transform(uint64_t subject, uint64_t loop)
{
    uint64_t res = 1;

    for (uint64_t i=0; i <loop; ++i)
    {
        res = (res*subject)%20201227;
    }

    return res;
}

int
main(int argc, char **argv)
{
    std::array<int64_t,2> pubkeys;

    if ((argc >= 2) && (std::string(argv[1]) == "--test"))
    {
        std::cout << "Running example test cases" << std::endl;
        pubkeys[0] = 5764801;
        pubkeys[1] = 17807724;
    }
    else
    {
        pubkeys[0] = 10943862;
        pubkeys[1] = 12721030;
    }

    std::array<int64_t, 2> loopsizes;
    for (size_t i=0; i<pubkeys.size(); ++i)
    {
        loopsizes[i] = findLoopSize(7, pubkeys[i]);
        std::cout << "Loop size " << i+1 << " : " << loopsizes[i] << std::endl;
    }

    std::array<int64_t, 2> encryptionkeys;
    for (size_t i=0; i<pubkeys.size(); ++i)
    {
        encryptionkeys[i] = transform(pubkeys[i], loopsizes[(i+1)%pubkeys.size()]);
        std::cout << "Encryption key " << i+1 << " : " << encryptionkeys[i] << std::endl;
    }

    return 0;
}
