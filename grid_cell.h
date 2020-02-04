//
// Created by david on 29/1/20.
//

#ifndef LAB5_GRID_CELL_H
#define LAB5_GRID_CELL_H

#include <vector>
#include <algorithm>
#include "game_object.h"
#include "avancezlib.h"

class GridCell {
public:
    std::vector<GameObject *> game_objects;

    void Add(GameObject *game_object) {
        game_objects.push_back(game_object);
    }

    void Remove(GameObject *game_object) {
        auto position = std::find(game_objects.begin(), game_objects.end(), game_object);
        if (position != game_objects.end()) {
            game_objects.erase(position);
        }
    }
};

class Grid {
private:
    std::vector<GridCell> cells;
    int cell_size;
    int row_size;
    int col_size;
public:
    struct CellsSquare {
        int min_cell_x, max_cell_x;
        int min_cell_y, max_cell_y;
    };

    GridCell *GetCell(int x, int y) {
        return &cells[y * row_size + x];
    }

    void GetOccupiedCells(CellsSquare &output, const Vector2D &position) {
        output.min_cell_x = std::min(std::max((int) floor(position.x / cell_size), 0), row_size - 1);
        output.max_cell_x = std::min(std::max((int) floor((position.x + 32) / cell_size), 0), row_size - 1);
        output.min_cell_y = std::min(std::max((int) floor(position.y / cell_size), 0), col_size - 1);
        output.max_cell_y = std::min(std::max((int) floor((position.y + 32) / cell_size), 0), col_size - 1);
    }

    void Create(int cell_size, int width, int height) {
        if (cell_size <= 0) cell_size = 1;
        this->cell_size = cell_size;
        this->row_size = width / cell_size;
        this->col_size = height / cell_size;
        for (int y = 0; y < height; y += cell_size) {
            for (int x = 0; x < width; x += cell_size) {
                GridCell cell;
                cells.push_back(cell);
            }
        }
    }

    void Update(GameObject *go, std::set<GridCell *> &occupiedCells) {
        CellsSquare occupied_now{};
        GetOccupiedCells(occupied_now, go->position);
        for (auto cell: occupiedCells) {
            cell->Remove(go);
        }
        occupiedCells.clear();
        for (int y = occupied_now.min_cell_y; y <= occupied_now.max_cell_y; y++) {
            for (int x = occupied_now.min_cell_x; x <= occupied_now.max_cell_x; x++) {
                GridCell *cell = GetCell(x, y);
                cell->Add(go);
                occupiedCells.insert(cell);
            }
        }
    }
};

#endif //LAB5_GRID_CELL_H
