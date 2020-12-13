#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>

struct Ship
{
    Ship() : m_Heading(0), m_Longitude(0), m_Latitude(0) {};

    void step(char action, int param )
    {
        switch(action)
        {
            case 'N':
                /*  Action N means to move north by the given value. */
                m_Longitude += (double)param;
                break;
            case 'S':
                /* Action S means to move south by the given value. */
                m_Longitude -= (double)param;
                break;
            case 'E':
                /* Action E means to move east by the given value. */
                m_Latitude += (double)param;
                break;
            case 'W':
                /* Action W means to move west by the given value.*/
                m_Latitude -= (double)param;
                break;
            case 'L':
                /* Action L means to turn left the given number of degrees. */
                m_Heading += (double)param;
                break;
            case 'R':
                /* Action R means to turn right the given number of degrees. */
                m_Heading -= (double)param;
                break;
            case 'F':
                /* Action F means to move forward by the given value in the direction the ship is currently facing. */
                m_Longitude += std::sin(m_Heading * M_PI / 180) * (double)param;
                m_Latitude += std::cos(m_Heading * M_PI / 180) * (double)param;
                break;
            default:
                throw std::invalid_argument("Unknown instruction");
                break;
        }
    }

    double getMd()
    {
        return std::abs(m_Longitude) + std::abs(m_Latitude);
    }

    void print()
    {
        std::cout << "At(" << m_Latitude << "," << m_Longitude << "), Hdng(" << m_Heading << ")" << std::endl;

    }

    double m_Heading;
    double m_Longitude;  /* N-S */
    double m_Latitude;   /* E-W */
};

struct Nav
{
    Nav() : m_Longitude(0), m_Latitude(0), m_WpLongitude(1), m_WpLatitude(10) {};

    void step(char action, int param )
    {
        double wplo = m_WpLongitude;
        double wpla = m_WpLatitude;

        switch(action)
        {
            case 'N':
                /* Action N means to move the waypoint north by the given value. */
                m_WpLongitude += (double)param;
                break;
            case 'S':
                /* Action S means to move the waypoint south by the given value. */
                m_WpLongitude -= (double)param;
                break;
            case 'E':
                /* Action E means to move the waypoint east by the given value. */
                m_WpLatitude += (double)param;
                break;
            case 'W':
                /* Action W means to move the waypoint west by the given value. */
                m_WpLatitude -= (double)param;
                break;
            case 'L':
                /* Action L means to rotate the waypoint around the ship left (counter-clockwise) the given number of degrees. */
                switch (param)
                {
                    case 90:
                        m_WpLatitude = -wplo;
                        m_WpLongitude = wpla;
                        break;
                    case 180:
                        m_WpLatitude = -wpla;
                        m_WpLongitude = -wplo;
                        break;
                    case 270:
                        m_WpLatitude = wplo;
                        m_WpLongitude = -wpla;
                        break;
                    default:
                        throw std::invalid_argument("Unknown instruction");
                }
                break;
            case 'R':
                /* Action R means to rotate the waypoint around the ship right (clockwise) the given number of degrees. */
                switch (param)
                {
                    case 90:
                        m_WpLatitude = wplo;
                        m_WpLongitude = -wpla;
                        break;
                    case 180:
                        m_WpLatitude = -wpla;
                        m_WpLongitude = -wplo;
                        break;
                    case 270:
                        m_WpLatitude = -wplo;
                        m_WpLongitude = wpla;
                        break;
                    default:
                        throw std::invalid_argument("Unknown instruction");
                }
                break;
            case 'F':
                /* Action F means to move forward to the waypoint a number of times equal to the given value. */
                m_Longitude += m_WpLongitude * (double)param;
                m_Latitude += m_WpLatitude * (double)param;
                break;
            default:
                throw std::invalid_argument("Unknown instruction");
                break;
        }
    }

    void rotate(double angle)
    {

        /* turn x y into distance / angle */
        double distance = std::sqrt( std::pow(m_WpLongitude, 2) + std::pow(m_WpLatitude, 2) );
        double heading = std::atan( m_WpLongitude / m_WpLatitude );

        if (m_WpLatitude < 0) heading += 180;

        heading += angle * M_PI / 180;

        m_WpLongitude = std::sin(heading) * distance;
        m_WpLatitude = std::cos(heading) * distance;
    }


    double getMd()
    {
        return std::abs(m_Longitude) + std::abs(m_Latitude);
    }

    void print()
    {
        std::cout << "At(" << m_Latitude << "," << m_Longitude << "), Hdng(" << m_WpLongitude << "," << m_WpLatitude << ")" << std::endl;

    }

    double m_Longitude;   /* N-S */
    double m_Latitude;    /* E-W */
    double m_WpLongitude; /* N-S */
    double m_WpLatitude;  /* E-W */
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<std::pair<char,int>> sequence;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_op("^([NSEWLRF])([0-9]+)$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_op))
            {
                sequence.emplace_back(match[1].str()[0], std::stol(match[2].str()));
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    Ship s;
    for (auto& i: sequence)
    {
        s.step(i.first, i.second);
        // std::cout << "I(" << i.first << "," << i.second << ") ";
        // s.print();
    }

    std::cout << "Manhatten : " << s.getMd() << std::endl;

    Nav n;
    for (auto& i: sequence)
    {
        n.step(i.first, i.second);
        // std::cout << "I(" << i.first << "," << i.second << ") ";
        // n.print();
    }

    std::cout << "Manhatten 2: " << n.getMd() << std::endl;

    return 0;
}
