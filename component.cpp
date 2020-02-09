#include "component.h"
#include "game_object.h"
#include "avancezlib.h"
#include "grid_cell.h"

void Component::Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects) {
    this->go = go;
    this->engine = engine;
    this->game_objects = game_objects;
}

void RenderComponent::Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects,
                             const char *sprite_name, float *camera_x) {
    Component::Create(engine, go, game_objects);
    sprite.reset(engine->createSprite(sprite_name));
    this->camera_x = camera_x;
}

void RenderComponent::Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects,
                             std::shared_ptr<Sprite> sprite, float *camera_x) {
    Component::Create(engine, go, game_objects);
    this->sprite = std::move(sprite);
    this->camera_x = camera_x;
}

void RenderComponent::Destroy() {
    sprite.reset();
}


void CollideComponent::Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Grid *grid,
                              int layer, int checkLayer) {
    Component::Create(engine, go, game_objects);
    this->grid = grid;
    this->m_layer = layer;
    this->m_checkLayer = checkLayer;
}

void CollideComponent::Update(float dt) {
    if (m_disabled) return;
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
                        bool colliding = IsColliding(*collider);
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
    if (m_layer >= 0) grid->Remove(this);
    Component::Destroy();
}

void CollideComponent::OnGameObjectDisabled() {
    if (m_layer >= 0) grid->Remove(this);
}

void CollideComponent::Disable() {
    if (m_layer >= 0) grid->Remove(this);
    m_disabled = true;
}

void CollideComponent::Enable() {
    m_disabled = false;
}

void BoxCollider::GetOccupiedCells(Grid::CellsSquare &square) {
    int cell_size = grid->getCellSize();
    int row_size = grid->getRowSize();
    int col_size = grid->getColSize();
    square.min_cell_x = std::min(std::max((int) floor((go->position.x + m_box.top_left_x) / cell_size), 0), row_size - 1);
    square.max_cell_x = std::min(std::max((int) floor((go->position.x + m_box.bottom_right_x) / cell_size), 0), row_size - 1);
    square.min_cell_y = std::min(std::max((int) floor((go->position.y + m_box.top_left_y) / cell_size), 0), col_size - 1);
    square.max_cell_y = std::min(std::max((int) floor((go->position.y + m_box.bottom_right_y) / cell_size), 0), col_size - 1);
}

bool BoxCollider::IsColliding(const CollideComponent &other) {
    auto *other_box = dynamic_cast< const BoxCollider * >( &other );
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
