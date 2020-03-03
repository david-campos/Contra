//
// Created by david on 3/3/20.
//

#include "CollideComponent.h"
#include "../scene.h"

void CollideComponent::Create(BaseScene *scene, GameObject *go, int layer, int checkLayer) {
    Component::Create(scene, go);
    this->m_layer = layer;
    this->m_checkLayer = checkLayer;
}

void CollideComponent::Update(float dt) {
    auto *grid = scene->GetGrid();
    if (m_disabled || !go->IsEnabled()) return;
    if (m_checkLayer >= 0) {
        Grid::CellsSquare square{};
        GetOccupiedCells(square);
        // Check collisions with our layer
        for (int y = square.min_cell_y; y <= square.max_cell_y; y++) {
            for (int x = square.min_cell_x; x <= square.max_cell_x; x++) {
                auto *layer = grid->GetCell(x, y)->GetLayer(m_checkLayer);
                for (auto *collider: *layer) {
                    if (collider == this) continue;
                    // Check if the other collider had already registered a collision with me
                    short collision = grid->GetCollisionCached(collider, this);
                    if (collision == 1) {
                        SendCollision(*collider); // It had already been checked
                    } else if (collision == -1) {
                        if (grid->GetCollisionCached(this, collider) != -1) {
                            continue; // Skip if we found it in a previous cell
                        }
                        bool colliding = IsColliding(collider);
                        grid->NotifyCacheCollision(this, collider, colliding); // Notify my result
                        if (colliding) SendCollision(*collider); // Notify my listener that I am colliding with collider
                    }
                }
            }
        }
    }
    // Update our layer information
    if (m_layer >= 0) grid->Update(this);
}

void CollideComponent::Destroy() {
    if (m_layer >= 0) {
        scene->GetGrid()->Remove(this);
    }
    Component::Destroy();
}

void CollideComponent::OnGameObjectDisabled() {
    if (m_layer >= 0) {
        scene->GetGrid()->Remove(this);
    }
}

void CollideComponent::Disable() {
    if (m_layer >= 0) {
        scene->GetGrid()->Remove(this);
    }
    m_disabled = true;
}

void CollideComponent::Enable() {
    m_disabled = false;
}