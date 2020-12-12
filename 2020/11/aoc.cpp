#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>

struct Seat
{
    enum State
    {
        EMPTY,
        OCCUPIED,
        FLOOR,
        OFFMAP,
    };

    Seat() : m_State(FLOOR), m_NextState(FLOOR)
    {
        m_Neighbours.fill(nullptr);
    };

    Seat(State a_State) : m_State(a_State), m_NextState(a_State)
    {
        m_Neighbours.fill(nullptr);
    }

    Seat(const Seat& other) : m_State(other.m_State), m_NextState(other.m_State)
    {
        m_Neighbours.fill(nullptr);
    }

    State m_State;
    State m_NextState;

    std::array<Seat*,8> m_Neighbours;

    void CalculateNext(unsigned tolerance)
    {
        m_NextState = m_State;
        if ((m_State == FLOOR) || (m_State == OFFMAP))
            return;

        if (m_State == EMPTY)
        {
            m_NextState = OCCUPIED;

            for (auto s : m_Neighbours)
            {
                if (s->IsOccupied())
                {
                    m_NextState = EMPTY;
                    break;
                }
            }
        }
        else
        {
            unsigned n_o = 0;

            for (auto s : m_Neighbours)
            {
                if (s->IsOccupied()) n_o++;
            }

            if (n_o >= tolerance)
            {
                m_NextState = EMPTY;
            }
            else
            {
                m_NextState = OCCUPIED;
            }
        }
    }

    void Setup()
    {
        for (int i=0; i<8; ++i)
        {
            while ((m_Neighbours[i]) && (m_Neighbours[i]->m_State == FLOOR))
            {
                m_Neighbours[i] = m_Neighbours[i]->m_Neighbours[i];
            }
        }
    }

    bool Commit()
    {
        if (m_State != m_NextState)
        {
            m_State = m_NextState;
            return true;
        }

        return false;
    }

    bool IsOccupied()
    {
        return m_State == OCCUPIED;
    }
};

struct SeatMap
{
    SeatMap()
        : m_VirtualEmptySeat(Seat::OFFMAP)
    {};

    SeatMap(const SeatMap& other)
        : m_VirtualEmptySeat(Seat::OFFMAP)
    {

        for (auto& r : other.m_Floor)
        {
            m_Floor.emplace_back();

            std::vector<Seat>& row = *m_Floor.rbegin();

            for (auto& s: r)
            {
                row.emplace_back(s);

            }
        }
    };

    void add_row(const std::string& r)
    {
        m_Floor.emplace_back();

        std::vector<Seat>& row = *m_Floor.rbegin();

        for (auto c: r)
        {
            switch (c)
            {
                case 'L':
                    row.emplace_back(Seat::EMPTY);
                    break;
                case '.':
                    row.emplace_back(Seat::FLOOR);
                    break;
                default:
                    throw std::range_error("Invalid seat code");
            }
        }
    }

    Seat* Resolve(int r, int c)
    {
        try
        {
            return & m_Floor.at(r).at(c);
        }
        catch(std::out_of_range&)
        {
            return &m_VirtualEmptySeat;
        }
    }

    void Setup()
    {
        for (int r = 0; r < m_Floor.size(); ++r)
        {
            std::vector<Seat>& row = m_Floor.at(r);

            for (int c = 0; c < row.size(); ++c)
            {
                Seat& s = row.at(c);

                s.m_Neighbours[0] = Resolve(r-1, c-1);
                s.m_Neighbours[1] = Resolve(r-1, c);
                s.m_Neighbours[2] = Resolve(r-1, c+1);
                s.m_Neighbours[3] = Resolve(r  , c-1);
                s.m_Neighbours[4] = Resolve(r  , c+1);
                s.m_Neighbours[5] = Resolve(r+1, c-1);
                s.m_Neighbours[6] = Resolve(r+1, c);
                s.m_Neighbours[7] = Resolve(r+1, c+1);
            }
        }

    }

    void Setup2()
    {
        for (int r = 0; r < m_Floor.size(); ++r)
        {
            std::vector<Seat>& row = m_Floor.at(r);

            for (int c = 0; c < row.size(); ++c)
            {
                Seat& s = row.at(c);
                s.Setup();
            }
        }
    }

    std::vector<std::vector<Seat>> m_Floor;
    Seat m_VirtualEmptySeat;

    bool iterate(unsigned tolerance)
    {
        bool ret = false;
        for (auto& r: m_Floor )
        {
            for (auto& s: r)
            {
                s.CalculateNext(tolerance);
            }
        }
        m_Occupied = 0;
        for (auto& r: m_Floor )
        {
            for (auto& s: r)
            {
                ret |= s.Commit();
                if (s.IsOccupied()) m_Occupied++;
            }
        }

        return ret;
    }

    void print()
    {
        for (auto& r: m_Floor )
        {
            for (auto& s: r)
            {
                switch (s.m_State)
                {
                    case Seat::EMPTY:
                        std::cout << "L";
                        break;
                    case Seat::FLOOR:
                        std::cout << ".";
                        break;
                    case Seat::OCCUPIED:
                        std::cout << "#";
                        break;

                }
            }

            std::cout << std::endl;
        }
    }

    unsigned m_Occupied;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    SeatMap smap;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^([.L]+)$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                smap.add_row(match[1].str());
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    /* Create a copy for the second part */
    SeatMap smap2(smap);

    /* Do the first assignment  */
    std::cout << "First assignment" << std::endl;
    smap.Setup();

    bool unstable = true;
    unsigned iterations = 0;
    do
    {
        unstable = smap.iterate(4);
        iterations++;

    } while (unstable);

    std::cout << "Map becomes stable after " << iterations << std::endl;
    std::cout << "We have " << smap.m_Occupied << " occupied seats" << std::endl;


    /* Do the second assignment */
    smap2.Setup();
    smap2.Setup2();

    std::cout << "Second assignment" << std::endl;
    unstable = true;
    iterations = 0;
    do
    {
        unstable = smap2.iterate(5);
        iterations++;

    } while (unstable);

    std::cout << "Map becomes stable after " << iterations << std::endl;
    std::cout << "We have " << smap2.m_Occupied << " occupied seats" << std::endl;

    return 0;
}
