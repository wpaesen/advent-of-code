#include <array>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
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
#include <map>

static bool with_debug = true;
using namespace std::string_view_literals;

enum Direction {
   NORTH,
   EAST,
   SOUTH,
   WEST
};

Direction operator*(const Direction &dir, int value) {
  if (value < 0) {
     switch (dir) { 
      case NORTH: 
         return SOUTH;
      case EAST: 
         return WEST;
      case SOUTH: 
         return NORTH;
      case WEST:
        return EAST;
    }
  }
  
  return dir;
}

struct Coord {
  Coord() : x(-1), y(-1){};
  Coord(int a_x, int a_y) : x(a_x), y(a_y){};

  int x;
  int y;

  Coord &operator+=(const Direction &dir) {
     switch (dir) {
        case NORTH:
          y -= 1;
          break;
        case EAST:
          x += 1;
          break;
        case SOUTH:
          y += 1;
          break;
        case WEST:
          x -= 1;
          break;
        }
     return *this;
  }

  Coord operator+(const Direction &dir) const { 
     Coord ret(*this);
     ret += dir;
     return ret;
  }

  Coord &operator-=(const Direction &dir) {
    switch (dir) {
    case NORTH:
      y += 1;
      break;
    case EAST:
      x -= 1;
      break;
    case SOUTH:
      y -= 1;
      break;
    case WEST:
      x += 1;
      break;
    }
    return *this;
  }

  Coord operator-(const Direction &dir) const {
    Coord ret(*this);
    ret += dir;
    return ret;
  }
};

class Cell
{
public:
  Cell(char a_type) : type(a_type){};

  char getType() const { return type; }

  bool isProcessed(const Direction &dir) const {
    return processedDirections.find(dir) != processedDirections.end();
  }

  void setProcessed(const Direction &dir) { 
     processedDirections.emplace(dir);
  }


  bool isEnergized() const { 
     return !processedDirections.empty();
  }

  void reset() { 
     processedDirections.clear(); 
  }

private:
  char type;
  std::set<Direction> processedDirections;
};

struct Beam
{
  Beam() : pos(0, 0), dir(EAST){};
  Beam(const Coord &a_pos, const Direction &a_dir) : pos(a_pos), dir(a_dir){};

  void run(Cell &cell, std::list<Beam>& ret) { 
    
     if (cell.getType() == 'X')
        return;

     if (cell.isProcessed(dir))
        return;

     cell.setProcessed(dir);

     switch (cell.getType()) { 
     case '.':
       ret.emplace_back(next());
       break;
     case '|':
        switch (dir) { 
        case NORTH:
        case SOUTH:
          ret.emplace_back(next());
          break;
        default:
          ret.emplace_back(next(NORTH));
          ret.emplace_back(next(SOUTH));
       }
       break;
     case '-':
       switch (dir) {
       case EAST:
       case WEST:
         ret.emplace_back(next());
         break;
       default:
         ret.emplace_back(next(EAST));
         ret.emplace_back(next(WEST));
       }
       break;
     case '\\':
       switch (dir) {
         case NORTH:
            ret.emplace_back(next(WEST));
            break;
         case EAST:
            ret.emplace_back(next(SOUTH));
            break;
         case SOUTH:
            ret.emplace_back(next(EAST));
            break;
         case WEST:
            ret.emplace_back(next(NORTH));
            break;
       }
       break;
     case '/':
       switch (dir) {
       case NORTH:
         ret.emplace_back(next(EAST));
         break;
       case WEST:
         ret.emplace_back(next(SOUTH));
         break;
       case EAST:
         ret.emplace_back(next(NORTH));
         break;
       case SOUTH:
         ret.emplace_back(next(WEST));
         break;

       }
       break;
     default:
          throw std::invalid_argument("Unknown cell type");
          break;
     }
  }

  Beam next() const {
    return Beam(pos + dir, dir);
  }

  Beam next(const Direction &new_dir) const {
    return Beam(pos + new_dir, new_dir);
  }

  Coord pos;
  Direction dir;
};

class Grid
{
public:
 
    Grid() : m_Offgrid('X')
    {
    }

    void addLine(const std::string& s)
    {
      m_Rows.resize(m_Rows.size() + 1);
      auto &row = *m_Rows.rbegin();
      for (auto &c : s) {
        row.emplace_back(c);
      }

      std::size_t width = 0;
      for (auto &r : m_Rows) {
        width = std::max(width, r.size());
        if (r.size() != width)
          throw std::invalid_argument("Irregular grid size");
      }
    }

    std::ostream &write(std::ostream &os) const {
      for (auto &row : m_Rows) {
        for (auto &cell : row) {
          if (cell.isEnergized()) {
            os << "#";          
          } else {
            os << cell.getType();
          }
        }
        os << std::endl;
      }
      return os;
    }

    Cell &operator[](const Coord& coord) {
      if (coord.y < 0)
        return m_Offgrid;
      if (coord.y >= m_Rows.size())
        return m_Offgrid;
      if (coord.x < 0)
        return m_Offgrid;
      if (coord.x >= m_Rows[coord.y].size())
        return m_Offgrid;

      return m_Rows[coord.y][coord.x];
    }

    const Cell &operator[](const Coord &coord) const {
      if (coord.y < 0)
        return m_Offgrid;
      if (coord.y >= m_Rows.size())
        return m_Offgrid;
      if (coord.x < 0)
        return m_Offgrid;
      if (coord.x >= m_Rows[coord.y].size())
        return m_Offgrid;

      return m_Rows[coord.y][coord.x];
    }

    std::size_t run(const Beam& startpoint = Beam()) 
    {
      for (auto &row : m_Rows) {
        for (auto &cell : row) {
          cell.reset();
        }
      }

      std::list<Beam> beams;
      beams.emplace_back(startpoint);

      while (!beams.empty()) {
        std::list<Beam> new_beams;
      
        for (auto &b : beams) {
          b.run((*this)[b.pos], new_beams);
        }

        std::swap(beams, new_beams);
      }
    
      return getCellsEnergized();
    }

    std::size_t getCellsEnergized() const {
      std::size_t ret = 0;

      for (auto &row : m_Rows) {
        for (auto &cell : row) {
          if (cell.isEnergized()) {
            ret++;
          }
        }
      }

      return ret;
    }

    std::list<Beam> border() const {
       std::list<Beam> ret;

       for (std::size_t y = 0; y < m_Rows.size(); ++y) {
         ret.emplace_back(Coord(0, y), EAST);
         ret.emplace_back(Coord(m_Rows[y].size()-1, y), WEST);
       }

       if (!m_Rows.empty()) {
         for (std::size_t x = 0; x < m_Rows[0].size(); ++x) {
           ret.emplace_back(Coord(x, 0), SOUTH);
           ret.emplace_back(Coord(x, m_Rows.size()-1), NORTH);
         }
       }

       return ret;
    };

  private: 

   std::vector<std::vector<Cell>> m_Rows;
    Cell m_Offgrid;
};

namespace std
{
    string to_string(const Grid& node)
    {
        std::ostringstream buf;
        node.write(buf);
        return buf.str();
    }

    ostream& operator<<(ostream& os, const Grid& node)
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

    Grid grid;

    try {
        with_debug = !(std::string(argv[1]) == "input.dat");
        
        std::ifstream infile(argv[1]);

        for (std::string line; std::getline(infile, line);) {
          if (!line.empty()) {
            grid.addLine(line);
          }
        }
    } catch (std::exception& e) {
        std::cerr << "Reading data error: " << e.what() << std::endl;
        std::exit(-1);
    }

    if (with_debug) {
      std::cout << grid << std::endl;
    }

    std::cout << "Cells energized A " << grid.run() << std::endl;

    if (with_debug) {
      std::cout << grid << std::endl;
    }

    std::size_t best = 0;
    for (auto &beam : grid.border()) {
      auto n_energized = grid.run(beam);
      best = std::max(best, n_energized);
    }

    std::cout << "Cells energized B " << best << std::endl;


    return 0;
}
