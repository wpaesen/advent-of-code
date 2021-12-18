#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
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

class Number : public std::enable_shared_from_this<Number>
{
public:

    Number() {};

    virtual int64_t magnitude() const = 0;
    virtual std::shared_ptr<Number> clone(int depth=0) const = 0;
   
    virtual bool add_explode(int side, int64_t value) = 0;

    static std::shared_ptr<Number> parse(const std::string& line)
    {
        auto i = line.cbegin();
        while ((i != line.cend()) && (*i != '[')) i++;

        if (i == line.cend())
        {
            throw std::invalid_argument("Syntax error");
        }

        i++;

        if (i == line.cend())
        {
            throw std::invalid_argument("Syntax error");
        }

        return parse(i, line.cend());
    }

    
    static std::shared_ptr<Number> parse(std::string::const_iterator& pos, const std::string::const_iterator& end );
    
    enum Action
    {
        NONE,
        RESTART,
        SPLIT,
        EXPLODE_LR,
        EXPLODE_L,
        EXPLODE_R,
    };

    static std::shared_ptr<Number> reduce( std::shared_ptr<Number> n );

    virtual std::tuple<Action, int64_t, int64_t> reduce(int depth = 0, int step = 0) = 0;

    virtual std::ostream& write(std::ostream& s) const = 0;
};

std::ostream& operator<< (std::ostream& s, std::shared_ptr<Number>& n)
{
    return n->write(s);
};

std::ostream& operator<< (std::ostream& s, const Number& n)
{
    return n.write(s);
};

class Pair : public Number
{
public:
    Pair( std::shared_ptr<Number> a, std::shared_ptr<Number> b )
    {
        children[0] = a;
        children[1] = b;
    }

    int64_t magnitude() const override 
    {
        return (3*children[0]->magnitude()) + (2*children[1]->magnitude());
    }

    std::shared_ptr<Number> clone(int depth=0) const override 
    {
        return std::make_shared<Pair>(children[0]->clone(), children[1]->clone());
    }

    bool add_explode(int side, int64_t a_value) override
    {
        return children.at(side)->add_explode(side, a_value);
    };

    std::ostream& write(std::ostream& s) const override
    {
        s << "[";
        children[0]->write(s);
        s << ",";
        children[1]->write(s);
        s << "]";

        return s;
    };

private:

    std::array<std::shared_ptr<Number>, 2> children;

public:

    std::tuple<Number::Action, int64_t, int64_t> reduce(int depth = 0, int step = 0) override;
};

class Value : public Number
{
public:

    Value(int64_t a_value) : value(a_value) {};

    int64_t magnitude() const override 
    {
        return value;
    }

    std::shared_ptr<Number> clone(int depth=0) const override 
    {
        return std::make_shared<Value>(value);
    }

    bool add_explode(int side, int64_t a_value) override
    {
        value += a_value;
        return true;
    };

    std::ostream& write(std::ostream& s) const override
    {
        s << value;
        return s;
    };

private:
    int64_t value;

public:

    std::tuple<Action, int64_t, int64_t> reduce(int depth = 0, int step = 0) override
    {
        if ((step == 1) && (value >= 10))
        {
            int64_t a = value >> 1;
            int64_t b = value - a;

            return std::make_tuple(Number::SPLIT, a, b);
        }
        else
        {
            return std::make_tuple(Number::NONE, 0, 0);
        }
    }
};

std::shared_ptr<Number> Number::reduce( std::shared_ptr<Number> n )
{
    Number::Action action;
    int64_t left;
    int64_t right;
   
    do
    {
        std::tie(action, left, right) = n->reduce(0, 0);
        if (action == NONE)
        {
            std::tie(action, left, right) = n->reduce(0, 1);
        }

        switch(action)
        {
        case SPLIT:
            n = std::make_shared<Pair>(
                std::make_shared<Value>(left),
                std::make_shared<Value>(right)
            );
            break;
        default:
            break;
        }

    } while( action != NONE);

    return n;
}

std::tuple<Number::Action, int64_t, int64_t> Pair::reduce(int depth, int step)
{
    if ((step == 0)&&(depth == 4))
    {
        // std::cout << "EXPLODE(" << children[0]->magnitude() << "," << children[1]->magnitude() << "),";

        return std::make_tuple(
            Number::EXPLODE_LR,
            children[0]->magnitude(),
            children[1]->magnitude()
        );
    }
    else
    {
        for (auto i = children.begin(); i != children.end(); ++i)
        {
            Number::Action action;
            int64_t left;
            int64_t right;

            std::tie(action, left, right) = (*i)->reduce(depth+1, step);

            switch(action)
            {
            case Number::SPLIT:
                *i = std::make_shared<Pair>(
                    std::make_shared<Value>(left),
                    std::make_shared<Value>(right)
                );

                return std::make_tuple(Number::RESTART, 0, 0);     
            case Number::EXPLODE_LR:
                *i = std::make_shared<Value>(0);

                if (i == children.begin())
                {
                    children[1]->add_explode(0, right);
                    return std::make_tuple(Number::EXPLODE_L, left, right);
                }
                else
                {
                    children[0]->add_explode(1, left);
                    return std::make_tuple(Number::EXPLODE_R, left, right);
                }
                break;
            case Number::EXPLODE_L:
                if (i == children.begin())
                {
                    return std::make_tuple(Number::EXPLODE_L, left, right);
                }
                else
                {
                    children[0]->add_explode(1, left);
                    return std::make_tuple(Number::RESTART, 0, 0 );
                }
                break;
            case Number::EXPLODE_R:
                if (i == children.begin())
                {
                    children[1]->add_explode(0, right);
                    return std::make_tuple(Number::RESTART, 0, 0 );
                }
                else
                {
                    return std::make_tuple(Number::EXPLODE_R, left, right);
                }
                break;
            case Number::RESTART:
                return std::make_tuple(Number::RESTART, 0, 0);
            case Number::NONE:
                break;
            }
        }
    }

     return std::make_tuple(Number::NONE, 0, 0);
}

std::shared_ptr<Number> Number::parse(std::string::const_iterator& pos, const std::string::const_iterator& end)
{
    std::array<std::shared_ptr<Number>, 2> parts;

    if (pos == end)
    {
        throw std::invalid_argument("Syntax error");
    }

    if ( *pos == '[' )
    {
        pos++;
        parts[0] = parse(pos, end);

    } else if (( *pos >= '0') && ( *pos <= '9' ))
    {
        parts[0] = std::make_shared<Value>( *pos - '0' );
        pos++;
    }
    else
    {
        throw std::invalid_argument("Syntax error");
    }

    if (*pos != ',')
    {
        throw std::invalid_argument("Syntax error");
    }
    pos++;

    if ( *pos == '[' )
    {
        pos++;
        parts[1] = parse(pos, end);

    } else if (( *pos >= '0') && ( *pos <= '9' ))
    {
        parts[1] = std::make_shared<Value>( *pos - '0' );
        pos++;
    }
    else
    {
        throw std::invalid_argument("Syntax error");
    }

    if (*pos != ']')
    {
        throw std::invalid_argument("Syntax error");
    }
    pos++;
   
    return std::make_shared<Pair>( parts[0], parts[1] );
}

std::shared_ptr<Number> operator+( std::shared_ptr<Number> lhs, std::shared_ptr<Number> rhs )
{
    if ((lhs) && (rhs))
    { 
        std::shared_ptr<Pair> ret = std::make_shared<Pair>(lhs->clone(), rhs->clone());
        return Number::reduce(ret);
    }
    if (lhs)
    {
        return lhs->clone();
    }
    if (rhs)
    {
        return rhs->clone();
    }

    return std::shared_ptr<Number>();
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<std::shared_ptr<Number>> homework;

    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            homework.emplace_back( Number::parse(line) );
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        exit(-1);
    }

    if (homework.empty())
    {
        std::cerr << "Bad dog !" << std::endl;
        exit(-1);
    }

    std::shared_ptr<Number> sum;

    for (auto i = homework.begin(); i != homework.end(); ++i)
    {
        sum = sum + *i;
    }

    std::cout << "Cumulative magnitude " << sum->magnitude() << std::endl;

    int64_t largest = 0;
    for (auto i = homework.begin(); ( i+1) != homework.end(); ++i)
    {
        for (auto j = i+1; j != homework.end(); ++ j)
        {
            sum = *i + *j;
            if (sum->magnitude() > largest)
            {
                largest = sum->magnitude();
            }

            sum = *j + *i;
            if (sum->magnitude() > largest)
            {
                largest = sum->magnitude();
            } 
        }
    }

    std::cout << "Largest sum " << largest << std::endl;    

    return 0;
}
