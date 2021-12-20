#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <set>
#include <vector>
#include <cstdint>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <iomanip>
#include <list>
#include <chrono>
#include <cmath>
#include <string_view>
#include <tuple>

class Transform;

class Image
{
public:
    struct Dim
    {
        Dim() : left(0), right(0), top(0), bottom(0) {};

        int left;
        int right;
        int top;
        int bottom;

        void extend(int value = 1)
        {
            left   -= 1;
            right  += 1;
            top    -= 1;
            bottom += 1;
        }

        int width() const
        {
            return 1 + right - left;
        }

        int height() const
        {
            return 1 + bottom - top;
        }
    };

    Image() : darkpixel(false), darkvalue(0) {};

    Image(const Dim& a_dimensions, int increase)
        : dimensions(a_dimensions)
        , darkpixel(false)
        , darkvalue(0)
    {
        dimensions.extend(increase);

        /* Fill the array with zeros */
        data.resize(dimensions.height());
        for (auto & row : data)
        {
            row.resize(dimensions.width());
        }
    }

    void addScanLine(const std::string& a_line)
    {
        data.push_back(std::vector<bool>());
        std::vector<bool>& row = *data.rbegin();

        for (auto c : a_line)
        {
            row.push_back(c=='#');
        }

        dimensions.bottom = data.size()-1;
        dimensions.right = std::max((int)(row.size()-1), dimensions.right);
    }

    std::vector<std::vector<bool>> data;

    bool darkpixel;
    unsigned darkvalue;

    Dim dimensions;

    void print(std::ostream& s) const
    {
        for (int y = dimensions.top-5; y <= dimensions.bottom+5; ++y)
        {
            for (int x = dimensions.left-5; x <= dimensions.right+5; ++x)
            {
                char state = '.';
                if (getPixel(x, y))
                {
                    state = '#';
                }
                s << state;
            }
            s << std::endl;
        }
    }

    const Dim& getDimensions() const
    {
        return dimensions;
    }

    bool getPixel(int x, int y) const
    {
        if (x < dimensions.left) return darkpixel;
        if (y < dimensions.top) return darkpixel;
        if (x > dimensions.right) return darkpixel;
        if (y > dimensions.bottom) return darkpixel;

        auto& row = data[y-dimensions.top];

        return row[x-dimensions.left];
    }

    void setPixel(int x, int y, bool value)
    {
        if (x < dimensions.left) return;
        if (y < dimensions.top) return;
        if (x > dimensions.right) return;
        if (y > dimensions.bottom) return;

        auto& row = data[y-dimensions.top];
        row[x-dimensions.left] = value;
    }

    unsigned getValue(int x, int y) const
    {
        unsigned ret = 0;

        if (x < dimensions.left-1) return darkvalue;
        if (y < dimensions.top-1) return darkvalue;
        if (x > dimensions.right+1) return darkvalue;
        if (y > dimensions.bottom+1) return darkvalue;

        static constexpr std::array<std::pair<unsigned, std::pair<int, int>>, 9> kernel{{
            { 1<<8, {-1, -1 } },
            { 1<<7, { 0, -1 } },
            { 1<<6, { 1, -1 } },
            { 1<<5, {-1,  0 } },
            { 1<<4, { 0,  0 } },
            { 1<<3, { 1,  0 } },
            { 1<<2, {-1,  1 } },
            { 1<<1, { 0,  1 } },
            { 1<<0, { 1,  1 } }
        }};

        for (auto &o : kernel)
        {
            if (getPixel(x + o.second.first, y + o.second.second))
            {
                ret |= o.first;
            }
        }

        return ret;
    }

    unsigned getLitPixels()
    {
        if (darkpixel) return std::numeric_limits<unsigned>::max();

        unsigned ret = 0;

        for (auto& row : data)
        {
            for (auto cell : row)
            {
                if (cell) ret++;
            }
        }

        return ret;
    }

    Image enhance(const Transform& xfrm) const;
};


class Transform
{
public:
    Transform() {};
    
    void setAlgorithm(const std::string& a_algo)
    {
        algo = a_algo;
    }

    bool initialised() const { return ! algo.empty(); };
    
    void print(std::ostream& s) const
    {
        s << algo;
    }
    
    bool operator()(unsigned value) const
    {
        if (value >= algo.length()) 
        {
            return false;
        }

        return (algo[value] == '#');
    }

private:
    
    std::string algo;
};

Image Image::enhance(const Transform& xfrm) const
{
    auto dim = getDimensions();

    Image ret(dim, 1);

    ret.darkpixel = xfrm( darkvalue );
    if (ret.darkpixel)
    {
        ret.darkvalue = (1<<10)-1;
    }
    else
    {
        ret.darkvalue = 0;
    }

    for (int y=ret.dimensions.top; y<=ret.dimensions.bottom; ++y)
    {
        for (int x=ret.dimensions.left; x<=ret.dimensions.right; ++x)
        {
            ret.setPixel(x, y, xfrm( getValue(x, y) ) );
        }
    }

    return ret;
}

std::ostream& operator<<(std::ostream& s, const Image& t)
{
    t.print(s); return s;
}

std::ostream& operator<<(std::ostream& s, const Transform& t)
{
    t.print(s); return s;
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    Image sourceImage;
    Transform algorithm;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            if (line.empty()) continue;

            if (! algorithm.initialised())
            {
                algorithm.setAlgorithm(line);
            }
            else
            {
                sourceImage.addScanLine(line);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    Image ni = sourceImage.enhance(algorithm);
    for (int i=2; i<51; ++i)
    {
        ni = ni.enhance(algorithm);
        if ((i==2) || (i==50))
        {
            std::cout << "after " << i << " iterations : " << ni.getLitPixels() << " Pixels lighting up" << std::endl;
        }
    }

    return 0;
}
