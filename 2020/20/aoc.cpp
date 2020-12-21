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

class Monster
{
    public:
        Monster() {};

        static const std::vector<std::pair<size_t, size_t>> get()
        {
            static std::vector<std::pair<size_t, size_t>> pattern;

            if (pattern.empty())
            {
                std::array<std::string,3> input =
                {
                    "                  # ",
                    "#    ##    ##    ###",
                    " #  #  #  #  #  #   "
                };

                for (size_t y=0; y<input.size(); ++y)
                {
                    for (size_t x=0; x<input[y].length(); ++x)
                    {
                        if (input[y][x] == '#')
                        {
                            pattern.emplace_back(x, y);
                        }
                    }
                }
            }

            return pattern;
        }
};

class Tile : public std::enable_shared_from_this<Tile>
{
    public:
        enum Edge
        {
           TOP    = 0,
           RIGHT  = 1,
           BOTTOM = 2,
           LEFT   = 3
        };

        Edge opposite(Edge x)
        {
            switch(x)
            {
                case TOP: return BOTTOM;
                case RIGHT: return LEFT;
                case LEFT: return RIGHT;
                case BOTTOM: return TOP;
            }
            throw std::invalid_argument("Invalid edge");
        }

        typedef std::pair<Edge,Edge> Corner;

        static Corner TOPLEFT;
        static Corner TOPRIGHT;
        static Corner BOTTOMLEFT;
        static Corner BOTTOMRIGHT;

        struct Xfrm
        {
            Xfrm()
                : rotation(0)
                , flip_x(false)
                , flip_y(false)
            {};

            unsigned rotation;
            bool     flip_x;
            bool     flip_y;

            Edge getEdge(Edge i) const
            {
                /* First do rotation */
                unsigned k = (rotation + (unsigned)i) & 0x03;
                switch(k)
                {
                    case TOP:    return (flip_y) ? BOTTOM : TOP;
                    case RIGHT:  return (flip_x) ? LEFT : RIGHT;
                    case BOTTOM: return (flip_y) ? TOP : BOTTOM;
                    case LEFT:   return (flip_x) ? RIGHT : LEFT;
                    default:
                        throw std::invalid_argument("Invalid edge");
                        break;
                }
            };

            static std::bitset<10> flip(std::bitset<10> x)
            {
                std::bitset<10> ret;
                for (size_t i=0; i<10; ++i)
                {
                    ret.set(9-i, x.test(i));
                }

                return ret;
            }

            std::bitset<10> normalizePattern(std::bitset<10> orig, Edge e) const
            {
                switch (e)
                {
                    case TOP:
                    case BOTTOM:
                        if (flip_x)
                        {
                            return flip(orig);
                        }
                        else
                        {
                            return orig;
                        }
                        break;
                    case LEFT:
                    case RIGHT:
                        if (flip_y)
                        {
                            return flip(orig);
                        }
                        else
                        {
                            return orig;
                        }
                        break;
                }
                throw std::invalid_argument("Data syntax mismatch");
            }

            std::string toString() const
            {
                std::array<char, 128> buf;

                return std::string(buf.data(), std::snprintf(buf.data(), buf.size(),"ROT:%d,FLIP:%s%s", rotation*90, flip_x ? "X" : "", flip_y ? "Y" : ""));
            }

            std::pair<size_t, size_t> transformCoord(size_t x, size_t y, size_t width) const
            {
                size_t rx, ry;

                switch(rotation)
                {
                    case 0:
                        rx = x;
                        ry = y;
                        break;
                    case 1:
                        ry = x;
                        rx = width - 1 - y;
                        break;
                    case 2:
                        rx = width - 1 - x;
                        ry = width - 1 - y;
                        break;
                    case 3:
                        ry = width - 1 - x;
                        rx = y;
                        break;
                    default:
                    throw std::invalid_argument("Rotation mismatch");
                }

                if (flip_x)
                {
                    rx = width - 1 - rx;
                }
                if (flip_y)
                {
                    ry = width - 1 - ry;
                }

                return std::make_pair(rx, ry);
            }

            /* List of all possible permutations */
            static const std::vector<Xfrm>& all()
            {
                static std::vector<Xfrm> values;

                if (values.empty())
                {
                    Xfrm v;
                    for (size_t x = 0; x<2; ++x)
                    {
                        v.flip_x = (x>0);
                        for (size_t y = 0; y<2; ++y)
                        {
                            v.flip_y = (y>0);
                            for (v.rotation=0; v.rotation<4; ++v.rotation)
                            {
                                values.push_back(v);
                            }
                        }
                    }
                }

                return values;
            }

        };

        Tile(unsigned a_Id) : id(a_Id)
        {
        };

        Tile(unsigned a_Id, const std::vector<std::string>& a_Data)
            : id(a_Id)
        {
            if (a_Data.size() != 10)
            {
                throw std::invalid_argument("Data height mismatch");
            }

            for (size_t i=0; i<10; ++i)
            {
                if (a_Data[i].length() < 10)
                {
                    throw std::invalid_argument("Data width mismatch");
                }

                for (size_t j=0; j<10; ++j)
                {
                    switch(a_Data[i][j])
                    {
                        case '#':
                            data[i].set(j, true);
                            break;
                        case '.':
                            data[i].set(j, false);
                            break;
                        default:
                            throw std::invalid_argument("Data syntax mismatch");
                            break;
                    }
                }
            }

            edges[(int)TOP] = data[0];
            edges[(int)BOTTOM] = data[9];

            for (size_t i=0; i<10; ++i)
            {
                edges[(int)RIGHT].set(i, data[i].test(9));
                edges[(int)LEFT].set(i, data[i].test(0));
            }
        }

        void unlink()
        {
            for (auto& e:edges_matches)
            {
                e.clear();
            }
        }

        std::string toString()
        {
            std::array<char, 64> buf;
            return std::string(buf.data(), std::snprintf(buf.data(), buf.size(), "Tile %d", id));
        }

        void scanMatchingEdges(std::shared_ptr<Tile> t2)
        {
            /* This doesn't take any transformation into account. */
            for (size_t i = 0; i<4; ++i)
            {
                std::bitset<10> inverted = Xfrm::flip(edges[i]);
                for (size_t j=0; j<4; ++j)
                {
                    if ((edges[i] == t2->edges[j]) || (inverted == t2->edges[j]))
                    {
                        edges_matches[i].push_back(t2);
                        t2->edges_matches[j].push_back(shared_from_this());
                    }
                }
            }
        }

        bool isCorner()
        {
            return getPopulatedEdges() == 2;
        }

        bool isEdge()
        {
            return getPopulatedEdges() == 3;
        }

        size_t getPopulatedEdges()
        {
            size_t n_edges = 0;
            for (auto& e: edges_matches)
            {
                if (! e.empty()) n_edges++;
            }

            return n_edges;
        }

        /* Get the edge pattern, normalized according to the specified xfrm
         * This might be more efficient.q
         * */
        std::bitset<10> getTransformedEdge(Edge i, const Xfrm& xfrm )
        {
            std::bitset<10> ret;

            switch (i)
            {
                case TOP:
                    for (size_t j=0; j<10; ++j)
                    {
                        ret.set(j, getPoint(j, 0, xfrm));
                    }
                    break;
                case RIGHT:
                    for (size_t j=0; j<10; ++j)
                    {
                        ret.set(j, getPoint(9, j, xfrm));
                    }
                    break;
                case BOTTOM:
                    for (size_t j=0; j<10; ++j)
                    {
                        ret.set(j, getPoint(j, 9, xfrm));
                    }
                    break;
                case LEFT:
                    for (size_t j=0; j<10; ++j)
                    {
                        ret.set(j, getPoint(0, j, xfrm));
                    }
                    break;
                default:
                    throw std::invalid_argument("Invalid edge");
                    break;
            }

            return ret;
        }

        /* Get the edge pattern, normalized accorindg to the tile rotation */
        std::bitset<10> getTransformedEdge(Edge i)
        {
            return getTransformedEdge(i, transformation);
        }

        int getId() const { return id; };


        std::shared_ptr<Tile> getOppositeTile( Edge i, const Xfrm& xfrm )
        {
            auto& v = edges_matches[xfrm.getEdge(i)];
            if (v.empty())
            {
                return std::shared_ptr<Tile>();
            }
            else
            {
                return v[0];
            }
        }

        std::shared_ptr<Tile> getOppositeTile( Edge i )
        {
            return getOppositeTile(i, transformation);
        }

        std::vector<std::shared_ptr<Tile>> getOppositeTiles( Edge i , const Xfrm& xfrm)
        {
            return edges_matches[xfrm.getEdge(i)];
        }

        std::vector<std::shared_ptr<Tile>> getOppositeTiles( Edge i )
        {
            return getOppositeTiles(i, transformation );
        }

        void setTransformation(const Xfrm& x = Tile::Xfrm())
        {
            transformation = x;
        }

        /* Get a list of all possible Xfrms where all of the specified edges are popuplated */
        std::vector<Xfrm> findXfrmsForEdge( const std::vector<Edge>& edges ) const
        {
            std::vector<Xfrm> ret;

            for (auto pos = Xfrm::all().begin(), last = Xfrm::all().end(); pos != last; pos++)
            {
                bool ok = true;
                for (auto e : edges)
                {
                    if (edges_matches[pos->getEdge(e)].size() == 0) continue;

                    ok = false;
                    break;
                }

                if (ok)
                {
                    ret.emplace_back(*pos);
                }
            }

            return ret;
        }

        /* For convenience where we only test one edge */
        std::vector<Xfrm> findXfrmsForEdge( Edge e ) const
        {
            std::vector<Edge> arg;
            arg.emplace_back(e);
            return findXfrmsForEdge(arg);
        }

        /* For convenience where we test a corner */
        std::vector<Xfrm> findXfrmsForEdge( const Corner& c ) const
        {
            std::vector<Edge> arg;
            arg.emplace_back(c.first);
            arg.emplace_back(c.second);
            return findXfrmsForEdge(arg);
        }

        std::vector<Xfrm> findFits( const std::vector<Xfrm>& xfrms,
                                    std::vector<std::pair<std::shared_ptr<Tile>, Edge>> edges)
        {
            std::vector<Xfrm> ret;

            /* Since we only rotate ourselves, get a snapshot of the fixed world. */
            std::vector<std::pair<std::bitset<10>, Edge>> fixed_edges;
            for (auto& e: edges)
            {
                fixed_edges.emplace_back( e.first->getTransformedEdge( opposite(e.second) ), e.second );
            }

            for (auto& x: xfrms)
            {
                bool match = true;
                for (auto& e: fixed_edges)
                {
                    if ((match) && (getTransformedEdge(e.second, x) != e.first))
                    {
                        match = false;
                    }
                }

                if (match)
                {
                    ret.emplace_back(x);
                }
            }

            return ret;
        }

        /* Try find a fit for the tile.  We do this by checking all of the
         * provided Xfrms, and checking if the edges match with their partner
         * */
        bool fit(const std::vector<Xfrm>& xfrms,
                 std::vector<std::pair<std::shared_ptr<Tile>, Edge>> edges)
        {
            auto options = findFits(xfrms, edges);

            if (! options.empty())
            {
                transformation = options[0];
            }

            return ! options.empty();
        }

        /* get the state of a pixel, taking into account the given transformation */
        bool getPoint(size_t x, size_t y, const Xfrm& xfrm)
        {
            auto c = xfrm.transformCoord(x, y, 10);

            return data[c.second].test(c.first);
        }

        /* get the state of a pixel, taking into account the current transformation */
        bool getPoint(size_t x, size_t y)
        {
            return getPoint(x, y, transformation);
        }

    private:

        int id;

        std::array<std::bitset<10>,10>                    data;
        std::array<std::bitset<10>,4>                     edges;
        std::array<std::vector<std::shared_ptr<Tile>>, 4> edges_matches;

        Xfrm   transformation;
};

Tile::Corner Tile::TOPLEFT = std::make_pair(TOP, LEFT);
Tile::Corner Tile::TOPRIGHT = std::make_pair(TOP, RIGHT);
Tile::Corner Tile::BOTTOMLEFT = std::make_pair(BOTTOM, LEFT);
Tile::Corner Tile::BOTTOMRIGHT = std::make_pair(BOTTOM, RIGHT);

class Image
{
    public:
        Image()
            : grid_size(0)
        {};

        ~Image()
        {
            for ( auto t : tiles)
            {
                t->unlink();
            }
        }

        void addTile(std::shared_ptr<Tile> tile)
        {
            tiles.emplace_back(tile);
        }

        void scanMatchingTiles()
        {
            /* Determine grid size. */
            grid_size = std::sqrt(tiles.size());
            if ((grid_size == 0) || ((grid_size * grid_size) != tiles.size()))
            {
                throw std::invalid_argument("Number of tiles doesn't allow for square grid");
            }

            /*  build a map of edges and determine which tiles have matching edges */
            for (auto i = tiles.begin(); i != tiles.end(); ++i)
            {
                for (auto j = i; j!=tiles.end(); ++j)
                {
                    if (j != i)
                    {
                        (*i)->scanMatchingEdges(*j);
                    }
                }
            }
        }

        bool assemble()
        {
            std::vector<std::shared_ptr<Tile>> corners;

            for (auto i = tiles.begin(); i != tiles.end(); ++i)
            {
                size_t n_matches = (*i)->getPopulatedEdges();

                if (n_matches == 2)
                {
                    corners.emplace_back((*i));
                }
            }

            cornerProduct = 1;
            /* get Product of each corner tile id */
            for (auto &c : corners)
            {
                cornerProduct *= (int64_t)c->getId();
            }

            /* try each corner tile as topleft and try to solve it from there. */
            for (size_t i=0; i<4; ++i)
            {
                if (_try_assemble(corners[i])) return true;
            }

            return false;
        }

        size_t getGridSize() const { return grid_size; };

        std::shared_ptr<Tile>& at(size_t x, size_t y)
        {
            size_t pos = x + (y * grid_size);
            return grid.at(pos);
        }

        size_t getTilesCount() const { return tiles.size(); };
        int64_t getCornerProduct() const { return cornerProduct; };

        void printLayout()
        {
            return;

            std::array<char, 521> buf;

            for (size_t y = 0; y<grid_size; ++y)
            {
                for (size_t x =0; x<grid_size; ++x)
                {
                    auto i = at(x,y);
                    if (i)
                    {
                        unsigned mask = 0;
                        if (! (i->getOppositeTile( Tile::TOP )))
                        {
                            mask |= 1;
                        }
                        if (! (i->getOppositeTile( Tile::BOTTOM )))
                        {
                            mask |= 2;
                        }

                        char c;
                        switch(mask)
                        {
                            case 1:
                                c = '^';
                                break;
                            case 2:
                                c = '_';
                                break;
                            case 3:
                                c = ' ';
                                break;
                            default:
                                c = '=';
                                break;
                        }

                        std::snprintf(buf.data(), buf.size(), "%c% 5d ", c, i->getId());
                        std::cout << buf.data();
                    }
                    else
                    {
                        std::cout << "###### ";
                    }
                }

                std::cout << std::endl;
            }
        }

        bool point(size_t x, size_t y)
        {
            return pixels.at(pixelCoord(x, y));
        }

        char pointM(size_t x, size_t y, const std::vector<std::pair<size_t, size_t>>& monsters = std::vector<std::pair<size_t, size_t>>())
        {
            if (pixels.at(pixelCoord(x, y)))
            {
                for (auto &m : monsters)
                {
                    size_t mx = x - m.first;
                    size_t my = y - m.second;

                    if (mx > 20) continue;
                    if (my > 2) continue;

                    for (auto &p : Monster::get())
                    {
                        if (p.first != mx) continue;
                        if (p.second != my) continue;

                        return 'O';
                    }
                }

                roughness++;
                return '#';
            }
            else
            {
                return '.';
            }
        }

        size_t getRoughness() const { return roughness; }

        void generateRawImage()
        {
            pixels.resize((grid_size * 8) * (grid_size * 8));
            rawpixels.resize(pixels.size());

            std::fill(pixels.begin(), pixels.end(), false);
            std::fill(rawpixels.begin(), rawpixels.end(), false);

            Tile::Xfrm xfrm;

            for (size_t gy = 0; gy < grid_size; ++gy)
            {
                for (size_t gx = 0; gx < grid_size; ++gx)
                {
                    auto elm = at(gx, gy);

                    size_t y_offset = (gy * 8);
                    size_t x_offset = (gx * 8);

                    for (size_t x=0; x<8; ++x)
                    {
                        for (size_t y=0; y<8; ++y)
                        {
                            rawpixels[pixelCoord(x_offset + x, y_offset+y, xfrm)] = elm->getPoint(x+1, y+1);
                        }
                    }
                }
            }
        }

        void generateImage()
        {
            Tile::Xfrm xf;
            generateImage(xf);
        }

        void generateImage(const Tile::Xfrm& xfrm)
        {
            std::fill(pixels.begin(), pixels.end(), false);

            Tile::Xfrm rawxfrm;
            for (size_t y = 0; y < grid_size*8; ++y)
            {
                for (size_t x = 0; x<grid_size*8; ++x)
                {
                    pixels[pixelCoord(x, y, xfrm)] = rawpixels[pixelCoord(x, y, rawxfrm)];
                }
            }
        }

        void printRawImage()
        {
            return;

            for (size_t gy = 0; gy < grid_size; ++gy)
            {
                for (size_t y=0; y<10; ++y)
                {
                    for (size_t gx = 0; gx < grid_size; ++gx)
                    {
                        auto elm = at(gx, gy);
                        for (size_t x=0; x<10; ++x)
                        {
                            if (! elm)
                            {
                                std::cout << "X";
                            }
                            else if (elm->getPoint(x, y))
                            {
                                std::cout << "#";
                            }
                            else
                            {
                                std::cout << ".";
                            }
                        }
                        std::cout << "|";
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
            }
        }

        void printImage()
        {
            generateImage();
            return;

            for (size_t y = 0; y<grid_size*8; ++y)
            {
                for (size_t x=0; x<grid_size*8; ++x)
                {
                    if (point(x,y))
                    {
                        std::cout << "#";
                    }
                    else
                    {
                        std::cout << ".";
                    }
                }
                std::cout << std::endl;
            }
        }

        void printImage(const std::vector<std::pair<size_t, size_t>>& monsters)
        {
            // generateImage();
            roughness = 0;  /* dirty shortcut */
            for (size_t y = 0; y<grid_size*8; ++y)
            {
                for (size_t x=0; x<grid_size*8; ++x)
                {
                    std::cout << pointM(x, y, monsters);
                }
                std::cout << std::endl;
            }
        }

        bool hasMonster(size_t x, size_t y)
        {
            for (auto &p: Monster::get())
            {
                if (! point(x + p.first, y+ p.second)) return false;
            }

            return true;
        }

        std::vector<std::pair<size_t, size_t>> findMonsters()
        {
            std::vector<std::pair<size_t, size_t>> ret;

            for (size_t y = 0; y < (grid_size*8)-3; ++y)
            {
                for (size_t x = 0; x < (grid_size*8)-21; ++x)
                {
                    if (hasMonster(x, y))
                    {
                        ret.emplace_back(x, y);
                    }
                }
            }

            return ret;
        }

    private:

        bool _try_assemble2(std::shared_ptr<Tile> first, const Tile::Xfrm& xfrm)
        {
            try
            {
                size_t x, y;

                std::fill(grid.begin(), grid.end(), std::shared_ptr<Tile>());

                first->setTransformation(xfrm);
                at(0, 0) = first;

                /* Tile out top edge row */
                y = 0;
                for (x=1; x<grid_size-1; ++x)
                {
                    at(x, y) = at(x-1, y)->getOppositeTile( Tile::RIGHT );
                    if (! at(x, 0))
                    {
                        std::cout << "Fail top at x=" << x << std::endl;
                        return false;
                    }
                    at(x, y)->setTransformation();
                    if (! at(x,y)->fit(
                            at(x,y)->findXfrmsForEdge({Tile::TOP}),
                            {{at(x-1,y), Tile::LEFT}}
                        ))
                    {
                        std::cout << "Fail top edge at x=" << x << std::endl;
                        return false;
                    }
                }

                /* Tile out top right corner */
                at(x, 0) = at(x-1, 0)->getOppositeTile( Tile::RIGHT );
                if (! at(x, 0)->fit(
                        at(x, 0)->findXfrmsForEdge( Tile::TOPRIGHT ),
                        {{at(x-1,0), Tile::LEFT}}
                ))
                {
                    std::cout << "Fail at top right corner" << std::endl;
                    return false;
                }
               // std::cout << "- Top edge ok" << std::endl;

                x=0;
                /* Tile out left edge row */
                for (y=1; y<grid_size-1; ++y)
                {
                    at(x, y) = at(x, y-1)->getOppositeTile( Tile::BOTTOM );
                    if (! at(x, y))
                    {
                        std::cout << "Failed left at y=" << y << std::endl;
                        return false;
                    }
                    if (! at(x, y)->fit(
                            at(x,y)->findXfrmsForEdge(Tile::LEFT),
                            {{at(x,y-1), Tile::TOP}}
                    )){
                        std::cout << "Fail left edge at y=" << y << std::endl;
                        return false;
                    }
                }

                /* Tile out bottom left corner */
                at(x, y) = at(x, y-1)->getOppositeTile( Tile::BOTTOM );
                if (! at(x, y)->fit(
                        at(x, y)->findXfrmsForEdge( Tile::BOTTOMLEFT ),
                        {{at(x,y-1), Tile::TOP}}
                ))
                {
                    std::cout << "Fail at bottom left corner" << std::endl;
                    return false;
                }

                // std::cout << "- Left edge ok" << std::endl;

                /* Tile out bottom edge row */
                y = grid_size-1;
                for (x = 1; x<grid_size-1; ++x)
                {
                    at(x, y) = at(x-1, y)->getOppositeTile( Tile::RIGHT );
                    if (! at(x, y))
                    {
                        std::cout << "Fail bottom at x=" << x << std::endl;
                        return false;
                    }
                    if (! at(x,y)->fit(
                            at(x,y)->findXfrmsForEdge(Tile::BOTTOM),
                            {{at(x-1,y), Tile::LEFT}}
                    )) {
                        std::cout << "Fail bottom edge at x=" << x << std::endl;
                        return false;
                    }
                }

                /* Tile out bottom right corner */
                at(x,y) = at(x-1, y)->getOppositeTile( Tile::RIGHT );
                if (! at(x,y)->fit(
                        at(x, y)->findXfrmsForEdge( Tile::BOTTOMRIGHT ),
                        {{at(x-1,y), Tile::LEFT}}
                )) {
                    std::cout << "Fail at bottom right corner" << std::endl;
                    return false;
                }

                // std::cout << "- Bottom edge ok" << std::endl;

                /* Tile out right edge */
                x = grid_size-1;
                for (y=1; y<grid_size-1;++y)
                {
                    at(x, y) = at(x, y-1)->getOppositeTile( Tile::BOTTOM );
                    if (! at(x, y))
                    {
                        std::cout << "Fail right at y=" << y << std::endl;
                        return false;
                    }
                    if (! at(x,y)->fit(
                             at(x,y)->findXfrmsForEdge(Tile::RIGHT),
                            {{at(x,y-1), Tile::TOP}}
                    )) {
                        std::cout << "Fail right edge at y=" << y << std::endl;
                        return false;
                    }
                }

                // std::cout << "- Right edge ok" << std::endl;

                /* Now fill in remaining cells, all cels can now be matched with TOP and LEFT edge */
                for (y=1; y<grid_size-1; ++y)
                {
                    /* Place all the tiles based on the top row */
                    for (x=1; x<grid_size-1; ++x)
                    {
                        at(x, y) = at(x,y-1)->getOppositeTile( Tile::BOTTOM );
                        if (! at(x, y))
                        {
                            std::cout << "Fail for (" << x << "," << y << ")" << std::endl;
                            return false;
                        }
                    }

                    /* Orientate all the tiles based on adjacent tile match, then on pattern match */
                    for (x=1; x<grid_size-1; ++x)
                    {
                        std::vector<Tile::Xfrm> positions;
                        for (auto &xfrm : Tile::Xfrm::all() )
                        {
                            if (at(x-1, y) != at(x,y)->getOppositeTile( Tile::LEFT , xfrm) ) continue;
                            if (at(x, y-1) != at(x,y)->getOppositeTile( Tile::TOP, xfrm )) continue;
                            if (at(x+1, y) != at(x,y)->getOppositeTile( Tile::RIGHT, xfrm )) continue;

                            positions.emplace_back(xfrm);
                        }

                        if (positions.empty())
                        {
                            std::cout << "Fail 2 for (" << x << "," << y << ")" << std::endl;
                            return false;
                        }

                        auto fits = at(x,y)->findFits(Tile::Xfrm::all(), {{at(x,y-1), Tile::TOP}});
                        if (fits.empty())
                        {
                            std::cout << "Fail 3 for (" << x << "," << y << ")" << std::endl;
                            return false;
                        }

                        auto fits2 = at(x,y)->findFits(fits, {{at(x-1,y), Tile::LEFT}});
                        if (fits2.empty())
                        {
                            std::cout << "Fail 4 for (" << x << "," << y << ")" << std::endl;
                            for (auto& f : fits)
                            {
                                std::cout << "- " << f.toString() << std::endl;
                            }
                            return false;
                        }

                        at(x,y)->setTransformation(fits2[0]);

                        #if 0
                        if (! at(x,y)->fit(
                                Tile::Xfrm::all(),
                                {{at(x-1,y), Tile::LEFT},
                                {at(x,y-1), Tile::TOP}}
                        )) {
                            std::cout << "Fail 3 for (" << x << "," << y << ")" << std::endl;
                            return false;
                        }
                        #endif
                    }
                }

                // std::cout << "- Grid ok" << std::endl;

                return true;

            }
            catch (std::invalid_argument& e)
            {
                std::cout << "- Ex :" << e.what() << std::endl;
            }

            return false;
        }

        bool _try_assemble(std::shared_ptr<Tile> first)
        {
            grid.resize(grid_size * grid_size);

            // std::cout << "Try assemble with " << first->toString() << " at top left" << std::endl;
            auto positions = first->findXfrmsForEdge( Tile::TOPLEFT );
            for (auto& xfrm : positions )
            {
                if (_try_assemble2(first, xfrm))
                {
                    return true;
                }
                return false;
            }

            return false;
        }

        size_t grid_size;

        std::vector<std::shared_ptr<Tile>> tiles;
        std::vector<std::shared_ptr<Tile>> grid;

        int64_t cornerProduct;

        std::vector<bool> pixels;
        std::vector<bool> rawpixels;
        size_t roughness;

        size_t pixelCoord(size_t x, size_t y)
        {
            return pixelCoord(x, y, Tile::Xfrm());
        }

        size_t pixelCoord(size_t x, size_t y, const Tile::Xfrm& xfrm)
        {
            auto c = xfrm.transformCoord(x, y, grid_size*8);

            return (c.first + (c.second * grid_size *8));
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

    Image image;

    std::unordered_map<unsigned,std::shared_ptr<Tile>> tiles;

    try
    {
        std::ifstream infile(argv[1]);

        const std::regex matchrule_w("^\\s*$");
        const std::regex matchrule_tileid("^Tile ([0-9]+):\\s*$");
        const std::regex matchrule_tiledata("^([.#]+)\\s*$");

        std::string line;
        std::vector<std::string> stack;
        unsigned last_id;

        while (std::getline(infile, line))
        {
            std::smatch match;
            if (std::regex_match(line, match, matchrule_tileid))
            {
                if (! stack.empty())
                {
                    image.addTile( std::make_shared<Tile>(last_id, stack) );
                }
                stack.clear();
                last_id = std::stoi(match[1].str());
            }
            else if (std::regex_match(line, match, matchrule_tiledata))
            {
                stack.emplace_back(match[1].str());
            }
        }

        if (! stack.empty())
        {
            image.addTile( std::make_shared<Tile>(last_id, stack) );
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Reading datafilenamea error: " << e.what() << std::endl;
        return 0;
    }

    image.scanMatchingTiles();
    if ( ! image.assemble() )
    {
        std::cout << "Could not assemble !!" << std::endl;
        return 0;
    }

    image.printLayout();
    std::cout << image.getTilesCount() << " tiles found." << std::endl;
    std::cout << "Grid size " << image.getGridSize() << std::endl;
    std::cout << "Corner product " << image.getCornerProduct() << std::endl;

    image.printRawImage();
    image.generateRawImage();

    for (auto& xfrm : Tile::Xfrm::all() )
    {
        image.generateImage(xfrm);

        auto nm = image.findMonsters();
        if (! nm.empty())
        {
            image.printImage(nm);
            nm.clear();
            std::cout << std::endl;
            std::cout << "Sea roughness : " << image.getRoughness() << std::endl;

            return 0;
        }
    }

    return 0;
}
