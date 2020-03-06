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
    if (m_disabled || !go->IsEnabled()) return;
    std::set<CollideComponent *> colliders;
    GetCurrentCollisions(&colliders);
    for (auto collider: colliders) {
        SendCollision(*collider);
    }
    // Update our layer information
    if (m_layer >= 0) scene->GetGrid()->Update(this);
}

void CollideComponent::GetCurrentCollisions(std::set<CollideComponent *> *out_set) {
    auto *grid = scene->GetGrid();
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
                        bool colliding;
                        int reversed = grid->GetCollisionCached(this, collider);
                        if (reversed != -1) {
                            colliding = reversed == 1;
                        } else {
                            colliding = IsColliding(collider);
                            grid->NotifyCacheCollision(this, collider, colliding); // Notify my result
                        }
                        if (colliding) out_set->insert(collider);
                    }
                }
            }
        }
    }
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
    if (!m_disabled) {
        if (m_layer >= 0) {
            scene->GetGrid()->Remove(this);
        }
        m_disabled = true;
    }
}

void CollideComponent::Enable() {
    if (m_disabled) m_disabled = false;
}
