#include <array>
#include <bits/c++config.h>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
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
#include <optional>
#include <string_view>

static constexpr bool with_debug = false;

using namespace std::string_view_literals;

class InstructionList;
class Node
{
public:
    Node(const std::string& name, const std::string& left, const std::string& right) : m_Name(name), m_Left(left), m_Right(right)
    {
        m_Final = (name.compare("ZZZ") == 0);
        m_GhostFinal = (*name.rbegin() == 'Z');

        m_NextNodes.fill(nullptr);
    }

    std::ostream& write(std::ostream& s) const
    {
        s << m_Name << " = (" << m_Left;
        if (m_NextNodes[0] == nullptr) {
            s << "!";
        }
        s << ", " << m_Right;
        if (m_NextNodes[1] == nullptr) {
            s << "!";
        }
        s << ")";
        return s;
    }

    const std::string& getName() const
    {
        return m_Name;
    }

    const std::string& getLeftName() const
    {
        return m_Left;
    }

    const std::string& getRightName() const
    {
        return m_Right;
    }

    const std::string& getNextName(unsigned dir) const
    {
        if ((dir & 1) == 0) {
            return m_Left;
        } else {
            return m_Right;
        }
    }

    Node* getNextNode(unsigned dir)
    {
        return m_NextNodes[dir & 1];
    }

    Node* getNextNode(InstructionList& il);
    Node* getNextNodeWithVisit(InstructionList& il);

    const Node* getNextNode(unsigned dir) const
    {
        return m_NextNodes[dir & 1];
    }

    void setNextNode(unsigned dir, Node* node)
    {
        m_NextNodes[dir & 1] = node;
    }

    bool isFinal() const
    {
        return m_Final;
    }

    bool isGhostFinal() const
    {
        return m_GhostFinal;
    }

    void reset()
    {
        m_LastVisitedInstruction.clear();
    }

    std::size_t getLastVisitedInstruction(std::size_t instruction_index) const
    {
        auto pos = m_LastVisitedInstruction.find(instruction_index);
        if (pos != m_LastVisitedInstruction.end()) {
            return pos->second;
        } else {
            return std::numeric_limits<std::size_t>::max();
        }
    }

    bool isVisited(std::size_t instruction_index) const
    {
        return m_LastVisitedInstruction.find(instruction_index) != m_LastVisitedInstruction.end();
    }

private:
    bool m_Final;
    bool m_GhostFinal;

    std::string m_Name;
    std::string m_Left;
    std::string m_Right;

    std::array<Node*, 2> m_NextNodes;

    std::unordered_map<std::size_t, std::size_t> m_LastVisitedInstruction;
};

void resetNodes(std::map<std::string, std::unique_ptr<Node>>& nodelist)
{
    for (auto& n : nodelist) {
        n.second->reset();
    }
}

std::string replace_all_occurences(std::string haystack, const std::string& needle, const std::string& replacement)
{
    while (!haystack.empty()) {
        auto pos = haystack.find(needle);
        if (pos == std::string::npos)
            break;

        haystack.replace(pos, needle.size(), replacement);
    }

    return haystack;
}

class InstructionList
{
public:
    InstructionList() : m_NextInstruction(0){};
    InstructionList(const std::string& il) : m_NextInstruction(0)
    {
        for (auto& i : il) {
            switch (i) {
                case 'L':
                    m_Instructions.emplace_back(0);
                    break;
                case 'R':
                    m_Instructions.emplace_back(1);
                    break;
                default:
                    throw std::invalid_argument("Invalid instruction");
                    break;
            }
        }
    };

    bool empty() const
    {
        return m_Instructions.empty();
    }

    std::size_t size() const
    {
        return m_Instructions.size();
    }

    std::ostream& write(std::ostream& s) const
    {
        const std::string_view map{"LR"sv};

        for (auto& i : m_Instructions) {
            s << map[i & 1];
        }
        return s;
    }

    unsigned getNextInstruction()
    {
        return m_Instructions[(m_NextInstruction++) % m_Instructions.size()];
    }

    std::size_t getProcessedInstructions() const
    {
        return m_NextInstruction;
    }

    std::size_t getInstructionIndex() const
    {
        return m_NextInstruction % m_Instructions.size();
    }

    void reset()
    {
        m_NextInstruction = 0;
    }

    struct Ghost {
        Node* start_node;
        std::size_t loop_point;
        std::size_t loop_length;
        std::size_t first_exit_pos;
    };

    Ghost getGhostParameters(Node* node)
    {
        Ghost ret;
        ret.start_node = node;

        reset();

        std::vector<std::size_t> finalnodes;
        Node* cn = node;
        while (!cn->isVisited(getInstructionIndex())) {
            cn = cn->getNextNodeWithVisit(*this);
            if (cn->isGhostFinal()) {
                finalnodes.emplace_back(getProcessedInstructions());
            }
        }

        ret.loop_point = cn->getLastVisitedInstruction(getInstructionIndex());
        ret.loop_length = getProcessedInstructions() - ret.loop_point;

        if (finalnodes.size() != 1)
            throw std::invalid_argument("Can't solve");

        ret.first_exit_pos = finalnodes[0];

        return ret;
    }

private:
    std::vector<unsigned> m_Instructions;
    std::size_t m_NextInstruction;
};

Node* Node::getNextNode(InstructionList& il)
{
    return getNextNode(il.getNextInstruction());
}

Node* Node::getNextNodeWithVisit(InstructionList& il)
{
    m_LastVisitedInstruction[il.getInstructionIndex()] = il.getProcessedInstructions();
    return getNextNode(il.getNextInstruction());
}

namespace std
{
    string to_string(const Node& node)
    {
        std::ostringstream buf;
        node.write(buf);
        return buf.str();
    }

    ostream& operator<<(ostream& os, const Node& node)
    {
        return node.write(os);
    }

    string to_string(const InstructionList& node)
    {
        std::ostringstream buf;
        node.write(buf);
        return buf.str();
    }

    ostream& operator<<(ostream& os, const InstructionList& node)
    {
        return node.write(os);
    }
} // namespace std

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " datafilename " << std::endl << std::endl;

        exit(-1);
    }

    std::map<std::string, std::unique_ptr<Node>> nodes;
    InstructionList instructions;

    const std::regex re_nodedesc{"\\s*([0-9A-Z]+)\\s+=\\s+[(]([0-9A-Z]+)\\s*,\\s*([0-9A-Z]+)\\s*[)]\\s*"};

    try {
        std::ifstream infile(argv[1]);

        std::string line;

        while (std::getline(infile, line)) {
            if (instructions.empty() && (line.find_first_not_of("LR") == std::string::npos)) {
                instructions = InstructionList(line);
            } else {
                std::smatch match;
                if (std::regex_match(line, match, re_nodedesc)) {
                    nodes.emplace(match[1].str(), std::make_unique<Node>(match[1].str(), match[2].str(), match[3].str()));
                }
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    /* Resolve pointers */
    for (auto& n : nodes) {
        for (std::size_t i = 0; i < 2; ++i) {
            auto node_i = nodes.find(n.second->getNextName(i));
            if (node_i == nodes.end()) {
                throw std::invalid_argument("Node " + n.second->getNextName(i) + " not defined");
            }
            n.second->setNextNode(i, node_i->second.get());
        }
    }

    if (with_debug) {
        std::cout << instructions << std::endl << std::endl;
        for (auto& n : nodes) {
            std::cout << *n.second << std::endl;
        }
    }

    bool isFinal = false;
    Node* currentNode = nullptr;
    {
        auto first_node = nodes.find("AAA");
        if (first_node == nodes.end()) {
            throw std::invalid_argument("First node AAA not defined");
        }
        currentNode = first_node->second.get();
    }

    instructions.reset();
    while (!isFinal) {
        currentNode = currentNode->getNextNode(instructions.getNextInstruction());
        isFinal = currentNode->isFinal();
    }

    std::cout << "A : Took " << instructions.getProcessedInstructions() << " steps to reach final node" << std::endl;

    std::vector<Node*> ghostStartNodes;
    for (auto& n : nodes) {
        if (*n.first.rbegin() == 'A') {
            ghostStartNodes.emplace_back(n.second.get());
        }
    }

    std::vector<InstructionList::Ghost> ghostPaths;
    for (auto& n : ghostStartNodes) {
        resetNodes(nodes);
        ghostPaths.emplace_back(instructions.getGhostParameters(n));
    }

    std::list<std::size_t> lcm_helper;
    for (auto& n : ghostPaths) {
        lcm_helper.push_back(n.loop_length);
    }

    /* Find LCM of all loop lenghts */
    while (lcm_helper.size() > 1) {
        lcm_helper.sort();

        std::size_t a = *lcm_helper.begin();
        lcm_helper.pop_front();

        std::size_t b = *lcm_helper.begin();
        lcm_helper.pop_front();

        lcm_helper.push_back(std::lcm(a, b));
    }

    std::cout << "B : All ghosts reach exit node at step " << *lcm_helper.begin() << std::endl;

    return 0;
}
