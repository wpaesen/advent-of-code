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

class DirEnt
{
public:
    DirEnt(const std::string& a_name, DirEnt* a_parent) : name(a_name), parent(a_parent) {};

    std::map<std::string, std::unique_ptr<DirEnt>> children;
    std::map<std::string, std::size_t> files;
    
    std::string name;
    DirEnt* parent;

    std::size_t getSize()
    {
        if (! sizeCache)
        {
            std::size_t ret = 0;

            for (const auto& d : children)
            {
                ret += d.second->getSize();
            }

            for (const auto& f : files)
            {
                ret += f.second;
            }

            sizeCache = std::make_unique<std::size_t>(ret);
        }

        return *sizeCache.get();
    }

    std::string getName() const
    {
        std::vector<std::string> parts;

        const DirEnt* p = this;
        while (p)
        {
            parts.push_back(p->name);
            p = p->parent;
        }

        std::string ret;
        for (auto i = parts.rbegin(); i != parts.rend(); i = std::next(i))
        {
            ret += *i;
            if (*i != "/") 
                ret += "/";
        }

        return ret;
    }

    std::unique_ptr<std::size_t> sizeCache;
};

class FsTree
{
public:

    FsTree(std::size_t a_size) : fs_size(a_size), root(std::make_unique<DirEnt>("/", nullptr)), cwd(root.get()), ls_active(false) {};

    void parse(const std::string& line)
    {
        if (! line.empty())
        {
            if (line[0] == '$')
            {
                parse_cmd(line);
            }
            else if (ls_active)
            {
                parse_file(line);
            }
            else
            {
                throw std::invalid_argument("Syntax error");
            }
        }
    };

    void walkEachDir(std::function<void(DirEnt* dir)> predicate)
    {
        walkEachDirImpl(root.get(), predicate);
    }

    void walkEachDirCond(std::function<bool(DirEnt *dir)> predicate)
    {
        walkEachDirCondImpl(root.get(), predicate);
    }

    std::size_t getFreeSpace() 
    {
        return fs_size - root->getSize();
    }

private:

    void walkEachDirImpl(DirEnt* dir, std::function<void(DirEnt* dir)> predicate)
    {
        predicate(dir);

        for (auto&c : dir->children)
        {
            walkEachDirImpl(c.second.get(), predicate);
        }
    }

    void walkEachDirCondImpl(DirEnt* dir, std::function<bool(DirEnt *dir)> predicate)
    {
        if ( predicate(dir) )
        {
            for (auto&c : dir->children)
            {
                walkEachDirCondImpl(c.second.get(), predicate);
            }
        }
    }

    void parse_cmd(const std::string& line);
    void parse_file(const std::string& line);

    std::unique_ptr<DirEnt> root;
    DirEnt* cwd;
    bool ls_active;
    std::size_t fs_size;

    static const std::regex filematch;
};

const std::regex FsTree::filematch{"^([0-9]+)\\s*(.*)$"};

void FsTree::parse_cmd(const std::string& line)
{
    if (line.find("$ cd ") == 0)
    {
        ls_active = false;
        const std::string filename = line.substr(5);

        if (filename == "..")
        {
            if (cwd->parent)
            {
                cwd = cwd->parent;
            }
            else
            {
                std::invalid_argument("Can't descend beyond root");
            }
        }
        else if (filename == "/")
        {
            cwd = root.get();
        }
        else 
        {
            auto cdir = cwd->children.find(filename);
            if (cdir != cwd->children.end())
            {
                cwd = cdir->second.get();
            }
            else
            {
                std::invalid_argument("Dir doesn't exist");
            }
        }
    }
    else if (line == "$ ls")
    {
        ls_active = true;
    }
    else
    {
        ls_active = false;
        throw std::invalid_argument("Unknown command");
    }
}

void FsTree::parse_file(const std::string& line)
{
    std::smatch match;
    if (line.find("dir ") == 0)
    {
        cwd->children.emplace(line.substr(4), std::make_unique<DirEnt>( line.substr(4)  ,cwd));
    }
    else if (std::regex_match(line, match, filematch))
    {
        cwd->files[match[2].str()] = std::stoull(match[1].str());
    } 
    else
    {
        throw std::invalid_argument("Invalid file listing");
    }
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    FsTree fs(70000000UL);
    try
    {
        std::ifstream infile(argv[1]);

        std::string line;
        while (std::getline(infile, line))
        {
            fs.parse(line);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    std::size_t parta = 0;
    fs.walkEachDir([&parta](DirEnt* dir) {
        std::size_t s = dir->getSize();
        if (s <= 100000)
        {
            parta += s;
        }

        std::cout << std::setw(10) << s << " : " << dir->getName() << std::endl;
    });

    std::cout << std::endl;
    
    std::cout << "Part a : total size " << parta << std::endl;

    std::vector<DirEnt*> to_delete;
    fs.walkEachDirCond([&to_delete, &fs](DirEnt* dir) {
        if (dir->getSize() + fs.getFreeSpace() < 30000000UL)
            return false;

        to_delete.push_back(dir);
        return true;
    });

    auto choice = std::min_element(to_delete.begin(), to_delete.end(), [](DirEnt*& a, DirEnt*& b ) {
        return a->getSize() < b->getSize();
    });

    if (choice == to_delete.end())
    {
        std::cerr << "No resolution" << std::endl;
    }
    else
    {
        std::cout << "Part b : size of dir [" << (*choice)->name << "] to delete " << (*choice)->getSize() << std::endl;
    }

    return 0;
}
