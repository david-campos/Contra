//
// Created by david on 29/1/20.
//

#ifndef LAB5_GRID_CELL_H
#define LAB5_GRID_CELL_H

#include <vector>
#include <algorithm>
#include <unordered_map>
#include "avancezlib.h"
#include "game_object.h"

class CollideComponent;

#define GRID_CELL_LAYERS 2

class GridCell {
    // We can't use std::set because, after loosing a WHOLE day debugging random errors
    // I found out std::set::erase is not safe while iterating, so we would not be able
    // to erase colliders while other colliders are iterating -.-'
    std::vector<CollideComponent *> colliders[GRID_CELL_LAYERS];
public:
    std::vector<CollideComponent *> *GetLayer(int layer) {
        if (layer < GRID_CELL_LAYERS && layer >= 0) {
            return &colliders[layer];
        }
        return nullptr;
    }

    void Add(CollideComponent *collider, int layer) {
        if (layer < GRID_CELL_LAYERS && layer >= 0) {
            colliders[layer].push_back(collider);
        }
    }

    void Remove(CollideComponent *collider, int layer) {
        if (layer < GRID_CELL_LAYERS && layer >= 0) {
            for (int i = 0; i < colliders[layer].size(); i++) {
                if (colliders[layer][i] == collider) {
                    colliders[layer].erase(colliders[layer].begin() + i);
                }
            }
        }
    }
};

class Grid {
private:
    std::vector<GridCell> cells;
    std::unordered_map<CollideComponent *, std::unordered_map<CollideComponent *, bool>> collision_cache;
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

    /**
     * Clears the collision cache, should be made before starting each frame
     */
    void ClearCollisionCache() {
        collision_cache.clear();
    }

    /**
     * Returns 1 if the collision of a to b was cached as true, 0 if it was false,
     * or -1 if it was not cached.
     * @param a
     * @param b
     * @return
     */
    short GetCollisionCached(CollideComponent *a, CollideComponent *b) const {
        if (collision_cache.count(a) > 0) {
            auto a_cache = collision_cache.at(a);
            if (a_cache.count(b) > 0) {
                return a_cache.at(b) ? 1 : 0;
            }
        }
        return -1;
    }

    void NotifyCacheCollision(CollideComponent *a, CollideComponent *b, bool colliding) {
        if (collision_cache.count(a) == 0) {
            std::pair<CollideComponent*, std::unordered_map<CollideComponent*, bool>> new_cache(a, {});
            collision_cache.insert(new_cache);
        }
        collision_cache.at(a)[b] = colliding;
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

    void Update(CollideComponent *collider);
    void Remove(CollideComponent *collider);

    [[nodiscard]] int getCellSize() const {
        return cell_size;
    }

    [[nodiscard]] int getRowSize() const {
        return row_size;
    }

    [[nodiscard]] int getColSize() const {
        return col_size;
    }
};

#endif //LAB5_GRID_CELL_H
