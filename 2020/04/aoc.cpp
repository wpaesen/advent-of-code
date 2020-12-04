#include <string>
#include <array>
#include <map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>

class Record
{
    public:

        static constexpr size_t NFIELDS = 8;
        static std::array<std::pair<std::string, std::function<bool(const std::string&)>>, NFIELDS> fielddef;
        static std::array<std::string, 7> eyecolors;

        Record() {};

        void setField(const std::string& id, const std::string& data)
        {
            for (size_t i=0; i<fielddef.size(); ++i )
            {
                if (id == fielddef[i].first)
                {
                    m_FieldData[i] = data;
                    m_FieldMap[i] = true;

                    return;
                }
            }
        }

        bool hasFields() {
            return m_FieldMap.any();
        }

        bool isComplete()
        {
            return ( m_FieldMap & m_FieldMapRequired ) == m_FieldMapRequired;
        }

        bool isValid()
        {
            bool ret = true;
            for (size_t i=0; ret && (i<fielddef.size()); ++i)
            {
                if (m_FieldMap[i])
                {
                    if (! fielddef[i].second(m_FieldData[i]))
                    {
                        return false;
                    }
                }
                else
                {
                    if (m_FieldMapRequired[i])
                    {
                        return false;
                    }
                }
            }

            return true;

        }

    private:

        std::array<std::string, NFIELDS> m_FieldData;
        std::bitset<NFIELDS> m_FieldMap;
        static std::bitset<NFIELDS> m_FieldMapRequired;

};

std::array<std::pair<std::string, std::function<bool(const std::string&)>>, Record::NFIELDS> Record::fielddef =
{
std::make_pair( "byr", [](const std::string& val){
    /*  byr (Birth Year) - four digits; at least 1920 and at most 2002. */
    if (val.length() != 4) return false;
    auto ival = std::stoi(val);
    return (ival >= 1920) && (ival <= 2002);
} ),
std::make_pair( "iyr", [](const std::string& val){
    /* iyr (Issue Year) - four digits; at least 2010 and at most 2020. */
    if (val.length() != 4) return false;
    auto ival = std::stoi(val);
    return (ival >= 2010) && (ival <= 2020);
} ),
std::make_pair( "eyr", [](const std::string& val){
    /* eyr (Expiration Year) - four digits; at least 2020 and at most 2030. */
    if (val.length() != 4) return false;
    auto ival = std::stoi(val);
    return (ival >= 2020) && (ival <= 2030);
} ),
std::make_pair( "hgt", [](const std::string& val){
    /* hgt (Height) - a number followed by either cm or in:
     * If cm, the number must be at least 150 and at most 193.
     *  If in, the number must be at least 59 and at most 76.
     */
    if (val.length() < 3) return false;

    size_t unitpos = -1;

    auto ival = std::stoi(val, &unitpos);

    std::string cm("cm");
    std::string in("in");
    if (std::equal(val.begin()+unitpos, val.end(), cm.begin()))
    {
        return (ival>=150)&&(ival <= 193);
    }
    else if (std::equal(val.begin()+unitpos, val.end(), in.begin()))
    {
        return (ival>=59)&&(ival <= 76);
    }

    return false;
} ),
std::make_pair( "hcl", [](const std::string& val){
    /* hcl (Hair Color) - a # followed by exactly six characters 0-9 or a-f. */
    if (val.length() != 7) return false;
    if (val[0] != '#') return false;
    for (auto i = val.begin()+1; i != val.end(); ++i)
    {
        if (! isxdigit(*i)) return false;
    }
    return true;
} ),
std::make_pair( "ecl", [](const std::string& val){
    for (auto &ecl: Record::eyecolors)
    {
        if (ecl == val) return true;
    }
    return false;
} ),
std::make_pair( "pid", [](const std::string& val){
    if (val.length() != 9) return false;
    for (auto c : val)
    {
        if (! isdigit(c)) return false;
    }
    return true;
} ),
std::make_pair( "cid", [](const std::string& val){
    return true;
} )
};

std::array<std::string, 7> Record::eyecolors =
{
    "amb", "blu", "brn", "gry", "grn", "hzl", "oth"
};

std::bitset<Record::NFIELDS> Record::m_FieldMapRequired = 0x7f;

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    std::vector<Record> records;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchfields("([a-z]{3}):([^[:s:]]+)");
        const std::regex emptyline("^[[:s:]]*$");

        Record rec;

        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, emptyline))
            {
                if (rec.hasFields())
                {
                    records.emplace_back(rec);
                    rec = Record();
                }
            }
            else
            {
                while (std::regex_search(line, match, matchfields))
                {
                    for (size_t i = 1; i+1 < match.size(); i+=2)
                    {
                        rec.setField(match[i].str(), match[i+1].str());
                    }
                    line = match.suffix();
                }
            }
        }

        if (rec.hasFields())
        {
            records.emplace_back(rec);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
    }

    std::cout << "Got " << records.size() << " records" <<std::endl;

    size_t n_complete = 0;
    size_t n_valid = 0;
    for (auto& r : records)
    {
        if (r.isComplete()) n_complete++;
        if (r.isValid()) n_valid++;
    }

    std::cout << "  of which " << n_complete << " are completed" <<std::endl;
    std::cout << "  of which " << n_valid << " are valid" <<std::endl;

    return 0;
}
