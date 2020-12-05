#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>
#include <utility>
#include <limits>

struct Seat
{
    Seat(unsigned row, unsigned column) : m_Row(row), m_Column(column) {};

    operator unsigned() const { return id(); };
    unsigned id() const { return (m_Row*8) + m_Column; }

    unsigned m_Row;
    unsigned m_Column;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    unsigned highest_seat_id = 0;

    std::map<unsigned,std::bitset<8>> seatmap;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            unsigned row = 0;
            unsigned column = 0;

            for (auto c : line)
            {
                switch(c)
                {
                    case 'F':
                        row = (row << 1);
                        break;
                    case 'B':
                        row = (row << 1) | 1;
                        break;
                    case 'L':
                        column = (column << 1);
                        break;
                    case 'R':
                        column = (column << 1) | 1;
                        break;
                    case '\n':
                    default:
                        break;
                }
            }

            Seat seat(row, column);

            highest_seat_id = std::max(seat.id(), highest_seat_id);

            seatmap[row].set(column);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    std::cout << "Highest seat id is " << highest_seat_id << std::endl;

    auto row_bounds = std::minmax_element(
        seatmap.begin(), seatmap.end(),
        [](const decltype(seatmap)::value_type& p1, const decltype(seatmap)::value_type& p2)
        {
            return p1.first < p2.first;
        }
    );

    if ((row_bounds.first == seatmap.end()) || (row_bounds.second == seatmap.end()))
    {
        throw std::logic_error("No tickets found");
    }

    std::cout << "First row is " << row_bounds.first->first << std::endl;
    std::cout << "Last row is " << row_bounds.second->first << std::endl;

    for (auto& row : seatmap)
    {
        if (row.second.none())
            // empty row
            continue;

        if (! row.second.all())
        {
            if (row.first == row_bounds.first->first )
                // first row, allowed to have empty seats
                continue;

            if (row.first == row_bounds.second->first )
                // last row, allowed to have empty seats
                continue;

            std::cout << "Row " << row.first << " has missing seats : " << row.second << std::endl;

            for (size_t column = 0; column < row.second.size(); ++column)
            {
                if (! row.second[column])
                {
                    Seat seat(row.first, column);

                    std::cout << "Your seat is " << seat << std::endl;
                    return 0;
                }
            }
        }
    }

    return 0;
}
