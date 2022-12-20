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
#include <functional>

class Node
{
public:
    Node(int64_t a_value, int64_t a_operations) : value(a_value), operations(a_operations), position(0), next(nullptr), prev(nullptr) {}
    Node(const Node& other) : value(other.value), operations(other.operations), position(other.position), next(nullptr), prev(nullptr) {};

    operator Node*() { return this; };

    int64_t getValue() const { return value; };
    int64_t getOperations() const { return operations; }

    Node* next;
    Node* prev;

    void setPosition(int pos) { position = pos; }
    int getPosition() const { return position; }

    void skipForward()
    {
        Node* LB = prev;
        Node* UB = next->next;
        Node* other = next;

        LB->next = other;

        other->prev = LB;
        other->next = this;

        prev = other;
        next = UB;

        UB->prev = this;
    }

    void setNext(Node *nnext)
    {
        next = nnext;
        if (next)
            next->prev = this;
    }

private:

    int64_t value;
    int64_t operations;
    int position;
    bool root;
};

class Work
{
public:
    Work(const std::vector<int64_t>& numbers, int64_t decryption_key = 1) {
        for (auto& n : numbers)
        {
            int64_t val = n * decryption_key;
            int64_t operations = val;
            int64_t size = (int64_t)numbers.size();

            while ((operations < 0) || (operations >= size))
            {
                int64_t rounds = operations / (size-1);
                operations -= rounds * (size-1);
                if (operations < 0)
                {
                    operations += (size-1);
                }
            }

            addNode(val, operations);
        }
    };

    void print(std::ostream& os) const
    {
        for (auto& node : nodes)
        {
            os << reinterpret_cast<const void*>(&node) << " : " << node.getValue() << ", next=" << reinterpret_cast<const void*>(node.next) << ", prev=" << reinterpret_cast<const void*>(node.prev);
            os << std::endl;
        }
    }

    const Work& decrypt(std::size_t n_rounds = 1)
    {
        for (auto i = nodes.begin(); i != std::prev(nodes.end()); i = std::next(i))
        {
            auto j = std::next(i);

            i->setNext(*j);
        }
        nodes.rbegin()->setNext(*nodes.begin());

        for (std::size_t round = 0; round < n_rounds; ++round)
        {
            for (auto& n : nodes)
            {
                for (int i=0; i<n.getOperations(); ++i)
                {
                    n.skipForward();
                }
            }
        }

        std::vector<Node> result;
        result.reserve(nodes.size());

        for (auto i = getRoot(); result.size() < nodes.size(); i = i->next)
        {
            result.emplace_back(*i);
        }

        std::swap(nodes, result);

        return *this;
    }
        
    std::array<int64_t, 3> getCoordinates() const
    {
        std::array<int64_t, 3> ret;

        for (std::size_t i=0; i<3; ++i)
        {
            ret[i] = nodes[((i+1) *1000) % nodes.size()].getValue();
        }

        return ret;
    }

    int64_t getCoordinatesSum() const
    {
        int64_t ret = 0;
        for (auto& c : getCoordinates())
        {
            ret += c;
        }

        return ret;
    }

private:

    void addNode(int64_t value, int64_t operations) 
    {
        nodes.emplace_back(value, operations);
    }

    Node* getRoot()
    {
        const Node* root = nullptr;
        for (auto& n : nodes)
        {
            if (n.getValue() == 0)
            {
                return n;
            }
        }

        throw std::logic_error("No root");

        return nullptr;
    }

    std::vector<Node> nodes;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    std::vector<int64_t> numbers;
    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            if (! line.empty())
            {
                numbers.push_back(std::stoi(line));
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    std::cout <<"Cosum A " << Work(numbers).decrypt().getCoordinatesSum() << std::endl;
    std::cout <<"Cosum B " << Work(numbers, 811589153).decrypt( 10 ).getCoordinatesSum() << std::endl;

    return 0;
}
