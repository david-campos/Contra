//
// Created by david on 8/2/20.
//

#include "grid.h"
#include "CollideComponent.h"

void Grid::Update(CollideComponent *collider) {
    CellsSquare occupied_now{}, occupied_before{};
    collider->GetPreviouslyOccupiedCells(occupied_before);
    for (int y = occupied_before.min_cell_y; y <= occupied_before.max_cell_y; y++) {
        for (int x = occupied_before.min_cell_x; x <= occupied_before.max_cell_x; x++) {
            GetCell(x, y)->Remove(collider, collider->GetLayer());
        }
    }
    collider->GetOccupiedCells(occupied_now);
    for (int y = occupied_now.min_cell_y; y <= occupied_now.max_cell_y; y++) {
        for (int x = occupied_now.min_cell_x; x <= occupied_now.max_cell_x; x++) {
            GridCell *cell = GetCell(x, y);
            cell->Add(collider, collider->GetLayer());
        }
    }
}

void Grid::Remove(CollideComponent *collider) {
    CellsSquare occupied_before{};
    collider->GetPreviouslyOccupiedCells(occupied_before, false);
    for (int y = occupied_before.min_cell_y; y <= occupied_before.max_cell_y; y++) {
        for (int x = occupied_before.min_cell_x; x <= occupied_before.max_cell_x; x++) {
            GetCell(x, y)->Remove(collider, collider->GetLayer());
        }
    }
}

