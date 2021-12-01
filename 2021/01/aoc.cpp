#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <vector>
#include <cstdint>
#include <fstream>

template<std::size_t N>
class Window
{
public:

    Window() : sum(0), idx(0)
    {
        values.fill(0);
    } 

    void push(size_t value)
    {
        sum_last = sum;

        size_t i = (idx++) % N;

        sum -= values[i];
        values[i] = value;
        sum += values[i];
    }

    bool isIncreased() const
    {
        return sum > sum_last;
    }

    bool isComplete() const
    {
        return (idx >= N);
    }
   
private:

    std::array<size_t, N> values;

    size_t sum;
    size_t sum_last;

    size_t idx;
};

template<std::size_t N>
std::size_t
get_measurement_trend( const std::vector<uint64_t>& measurements )
{
    std::size_t n_increases = 0;
    Window<N> w;
    auto i = measurements.cbegin();

    for ( size_t j = 0; ( ! w.isComplete() ) && ( i!= measurements.cend() ); ++j, ++i)
    {
        w.push(*i);
    }

    for ( ; i != measurements.cend(); ++ i)
    {
        w.push(*i);
        if ( w.isIncreased() ) n_increases++;
    }

    return n_increases;
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<uint64_t> measurements;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            measurements.emplace_back(std::stoi(line));
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    std::cout << "A : " << get_measurement_trend<1>( measurements ) << " measurements where larger than their previous" << std::endl;
    std::cout << "B : " << get_measurement_trend<3>( measurements ) << " 3-windowed measurements where larger than their previous" << std::endl;

    return 0;
}
