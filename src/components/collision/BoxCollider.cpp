//
// Created by david on 3/3/20.
//

#include "BoxCollider.h"
#include "../scene.h"

void BoxCollider::GetOccupiedCells(Grid::CellsSquare &square) {
    auto *grid = scene->GetGrid();
    int cell_size = grid->getCellSize();
    int row_size = grid->getRowSize();
    int col_size = grid->getColSize();
    square.min_cell_x = std::min(std::max((int) floor((go->position.x + m_box.top_left_x) / cell_size), 0),
            row_size - 1);
    square.max_cell_x = std::min(std::max((int) floor((go->position.x + m_box.bottom_right_x) / cell_size), 0),
            row_size - 1);
    square.min_cell_y = std::min(std::max((int) floor((go->position.y + m_box.top_left_y) / cell_size), 0),
            col_size - 1);
    square.max_cell_y = std::min(std::max((int) floor((go->position.y + m_box.bottom_right_y) / cell_size), 0),
            col_size - 1);
}

bool BoxCollider::IsColliding(const CollideComponent *other) {
    auto *other_box = dynamic_cast< const BoxCollider * >( other );
    if (other_box) {
        float a_x_min = AbsoluteTopLeftX(),
                a_x_max = AbsoluteBottomRightX(),
                a_y_min = AbsoluteTopLeftY(),
                a_y_max = AbsoluteBottomRightY(),
                b_x_min = other_box->AbsoluteTopLeftX(),
                b_x_max = other_box->AbsoluteBottomRightX(),
                b_y_min = other_box->AbsoluteTopLeftY(),
                b_y_max = other_box->AbsoluteBottomRightY();
        return a_x_max >= b_x_min && b_x_max >= a_x_min
               && a_y_max >= b_y_min && b_y_max >= a_y_min;
    }
}

void BoxCollider::Update(float dt) {
    CollideComponent::Update(dt);
    if (!m_disabled) {
        scene->GetEngine()->strokeSquare(
                AbsoluteTopLeftX() - scene->GetCameraX(), AbsoluteTopLeftY(),
                AbsoluteBottomRightX() - scene->GetCameraX(), AbsoluteBottomRightY(),
                {0, 0, 255});
    }
}
