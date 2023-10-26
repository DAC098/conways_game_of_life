#include<cstdio>
#include<iostream>
#include<algorithm>
#include<iterator>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<utility>
#include<optional>
#include<stdio.h>

struct Coord
{
    size_t x;
    size_t y;

    Coord(size_t x, size_t y) :
        x(x), y(y)
    {}

    size_t north()
    {
        return this->y - 1;
    }

    size_t south()
    {
        return this->y + 1;
    }

    size_t west()
    {
        return this->x - 1;
    }

    size_t east()
    {
        return this->x + 1;
    }

    Coord coord_north_west()
    {
        return Coord(this->west(), this->north());
    }

    Coord coord_north()
    {
        return Coord(this->x, this->north());
    }

    Coord coord_north_east()
    {
        return Coord(this->east(), this->north());
    }

    Coord coord_west()
    {
        return Coord(this->west(), this->y);
    }

    Coord coord_east()
    {
        return Coord(this->east(), this->y);
    }

    Coord coord_south_west()
    {
        return Coord(this->west(), this->south());
    }

    Coord coord_south()
    {
        return Coord(this->x, this->south());
    }

    Coord coord_south_east()
    {
        return Coord(this->east(), this->south());
    }

};

using CoordList = std::vector<Coord>;

const unsigned char CELL_ALIVE   = 0b01;
const unsigned char CELL_CHECKED = 0b10;

enum class Direction {
    N,
    S,
    E,
    W,
    NW,
    NE,
    SW,
    SE
};

struct World
{
    size_t x_size = 0;
    size_t y_size = 0;
    size_t x_max = 0;
    size_t y_max = 0;

    std::vector<std::vector<unsigned char>> grid;
    std::vector<std::vector<unsigned char>> next_grid;
    std::vector<Coord> alive;
    std::vector<Coord> next_alive;

    World(size_t x_size, size_t y_size) :
        x_size(x_size), y_size(y_size),
        grid(x_size, std::vector<unsigned char>(y_size)),
        next_grid(x_size, std::vector<unsigned char>(y_size))
    {
        if (x_size != 0)
        {
            this->x_max = x_size - 1;
        }

        if (y_size != 0)
        {
            this->y_max = y_size - 1;
        }
    }

    bool is_checked(Coord& cell)
    {
        return this->next_grid[cell.x][cell.y] & CELL_CHECKED;
    }

    bool is_alive(Coord& cell)
    {
        return this->next_grid[cell.x][cell.y] & CELL_ALIVE;
    }

    void set_checked(Coord& cell)
    {
        this->next_grid[cell.x][cell.y] |= CELL_CHECKED;
    }

    void set_alive(Coord& cell)
    {
        this->next_grid[cell.x][cell.y] |= CELL_ALIVE;
    }

    bool spawn(Coord& cell)
    {
        if (this->is_alive(cell))
        {
            return false;
        }

        printf("spawning cell at %zu:%zu\n", cell.x, cell.y);

        this->set_alive(cell);
        this->next_alive.push_back(cell);

        return true;
    }

    bool check_spawn(Coord& check)
    {
        if (this->is_checked(check))
        {
            printf("try spawn %zu:%zu already checked\n", check.x, check.y);
            return false;
        }

        printf("try spawning %zu:%zu", check.x, check.y);

        unsigned char neighbours = this->neighbours(check);

        this->set_checked(check);

        if (this->grid[check.x][check.y] & CELL_ALIVE)
        {
            if (neighbours == 2 || neighbours == 3)
            {
                return this->spawn(check);
            }
        }
        else
        {
            if (neighbours == 3)
            {
                return this->spawn(check);
            }
        }

        return false;
    }

    unsigned char get_neighbour(Coord& cell, Direction direction)
    {
        switch (direction)
        {
            case Direction::N:
                return this->grid[cell.x][cell.north()];
            case Direction::S:
                return this->grid[cell.x][cell.south()];
            case Direction::E:
                return this->grid[cell.east()][cell.y];
            case Direction::W:
                return this->grid[cell.west()][cell.y];

            case Direction::NE:
                return this->grid[cell.east()][cell.north()];
            case Direction::NW:
                return this->grid[cell.west()][cell.north()];
            case Direction::SE:
                return this->grid[cell.east()][cell.south()];
            case Direction::SW:
                return this->grid[cell.west()][cell.south()];
            default:
                __builtin_unreachable();
        }
    }

    bool neighbour_alive(Coord& cell, Direction direction)
    {
        switch (direction)
        {
            case Direction::N:
                return this->grid[cell.x][cell.north()] & CELL_ALIVE;
            case Direction::S:
                return this->grid[cell.x][cell.south()] & CELL_ALIVE;
            case Direction::E:
                return this->grid[cell.east()][cell.y] & CELL_ALIVE;
            case Direction::W:
                return this->grid[cell.west()][cell.y] & CELL_ALIVE;

            case Direction::NE:
                return this->grid[cell.east()][cell.north()] & CELL_ALIVE;
            case Direction::NW:
                return this->grid[cell.west()][cell.north()] & CELL_ALIVE;
            case Direction::SE:
                return this->grid[cell.east()][cell.south()] & CELL_ALIVE;
            case Direction::SW:
                return this->grid[cell.west()][cell.south()] & CELL_ALIVE;
            default:
                __builtin_unreachable();
        }
    }

    unsigned char neighbours(Coord& cell)
    {
        unsigned char neighbours = 0;

        if (cell.x == 0)
        {
            if (cell.y == 0)
            {
                // o x
                // x x
                if (this->neighbour_alive(cell, Direction::E))
                {
                    printf(" e[%u]", this->get_neighbour(cell, Direction::E));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::S))
                {
                    printf(" s[%u]", this->get_neighbour(cell, Direction::S));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::SE))
                {
                    printf(" se[%u]", this->get_neighbour(cell, Direction::SE));
                    neighbours += 1;
                }
            }
            else if (cell.y == this->y_max)
            {
                // x x
                // o x
                if (this->neighbour_alive(cell, Direction::N))
                {
                    printf(" n[%u]", this->get_neighbour(cell, Direction::N));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::NE))
                {
                    printf(" ne[%u]", this->get_neighbour(cell, Direction::NE));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::E))
                {
                    printf(" e[%u]", this->get_neighbour(cell, Direction::E));
                    neighbours += 1;
                }
            }
            else
            {
                // x x
                // o x
                // x x
                if (this->neighbour_alive(cell, Direction::N))
                {
                    printf(" n[%u]", this->get_neighbour(cell, Direction::N));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::NE))
                {
                    printf(" ne[%u]", this->get_neighbour(cell, Direction::NE));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::E))
                {
                    printf(" e[%u]", this->get_neighbour(cell, Direction::E));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::S))
                {
                    printf(" s[%u]", this->get_neighbour(cell, Direction::S));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::SE))
                {
                    printf(" se[%u]", this->get_neighbour(cell, Direction::SE));
                    neighbours += 1;
                }
            }
        }
        else if (cell.x == this->x_max)
        {
            if (cell.y == 0)
            {
                // x o
                // x x
                if (this->neighbour_alive(cell, Direction::W))
                {
                    printf(" w[%u]", this->get_neighbour(cell, Direction::W));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::SW))
                {
                    printf(" sw[%u]", this->get_neighbour(cell, Direction::SW));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::S))
                {
                    printf(" s[%u]", this->get_neighbour(cell, Direction::S));
                    neighbours += 1;
                }
            }
            else if (cell.y == this->y_max)
            {
                // x x
                // x o
                if (this->neighbour_alive(cell, Direction::NW))
                {
                    printf(" nw[%u]", this->get_neighbour(cell, Direction::NW));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::N))
                {
                    printf(" n[%u]", this->get_neighbour(cell, Direction::N));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::W))
                {
                    printf(" w[%u]", this->get_neighbour(cell, Direction::W));
                    neighbours += 1;
                }
            }
            else
            {
                // x x
                // x o
                // x x
                if (this->neighbour_alive(cell, Direction::NW))
                {
                    printf(" nw[%u]", this->get_neighbour(cell, Direction::NW));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::N))
                {
                    printf(" n[%u]", this->get_neighbour(cell, Direction::N));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::W))
                {
                    printf(" w[%u]", this->get_neighbour(cell, Direction::W));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::SW))
                {
                    printf(" sw[%u]", this->get_neighbour(cell, Direction::SW));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::S))
                {
                    printf(" s[%u]", this->get_neighbour(cell, Direction::S));
                    neighbours += 1;
                }
            }
        }
        else
        {
            if (cell.y == 0)
            {
                // x o x
                // x x x
                if (this->neighbour_alive(cell, Direction::W))
                {
                    printf(" w[%u]", this->get_neighbour(cell, Direction::W));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::E))
                {
                    printf(" e[%u]", this->get_neighbour(cell, Direction::E));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::SW))
                {
                    printf(" sw[%u]", this->get_neighbour(cell, Direction::SW));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::S))
                {
                    printf(" s[%u]", this->get_neighbour(cell, Direction::S));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::SE))
                {
                    printf(" se[%u]", this->get_neighbour(cell, Direction::SE));
                    neighbours += 1;
                }
            }
            else if(cell.y == this->y_max)
            {
                // x x x
                // x o x
                if (this->neighbour_alive(cell, Direction::NW))
                {
                    printf(" nw[%u]", this->get_neighbour(cell, Direction::NW));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::N))
                {
                    printf(" n[%u]", this->get_neighbour(cell, Direction::N));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::NE))
                {
                    printf(" ne[%u]", this->get_neighbour(cell, Direction::NE));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::W))
                {
                    printf(" w[%u]", this->get_neighbour(cell, Direction::W));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::E))
                {
                    printf(" e[%u]", this->get_neighbour(cell, Direction::E));
                    neighbours += 1;
                }
            }
            else
            {
                // x x x
                // x o x
                // x x x
                if (this->neighbour_alive(cell, Direction::NW))
                {
                    printf(" nw[%u]", this->get_neighbour(cell, Direction::NW));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::N))
                {
                    printf(" n[%u]", this->get_neighbour(cell, Direction::N));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::NE))
                {
                    printf(" ne[%u]", this->get_neighbour(cell, Direction::NE));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::W))
                {
                    printf(" w[%u]", this->get_neighbour(cell, Direction::W));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::E))
                {
                    printf(" e[%u]", this->get_neighbour(cell, Direction::E));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::SW))
                {
                    printf(" sw[%u]", this->get_neighbour(cell, Direction::SW));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::S))
                {
                    printf(" s[%u]", this->get_neighbour(cell, Direction::S));
                    neighbours += 1;
                }

                if (this->neighbour_alive(cell, Direction::SE))
                {
                    printf(" se[%u]", this->get_neighbour(cell, Direction::SE));
                    neighbours += 1;
                }
            }
        }

        printf(" %u\n", neighbours);

        return neighbours;
    }

    void tick()
    {
        printf("currently alive cells: %zu\n", this->alive.size());

        for (size_t index = 0; index < this->alive.size(); ++index)
        {
            this->check_spawn(this->alive[index]);

            // check surrounding cells
            if (this->alive[index].x == 0)
            {
                if (this->alive[index].y == 0)
                {
                    // o x
                    // x x
                    Coord east = this->alive[index].coord_east();
                    Coord south_east = this->alive[index].coord_south_east();
                    Coord south = this->alive[index].coord_south();

                    this->check_spawn(east);
                    this->check_spawn(south_east);
                    this->check_spawn(south);
                }
                else if (this->alive[index].y == this->y_max)
                {
                    // x x
                    // o x
                    Coord north = this->alive[index].coord_north();
                    Coord north_east = this->alive[index].coord_north_east();
                    Coord east = this->alive[index].coord_east();

                    this->check_spawn(north);
                    this->check_spawn(north_east);
                    this->check_spawn(east);
                }
                else
                {
                    // x x
                    // o x
                    // x x
                    Coord north = this->alive[index].coord_north();
                    Coord north_east = this->alive[index].coord_north_east();
                    Coord east = this->alive[index].coord_east();
                    Coord south = this->alive[index].coord_south();
                    Coord south_east = this->alive[index].coord_south_east();

                    this->check_spawn(north);
                    this->check_spawn(north_east);
                    this->check_spawn(east);
                    this->check_spawn(south);
                    this->check_spawn(south_east);
                }
            }
            else if (this->alive[index].x == this->x_max)
            {
                if (this->alive[index].y == 0)
                {
                    // x o
                    // x x
                    Coord west = this->alive[index].coord_west();
                    Coord south_west = this->alive[index].coord_south_west();
                    Coord south = this->alive[index].coord_south();

                    this->check_spawn(west);
                    this->check_spawn(south_west);
                    this->check_spawn(south);
                }
                else if (this->alive[index].y == this->y_max)
                {
                    // x x
                    // x o
                    Coord north_west = this->alive[index].coord_north_west();
                    Coord north = this->alive[index].coord_north();
                    Coord west = this->alive[index].coord_west();

                    this->check_spawn(north_west);
                    this->check_spawn(north);
                    this->check_spawn(west);
                }
                else
                {
                    // x x
                    // x o
                    // x x
                    Coord north_west = this->alive[index].coord_north_west();
                    Coord north = this->alive[index].coord_north();
                    Coord west = this->alive[index].coord_west();
                    Coord south_west = this->alive[index].coord_south_west();
                    Coord south = this->alive[index].coord_south();

                    this->check_spawn(north_west);
                    this->check_spawn(north);
                    this->check_spawn(west);
                    this->check_spawn(south_west);
                    this->check_spawn(south);
                }
            }
            else
            {
                if (this->alive[index].y == 0)
                {
                    // x o x
                    // x x x
                    Coord west = this->alive[index].coord_west();
                    Coord east = this->alive[index].coord_east();
                    Coord south_west = this->alive[index].coord_south_west();
                    Coord south = this->alive[index].coord_south();
                    Coord south_east = this->alive[index].coord_south_east();

                    this->check_spawn(west);
                    this->check_spawn(east);
                    this->check_spawn(south_west);
                    this->check_spawn(south);
                    this->check_spawn(south_east);
                }
                else if(this->alive[index].y == this->y_max)
                {
                    // x x x
                    // x o x
                    Coord north_west = this->alive[index].coord_north_west();
                    Coord north = this->alive[index].coord_north();
                    Coord north_east = this->alive[index].coord_north_east();
                    Coord west = this->alive[index].coord_west();
                    Coord east = this->alive[index].coord_east();

                    this->check_spawn(north_west);
                    this->check_spawn(north);
                    this->check_spawn(north_east);
                    this->check_spawn(west);
                    this->check_spawn(east);
                }
                else
                {
                    // x x x
                    // x o x
                    // x x x
                    Coord north_west = this->alive[index].coord_north_west();
                    Coord north = this->alive[index].coord_north();
                    Coord north_east = this->alive[index].coord_north_east();
                    Coord west = this->alive[index].coord_west();
                    Coord east = this->alive[index].coord_east();
                    Coord south_west = this->alive[index].coord_south_west();
                    Coord south = this->alive[index].coord_south();
                    Coord south_east = this->alive[index].coord_south_east();

                    this->check_spawn(north_west);
                    this->check_spawn(north);
                    this->check_spawn(north_east);
                    this->check_spawn(west);
                    this->check_spawn(east);
                    this->check_spawn(south_west);
                    this->check_spawn(south);
                    this->check_spawn(south_east);
                }
            }
        }
    }

    void update()
    {
        this->grid.swap(this->next_grid);
        this->alive.swap(this->next_alive);

        for (size_t col = 0; col < this->x_size; ++col)
        {
            std::fill(this->next_grid[col].begin(), this->next_grid[col].end(), 0);
        }

        this->next_alive.clear();
    }

    bool to_file(std::string file_name)
    {
        std::ofstream file(file_name);

        if (!file.is_open())
        {
            return false;
        }

        for (size_t y_index = 0; y_index < this->y_size; ++y_index)
        {
            for (size_t x_index = 0; x_index < this->x_size; ++x_index)
            {
                if (this->grid[x_index][y_index] & CELL_ALIVE)
                {
                    file << '1';
                }
                else
                {
                    file << ' ';
                }
            }

            file << '\n';
        }

        file.close();

        return true;
    }
};

int main(int argc, char** argv)
{
    size_t generations = 2;

    if (argc < 2)
    {
        printf("provide a file to start the game\n");
        return 0;
    }

    std::ifstream input_file(argv[1]);

    if (!input_file.is_open())
    {
        printf("failed to open start file \"%s\"\n", argv[1]);
        return 0;
    }

    if (argc == 3)
    {
        if (1 != sscanf(argv[2], "%zu", &generations))
        {
            printf("failed to parse generations amount \"%s\"", argv[2]);
            return 0;
        }
    }

    if (generations == 0)
    {
        printf("generations specified is zero\n");
        return 0;
    }

    std::string line;
    size_t x_size = 0;
    size_t y_size = 0;

    // get the grid size
    if (std::getline(input_file, line)) 
    {
        if (2 != sscanf(line.c_str(), "%zu:%zu", &x_size, &y_size))
        {
            printf("invalid grid size of first line of file\n");
            return 0;
        }

        if (x_size < 3 || y_size < 3)
        {
            printf("grid size is too small. x and y must be greater than 3");
            return 0;
        }
    }
    else
    {
        printf("failed to read first line of file\n");
        return 0;
    }

    World world(x_size, y_size);
    Coord pos(0, 0);

    while (std::getline(input_file, line))
    {
        if (2 != sscanf(line.c_str(), "%zu,%zu", &pos.x, &pos.y))
        {
            printf("failed to parse coordinate %s", line.c_str());
            return 0;
        }

        world.spawn(pos);
    }

    world.update();

    if (!world.to_file("initial.txt"))
    {
        printf("failed to output initial state to file");
        return 0;
    }

    printf("running for %zu generations\n", generations);

    size_t current_gen = 1;

    // begin the game of life
    while (generations--)
    {
        printf("---------- processing generation %zu\n", current_gen);
        world.tick();
        world.update();

        std::ostringstream output_name;

        output_name << "generation_" << current_gen << ".txt";

        world.to_file(output_name.str());

        current_gen += 1;
    }

    return 0;
}

