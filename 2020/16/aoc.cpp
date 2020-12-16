#include <string>
#include <array>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>

class Ticket
{
public:

    Ticket() : m_Invalid(false) {};
    Ticket(const std::smatch& match) : m_Invalid(false)
    {
        std::string fields = match[0].str();

        size_t pos;
        while ((pos = fields.find(",")) != std::string::npos)
        {
            m_Fields.emplace_back(std::stoi(fields.substr(0, pos)));
            fields.erase(0, pos + 1);
        }
        if (! fields.empty())
        {
            m_Fields.emplace_back(std::stoi(fields));
        }
    }

    void print()
    {
        for (auto &i : m_Fields)
        {
            std::cout << i << ",";
        }
        std::cout << std::endl;
    }

    std::vector<int> m_Fields;
    bool m_Invalid;
};

class Rule
{
public:
    Rule(const std::string& id, unsigned min1, unsigned max1, unsigned min2, unsigned max2)
    {
        m_Id = id;
        m_Range[0].first = min1;
        m_Range[0].second = max1;
        m_Range[1].first = min2;
        m_Range[1].second = max2;
        m_InUse = false;
        m_Position = 0;
    };

    bool valid(unsigned val)
    {
        for (auto& r: m_Range)
        {
            if ((val >= r.first) && (val <= r.second)) return true;
        }

        return false;
    }

    void print()
    {
        std::cout << m_Id << " : "
                  << m_Range[0].first << "-" << m_Range[0].second << " or "
                  << m_Range[1].first << "-" << m_Range[1].second << std::endl;
    };

    std::string m_Id;
    std::array<std::pair<unsigned,unsigned>,2> m_Range;

    bool m_InUse;
    size_t m_Position;
};

class Notes
{
public:
    Notes() {};

    void prepare()
    {
        for (auto r : rules)
        {
            for (auto& k : r->m_Range)
            {
                for (unsigned i=k.first; i<=k.second; ++i)
                {
                    unsigned idx = i>>5;
                    rulemap[idx].set(i&0x1f, true);
                }
            }
        }
    }

    void scan_invalid()
    {
        int64_t error_scan_rate = 0;
        int n_invalid = 0;
        for (auto& t: nearby_tickets)
        {
            for (auto& f: t.m_Fields)
            {
                unsigned idx = (f>>5);

                if (! rulemap[idx].test(idx&0x1f))
                {
                    error_scan_rate += f;
                    t.m_Invalid = true;
                }
            }

            if (t.m_Invalid)
            {
                n_invalid++;
            }
        }

        std::cout << "Error scanning rate " << error_scan_rate << std::endl;

        nearby_tickets.erase(
            std::remove_if(
                nearby_tickets.begin(), nearby_tickets.end(),
                [](const Ticket& a) { return a.m_Invalid;}
            )
            , nearby_tickets.end()
        );
    }

    std::shared_ptr<Rule> getFieldRule(size_t pos)
    {
        for (auto r: rules)
        {
            if (r->m_InUse)
            {
                if (r->m_Position == pos) return r;
            }
        }

        return std::shared_ptr<Rule>();
    }

    void resolve()
    {
        std::vector<std::vector<std::shared_ptr<Rule>>> fm;

        /* Count number of fields in remaining tickets */
        size_t max_fields = 0;
        for (auto& t: nearby_tickets)
        {
            max_fields = std::max(max_fields, t.m_Fields.size());
        }

        /* for each field, prepare an array of all Rules.  Since in the beginnen of the scan
         * every rule is a possible match.
         */
        for (size_t i=0; i<max_fields; ++i)
        {
            fm.push_back(rules);
        }

        /* Now scan all tickets and eliminate rules.*/
        for (auto& t: nearby_tickets)
        {
            for (size_t i=0; i<t.m_Fields.size(); ++i)
            {
                unsigned fv = t.m_Fields[i];

                fm[i].erase(
                    std::remove_if(
                        fm[i].begin(), fm[i].end(),
                        [fv](const std::shared_ptr<Rule> r){ return ! r->valid(fv); }
                    )
                    , fm[i].end()
                );
            }
        }

        bool isEmpty = false;
        while (! isEmpty)
        {
            isEmpty = true;

            /* Remove fields that might match according to the rules, but that are already tied to
             * another position
             */
            for (auto& fl: fm)
            {
                fl.erase(std::remove_if(fl.begin(), fl.end(), [](std::shared_ptr<Rule>& r){ return r->m_InUse;}), fl.end());
            }

            /* Check if there are fields that only have one matching rule */
            size_t i = 0;
            for (auto& fl: fm)
            {
                if (fl.size() == 1)
                {
                    /* Only one rule left for this field.  So pin it to this field. */
                    fl[0]->m_InUse = true;
                    fl[0]->m_Position = i;
                }

                i++;

                if (! fl.empty() )
                {
                    isEmpty = false;
                }
            }
        }

        size_t i = 0;
        int64_t mul = 1;
        std::string prefix("departure");

        for (auto& v : my_ticket.m_Fields)
        {
            std::shared_ptr<Rule> field = getFieldRule(i++);

            if (field->m_Id.compare(0, prefix.length(), prefix) == 0)
            {
                mul *= v;
            }
        }

        std::cout << "Result " << mul << std::endl;
    }

    std::unordered_map<unsigned, std::bitset<64>> rulemap;
    std::vector<std::shared_ptr<Rule>> rules;
    Ticket my_ticket;
    std::vector<Ticket> nearby_tickets;
};

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    Notes notes;

    try
    {
        std::ifstream infile(argv[1]);

        int state = 0;

        const std::regex matchrule_rule("^(.+)\\s*:\\s+([0-9]+)-([0-9]+)\\s+or\\s+([0-9]+)-([0-9]+)\\s*$");
        const std::regex matchrule_ticket("^([0-9]+,?)+$");

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            switch (state)
            {
                case 0:
                    if (std::regex_match(line, match, matchrule_rule))
                    {
                        notes.rules.emplace_back(
                            std::make_shared<Rule>(
                                match[1].str(),
                                std::stoi(match[2].str()),
                                std::stoi(match[3].str()),
                                std::stoi(match[4].str()),
                                std::stoi(match[5].str())
                            )
                        );
                    }
                    else if (line == "your ticket:")
                    {
                        state = 1;
                    }
                    break;
                case 1:
                    if (std::regex_match(line, match, matchrule_ticket))
                    {
                        notes.my_ticket = Ticket(match);
                    }
                    else if (line == "nearby tickets:")
                    {
                        state = 2;
                    }
                    break;
                case 2:
                    if (std::regex_match(line, match, matchrule_ticket))
                    {
                        notes.nearby_tickets.emplace_back(match);
                    }
                    break;
                default:
                    break;
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

#if 0
    for (auto r : notes.rules)
    {
        r->print();
    }

    std::cout << std::endl << "your ticket:" << std::endl;
    notes.my_ticket.print();
    std::cout << std::endl << "nearby tickets:" << std::endl;
    for (auto& t : notes.nearby_tickets)
    {
        t.print();
    }
#endif

    notes.prepare();
    notes.scan_invalid();
    notes.resolve();

    return 0;
}
