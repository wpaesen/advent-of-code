#include <string>
#include <array>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>

#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <future>

class Ingredient;

class Allergen : public std::enable_shared_from_this<Allergen>
{
    public:
        Allergen(const std::string& name, size_t id) : m_Name(name), m_Id(id)
        {
        }

        operator size_t() const { return m_Id; }
        size_t getId() { return m_Id; };

        std::string toString() const { return m_Name; };

        void setIngredient(std::shared_ptr<Ingredient> ingredient )
        {
            m_Ingredient = ingredient;
        }

        std::shared_ptr<Ingredient> getIngredient()
        {
            return m_Ingredient;
        }

    private:

        std::string m_Name;
        size_t m_Id;

        std::shared_ptr<Ingredient> m_Ingredient;
};

class Ingredient : public std::enable_shared_from_this<Ingredient>
{
    public:
        Ingredient(const std::string& name, size_t id ) : m_Name(name), m_Id(id), m_Used(0), m_Inert(false)
        {
        }

        operator size_t() const { return m_Id; }
        size_t getId() { return m_Id; };

        std::string toString() const
        {
            std::string ret = m_Name;
            while (ret.length() < 32)
            {
                ret += " ";
            }

            return ret;
        };

        std::string getName() const
        {
            return m_Name;
        }

        void setAllergen(std::shared_ptr<Allergen> a )
        {
            m_Allergen = a;
        }

        void resetAllergen()
        {
            m_Allergen.reset();
        }

        std::shared_ptr<Allergen> getAllergen()
        {
            return m_Allergen;
        }

        size_t getAllergenId()
        {
            if (m_Allergen)
            {
                return m_Allergen->getId();
            }

            return 0;
        }

        void addUsed()
        {
            m_Used++;
        }

        size_t getUsed()
        {
            return m_Used;
        }

        void setInert()
        {
            m_Inert = true;
        }

        bool isInert()
        {
            return m_Inert;
        }

    private:

        std::string m_Name;
        size_t m_Id;
        unsigned m_Used;
        bool m_Inert;

        std::shared_ptr<Allergen> m_Allergen;
};


class Product : public std::enable_shared_from_this<Product>
{
    public:
        Product()
        {};

        std::vector<std::shared_ptr<Ingredient>> m_Ingredients;
        std::vector<std::shared_ptr<Allergen>> m_Allergens;

        std::string toString() const
        {
            std::string ret;

            for (auto i : m_Ingredients)
            {
                ret += i->toString() + " ";
            }

            if (! m_Allergens.empty())
            {
                ret += "(contains ";
                bool first = true;
                for (auto a: m_Allergens)
                {
                    if (! first)
                    {
                        ret += ", ";
                    }

                    ret += a->toString();
                    first = false;
                }
                ret += ")";
            }

            return ret;
        }

        bool validate(std::shared_ptr<Allergen> a)
        {
            bool hasAllergen = false;
            for (auto& i : m_Allergens)
            {
                if (i == a)
                {
                    hasAllergen = true;
                    break;
                }
            }

            if (! hasAllergen)
            {
                return true;
            }

            for (auto& i : m_Ingredients)
            {
                if (i->getAllergenId() == a->getId())
                {
                    return true;
                }
            }

            return false;
        }

        bool validateFull()
        {
            for (auto& a: m_Allergens)
            {
                bool match = false;
                for (auto& i : m_Ingredients)
                {
                    if (i->getAllergenId() == a->getId())
                    {
                        match = true;
                        break;
                    }
                }

                if (! match)
                {
                    return false;
                }
            }

            return true;
        }

        void cleanupInert()
        {
            m_Ingredients.erase(
                std::remove_if(m_Ingredients.begin(), m_Ingredients.end(), [](const std::shared_ptr<Ingredient>& x){
                    return x->isInert();
                }),
                m_Ingredients.end()
            );
        }
};

class List
{
    public:
        List() : m_IngredientIds(0), m_AllergenIds(0) {};

        std::unordered_map<std::string, std::shared_ptr<Ingredient>> m_Ingredients;
        size_t m_IngredientIds;
        std::unordered_map<std::string, std::shared_ptr<Allergen>> m_Allergens;
        size_t m_AllergenIds;

        std::vector<std::shared_ptr<Product>> m_Products;

        void addLine(const std::string& a_ingredients, const std::string& a_allergens)
        {
            /* Skip leading whitespace (if any) */
            auto ingr = split(a_ingredients);
            auto alrg = split(a_allergens);

            std::shared_ptr<Product> p = std::make_shared<Product>();

            for (auto &s : ingr)
            {
                auto i = lookupIngredient( s );
                p->m_Ingredients.emplace_back( i );
                i->addUsed();
            }
            for (auto &s : alrg)
            {
                p->m_Allergens.emplace_back( lookupAllergen( s ) );
            }

            m_Products.emplace_back(p);
        }

        std::shared_ptr<Ingredient> lookupIngredient(const std::string& name)
        {
            std::shared_ptr<Ingredient>& e = m_Ingredients[name];
            if (! e)
            {
                e = std::make_shared<Ingredient>(name, ++m_IngredientIds);
            }

            return e;
        }

        std::shared_ptr<Allergen> lookupAllergen(const std::string& name)
        {
            std::shared_ptr<Allergen>& e = m_Allergens[name];
            if (! e)
            {
                e = std::make_shared<Allergen>(name, ++m_AllergenIds);
            }

            return e;
        }

        std::vector<std::string> split(const std::string& line)
        {
            std::vector<std::string> ret;

            /* Skip leading whitespace (if any) */
            auto pos = line.find_first_not_of(" ,");
            if (pos == std::string::npos)
            {
                ret.emplace_back(line);
            }
            else
            {
                std::string t = line.substr(pos);
                while ((pos = t.find_first_of(" ,")) != std::string::npos)
                {
                    ret.emplace_back(t.substr(0, pos));
                    auto npos = t.find_first_not_of(" ,", pos);
                    if (npos == std::string::npos)
                    {
                        t.clear();
                    }
                    else
                    {
                        t = t.substr(npos);
                    }
                }

                if (! t.empty())
                {
                    ret.emplace_back(t);
                }
            }

            /* Strip trailing whitespace from individual elements if any */
            for (auto& s : ret)
            {
                while ((! s.empty()) && ((' ' == *s.rbegin()) || (',' == *s.rbegin())))
                {
                    s.pop_back();
                }
            }

            return ret;
        }

        bool validate(std::shared_ptr<Allergen> a)
        {
            for (auto& p : m_Products)
            {
                if (! p->validate(a))
                {
                    return false;
                }
            }

            return true;
        }

        bool validateFull()
        {
            for (auto& p : m_Products)
            {
                if (! p->validateFull())
                {
                    return false;
                }
            }

            return true;
        }


        bool checkIngredientForAllergen(std::shared_ptr<Ingredient> i)
        {
            size_t n_matches = 0;
            for (auto& a: m_Allergens)
            {
                i->setAllergen(a.second);
                if (validate(a.second))
                {
                    n_matches++;
                }
            }
            i->resetAllergen();

            return (n_matches > 0);
        }

        void cleanupInert()
        {
            for (auto &p: m_Products)
            {
                p->cleanupInert();
            }

            for (auto i = m_Ingredients.begin(), last = m_Ingredients.end(); i != last;)
            {
                if (i->second->isInert())
                {
                    i = m_Ingredients.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }

        bool resolve()
        {
            /* try all permutations of allergens on ingredientes */
            std::vector<std::shared_ptr<Ingredient>> all_ingredients;
            for (auto& i: m_Ingredients)
            {
                all_ingredients.emplace_back(i.second);
            }

            std::vector<std::string> all_allergens;
            for (auto& a: m_Allergens)
            {
                all_allergens.emplace_back(a.first);
            }

            if (all_ingredients.size() != all_allergens.size())
            {
                throw std::out_of_range("Ingredients vs allergens");
            }

            std::sort(all_allergens.begin(), all_allergens.end());
            do {
                for (size_t i=0; i<all_ingredients.size(); ++i)
                {
                    all_ingredients[i]->setAllergen(m_Allergens[all_allergens[i]]);
                }

                if (validateFull())
                {
                    return true;
                }
            } while (std::next_permutation(all_allergens.begin(), all_allergens.end()));

            return false;
        }

        std::string generateDangerList()
        {
            for (auto& i: m_Ingredients)
            {
                auto allergen = i.second->getAllergen();
                allergen->setIngredient(i.second);
            }

            std::vector<std::string> all_allergens;
            for (auto& a: m_Allergens)
            {
                all_allergens.emplace_back(a.first);
            }

            std::string ret;
            std::sort(all_allergens.begin(), all_allergens.end());
            for (auto& an: all_allergens)
            {
                if (! (ret.empty()))
                {
                    ret += ",";
                }

                ret += m_Allergens[an]->getIngredient()->getName();
            }

            return ret;
        }
};


int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    List list;
    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_w("^(.*)\\s*[(]contains\\s+(.*)[)]\\s*$");
        std::string line;
        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_w))
            {
                list.addLine(match[1].str(), match[2].str());
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        return -1;
    }

    size_t sum_a = 0;
    for (auto& p: list.m_Ingredients)
    {
        if (! list.checkIngredientForAllergen(p.second))
        {
            sum_a += p.second->getUsed();
            p.second->setInert();
        }
    }

    std::cout << "Sum a : " << sum_a << std::endl;

    list.cleanupInert();

    for (auto& p: list.m_Ingredients)
    {
        std::cout << p.second->toString() << std::endl;
    }

    if (! list.resolve())
    {
        std::cout << "Could not resolve list" << std::endl;
        return 0;
    }

    std::cout << "Canonical dangerours shopping list " << std::endl;
    std::cout << list.generateDangerList() << std::endl;

    return 0;
}
