use std::io::BufRead;
use std::str::FromStr;

enum Direction {
    N, S, E, W,
    NE, NW, SE, SW
}

impl std::fmt::Display for Direction {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Direction::N => f.write_str("N"),
            Direction::S => f.write_str("S"),
            Direction::E => f.write_str("E"),
            Direction::W => f.write_str("W"),
            Direction::NE => f.write_str("NE"),
            Direction::NW => f.write_str("NW"),
            Direction::SE => f.write_str("SE"),
            Direction::SW => f.write_str("SW"),
        }
    }
}

struct Coord {
    x: usize,
    y: usize,
}

impl Coord {
    fn new(x: usize, y: usize) -> Self {
        Coord { x, y }
    }

    fn north(&self) -> usize {
        self.y - 1
    }

    fn south(&self) -> usize {
        self.y + 1
    }

    fn east(&self) -> usize {
        self.x + 1
    }

    fn west(&self) -> usize {
        self.x - 1
    }

    fn coord(&self, direction: Direction) -> Self {
        match direction {
            Direction::N => Coord {
                x: self.x,
                y: self.north()
            },
            Direction::S => Coord {
                x: self.x,
                y: self.south()
            },
            Direction::E => Coord {
                x: self.east(),
                y: self.y
            },
            Direction::W => Coord {
                x: self.west(),
                y: self.y
            },
            Direction::NE => Coord {
                x: self.east(),
                y: self.north()
            },
            Direction::NW => Coord {
                x: self.west(),
                y: self.north(),
            },
            Direction::SE => Coord {
                x: self.east(),
                y: self.south(),
            },
            Direction::SW => Coord {
                x: self.west(),
                y: self.south(),
            }
        }
    }
}

impl std::fmt::Display for Coord {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.x.fmt(f)?;
        f.write_str(":")?;
        self.y.fmt(f)?;

        Ok(())
    }
}

impl Clone for Coord {
    fn clone(&self) -> Self {
        Coord {
            x: self.x,
            y: self.y,
        }
    }
}

impl Copy for Coord {}

const CELL_ALIVE: u8 = 0b01;
const CELL_CHECKED: u8 = 0b10;

struct Cell(u8);

impl Cell {
    fn new() -> Self {
        Cell(0)
    }

    fn is_alive(&self) -> bool {
        self.0 & CELL_ALIVE != 0
    }

    fn is_checked(&self) -> bool {
        self.0 & CELL_CHECKED != 0
    }

    fn set_alive(&mut self) {
        self.0 |= CELL_ALIVE;
    }

    fn set_checked(&mut self) {
        self.0 |= CELL_CHECKED;
    }
}

impl Clone for Cell {
    fn clone(&self) -> Self {
        Cell(self.0)
    }
}

impl Copy for Cell {}

struct World {
    grid: Vec<Vec<Cell>>,
    next_grid: Vec<Vec<Cell>>,
    alive: Vec<Coord>,
    next_alive: Vec<Coord>,
    x_size: usize,
    y_size: usize,
    x_max: usize,
    y_max: usize,
}

impl World {
    fn new(x_size: usize, y_size: usize) -> Self {
        let x_max = if x_size != 0 { x_size - 1 } else { 0 };
        let y_max = if y_size != 0 { y_size - 1 } else { 0 };

        let mut grid = Vec::with_capacity(x_size);

        for _ in 0..x_size {
            let mut nested = Vec::with_capacity(y_size);

            for _ in 0..y_size {
                nested.push(Cell::new());
            }

            grid.push(nested);
        }

        let next_grid = grid.clone();

        World {
            grid,
            next_grid,
            alive: Vec::new(),
            next_alive: Vec::new(),
            x_size,
            y_size,
            x_max,
            y_max,
        }
    }

    fn is_checked(&self, check: &Coord) -> bool {
        self.next_grid[check.x][check.y].is_checked()
    }

    fn is_alive(&self, check: &Coord) -> bool {
        self.next_grid[check.x][check.y].is_alive()
    }

    fn set_checked(&mut self, coord: &Coord) {
        self.next_grid[coord.x][coord.y].set_checked();
    }

    fn set_alive(&mut self, coord: &Coord) {
        self.next_grid[coord.x][coord.y].set_alive();
    }

    fn spawn(&mut self, coord: Coord) -> bool {
        if self.is_alive(&coord) {
            return false;
        }

        println!("spawning cell at {}", coord);

        self.set_alive(&coord);
        self.next_alive.push(coord);

        true
    }

    fn neighbour_alive(&self, coord: &Coord, direction: Direction) -> bool {
        if (match direction {
            Direction::N => self.grid[coord.x][coord.north()],
            Direction::S => self.grid[coord.x][coord.south()],
            Direction::E => self.grid[coord.east()][coord.y],
            Direction::W => self.grid[coord.west()][coord.y],
            Direction::NE => self.grid[coord.east()][coord.north()],
            Direction::NW => self.grid[coord.west()][coord.north()],
            Direction::SE => self.grid[coord.east()][coord.south()],
            Direction::SW => self.grid[coord.west()][coord.south()],
        }).is_alive() {
            print!(" {}", direction);
            true
        } else {
            false
        }
    }

    fn neighbours(&self, cell: &Coord) -> u8 {
        let mut neighbours = 0;

        if cell.x == 0 {
            if cell.y == 0 {
                // o x
                // x x
                let checking = [Direction::E, Direction::S, Direction::SE];

                for check in checking {
                    if self.neighbour_alive(cell, check) {
                        neighbours += 1;
                    }
                }
            } else if cell.y == self.y_max {
                // x x
                // o x
                let checking = [Direction::N, Direction::NE, Direction::E];

                for check in checking {
                    if self.neighbour_alive(cell, check) {
                        neighbours += 1;
                    }
                }
            } else {
                // x x
                // o x
                // x x
                let checking = [Direction::N, Direction::NE, Direction::E, Direction::S, Direction::SE];

                for check in checking {
                    if self.neighbour_alive(cell, check) {
                        neighbours += 1;
                    }
                }
            }
        } else if cell.x == self.x_max {
            if cell.y == 0 {
                // x o
                // x x
                let checking = [Direction::W, Direction::SW, Direction::S];

                for check in checking {
                    if self.neighbour_alive(cell, check) {
                        neighbours += 1;
                    }
                }
            } else if cell.y == self.y_max {
                // x x
                // x o
                let checking = [Direction::NW, Direction::N, Direction::W];

                for check in checking {
                    if self.neighbour_alive(cell, check) {
                        neighbours += 1;
                    }
                }
            } else {
                // x x
                // x o
                // x x
                let checking = [Direction::NW, Direction::N, Direction::W, Direction::SW, Direction::S];

                for check in checking {
                    if self.neighbour_alive(cell, check) {
                        neighbours += 1;
                    }
                }
            }
        } else {
            if cell.y == 0 {
                // x o x
                // x x x
                let checking = [Direction::W, Direction::E, Direction::SW, Direction::S, Direction::SE];

                for check in checking {
                    if self.neighbour_alive(cell, check) {
                        neighbours += 1;
                    }
                }
            } else if cell.y == self.y_max {
                // x x x
                // x o x
                let checking = [Direction::NW, Direction::N, Direction::NE, Direction::W, Direction::E];

                for check in checking {
                    if self.neighbour_alive(cell, check) {
                        neighbours += 1;
                    }
                }
            } else {
                // x x x
                // x o x
                // x x x
                let checking = [Direction::NW, Direction::N, Direction::NE, Direction::W, Direction::E, Direction::SW, Direction::S, Direction::SE];

                for check in checking {
                    if self.neighbour_alive(cell, check) {
                        neighbours += 1;
                    }
                }
            }
        }

        print!(" {}\n", neighbours);

        neighbours
    }

    fn check_spawn(&mut self, coord: Coord) -> bool {
        if self.is_checked(&coord) {
            println!("check spawn {} already checked", coord);
            return false;
        }

        print!("check spawn {}", coord);

        let neighbours = self.neighbours(&coord);

        self.set_checked(&coord);

        if self.grid[coord.x][coord.y].is_alive() {
            if neighbours == 2 || neighbours == 3 {
                return self.spawn(coord);
            }
        } else {
            if neighbours == 3 {
                return self.spawn(coord);
            }
        }

        false
    }

    fn tick(&mut self) {
        let alive = std::mem::take(&mut self.alive);

        println!("currently alive cells: {}", alive.len());

        for cell in &alive {
            self.check_spawn(*cell);

            if cell.x == 0 {
                if cell.y == 0 {
                    // o x
                    // x x
                    let checking = [Direction::E, Direction::S, Direction::SE];

                    for check in checking {
                        self.check_spawn(cell.coord(check));
                    }
                } else if cell.y == self.y_max {
                    // x x
                    // o x
                    let checking = [Direction::N, Direction::NE, Direction::E];

                    for check in checking {
                        self.check_spawn(cell.coord(check));
                    }
                } else {
                    // x x
                    // o x
                    // x x
                    let checking = [Direction::N, Direction::NE, Direction::E, Direction::S, Direction::SE];

                    for check in checking {
                        self.check_spawn(cell.coord(check));
                    }
                }
            } else if cell.x == self.x_max {
                if cell.y == 0 {
                    // x o
                    // x x
                    let checking = [Direction::W, Direction::SW, Direction::S];

                    for check in checking {
                        self.check_spawn(cell.coord(check));
                    }
                } else if cell.y == self.y_max {
                    // x x
                    // x o
                    let checking = [Direction::NW, Direction::N, Direction::W];

                    for check in checking {
                        self.check_spawn(cell.coord(check));
                    }
                } else {
                    // x x
                    // x o
                    // x x
                    let checking = [Direction::NW, Direction::N, Direction::W, Direction::SW, Direction::S];

                    for check in checking {
                        self.check_spawn(cell.coord(check));
                    }
                }
            } else {
                if cell.y == 0 {
                    // x o x
                    // x x x
                    let checking = [Direction::W, Direction::E, Direction::SW, Direction::S, Direction::SE];

                    for check in checking {
                        self.check_spawn(cell.coord(check));
                    }
                } else if cell.y == self.y_max {
                    // x x x
                    // x o x
                    let checking = [Direction::NW, Direction::N, Direction::NE, Direction::W, Direction::E];

                    for check in checking {
                        self.check_spawn(cell.coord(check));
                    }
                } else {
                    // x x x
                    // x o x
                    // x x x
                    let checking = [Direction::NW, Direction::N, Direction::NE, Direction::W, Direction::E, Direction::SW, Direction::S, Direction::SE];

                    for check in checking {
                        self.check_spawn(cell.coord(check));
                    }
                }
            }
        }

        let _ = std::mem::replace(&mut self.alive, alive);
    }

    fn update(&mut self) {
        std::mem::swap(&mut self.grid, &mut self.next_grid);

        for nested in &mut self.next_grid {
            nested.fill(Cell::new());
        }

        std::mem::swap(&mut self.alive, &mut self.next_alive);
    }

    fn to_writer(&self, writer: &mut impl std::io::Write) -> Result<(), std::io::Error> {
        for y_index in 0..self.y_size {
            for x_index in 0..self.x_size {
                if self.grid[x_index][y_index].is_alive() {
                    writer.write(b"1")?;
                } else {
                    writer.write(b" ")?;
                }
            }

            writer.write(b"\n")?;
            writer.flush()?;
        }

        Ok(())
    }
}

fn main() {
    let mut generations: usize = 2;
    let mut args = std::env::args();
    args.next();

    let Some(file_string) = args.next() else {
        println!("no file path specified");
        return;
    };

    let Ok(file) = std::fs::OpenOptions::new()
        .read(true)
        .open(&file_string) else {
        println!("failed to open the desired file");
        return;
    };

    if let Some(generation_str) = args.next() {
        let Ok(parsed): Result<usize, _> = FromStr::from_str(&generation_str) else {
            println!("failed to parsed generations amount");
            return;
        };

        generations = parsed;
    }

    let mut lines = std::io::BufReader::new(file).lines();

    let Some(grid_size) = lines.next() else {
        println!("grid size was not provided");
        return;
    };

    let mut world: World;

    if let Ok(grid_size) = grid_size {
        let Some((x_str, y_str)) = grid_size.split_once(':') else {
            println!("invalid grid size format");
            return;
        };

        let Ok(x_size): Result<usize, _> = FromStr::from_str(x_str) else {
            println!("invalid grid x size value");
            return;
        };

        let Ok(y_size): Result<usize, _> = FromStr::from_str(y_str) else {
            println!("invalid grid y size value");
            return;
        };

        world = World::new(x_size, y_size);
    } else {
        println!("failed to read grid size from file");
        return;
    }

    while let Some(check) = lines.next() {
        let Ok(line) = check else {
            println!("failed to parse cell coordiate");
            return;
        };

        let Some((x_str, y_str)) = line.split_once(',') else {
            println!("invalid coordinate format");
            return;
        };

        let Ok(x_pos): Result<usize, _> = FromStr::from_str(x_str) else {
            println!("invalid x pos value");
            return;
        };

        let Ok(y_pos): Result<usize, _> = FromStr::from_str(y_str) else {
            println!("invalid y pos value");
            return;
        };

        world.spawn(Coord::new(x_pos, y_pos));
    }

    world.update();

    {
        let file_name = format!("rust_initial.txt");

        let Ok(file) = std::fs::OpenOptions::new()
            .write(true)
            .truncate(true)
            .create(true)
            .open(&file_name) else {
            println!("failed to open initial file");
            return;
        };
        let mut writer = std::io::BufWriter::new(file);

        if let Err(_e) = world.to_writer(&mut writer) {
            println!("failed to write initial to file");
            return;
        }
    }

    let mut current_gen: usize = 1;

    while current_gen <= generations {
        println!("---------- processing generation {}", current_gen);

        world.tick();
        world.update();

        {
            let file_name = format!("rust_generation_{}.txt", current_gen);

            let Ok(file) = std::fs::OpenOptions::new()
                .write(true)
                .truncate(true)
                .create(true)
                .open(&file_name) else {
                println!("failed to open generation file");
                return;
            };
            let mut writer = std::io::BufWriter::new(file);

            if let Err(_e) = world.to_writer(&mut writer) {
                println!("failed to write generation to file");
                return;
            }
        }

        current_gen += 1;
    }
}
