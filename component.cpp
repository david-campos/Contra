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
                             const char *sprite_name, float* camera_x) {
    Component::Create(engine, go, game_objects);
    sprite.reset(engine->createSprite(sprite_name));
    this->camera_x = camera_x;
}

void RenderComponent::Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects,
                             std::shared_ptr<Sprite> sprite, float* camera_x) {
    Component::Create(engine, go, game_objects);
    this->sprite = std::move(sprite);
    this->camera_x = camera_x;
}

void RenderComponent::Update(float dt) {
    if (!go->enabled)
        return;

    if (sprite)
        sprite->draw(int(round(go->position.x - *camera_x)), int(round(go->position.y)));
}

void RenderComponent::Destroy() {
    sprite.reset();
}


void CollideComponent::Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Grid *grid) {
    Component::Create(engine, go, game_objects);
    this->grid = grid;
}


void CollideComponent::Update(float dt) {
//    for (auto i = 0; i < coll_objects->pool.size(); i++) {
//        GameObject *go0 = coll_objects->pool[i];
//        if (go0->enabled) {
//            if ((go0->position.x > go->position.x - 10) &&
//                (go0->position.x < go->position.x + 10) &&
//                (go0->position.y > go->position.y - 10) &&
//                (go0->position.y < go->position.y + 10)) {
//                go->Receive(HIT);
//                go0->Receive(HIT);
//            }
//        }
//    }
}

void CircleCollideComponent::Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects,
                                    Grid *grid, double radius) {
    CollideComponent::Create(engine, go, game_objects, grid);
    this->radius = radius;
}


void CircleCollideComponent::Update(float dt) {

    //If we are using a uniform grid, instead query the grid. The grid should then return a number of potential
    // ball objects that we then check for collisions with.
    Grid::CellsSquare square{};
    grid->GetOccupiedCells(square, go->position);

    for (int y = square.min_cell_y; y <= square.max_cell_y; y++) {
        for (int x = square.min_cell_x; x <= square.max_cell_x; x++) {
            auto coll_objects = &grid->GetCell(x, y)->game_objects;
            for (auto i = 0; i < coll_objects->size(); i++) {
                GameObject *go0 = (*coll_objects)[i];

                if (go0 != go && go0->enabled) {
                    CircleCollideComponent *otherCollider = go0->GetComponent<CircleCollideComponent *>();
                    //if the other GameObject doesn't have a CircleColliderComponent we shouldn't go in here...
                    if (otherCollider != nullptr) {
                        // We don't need to shift the centers as shifting both will cancel when substracting
                        // one from the other (although the centers should be shifted 16 pixels in each axis
                        // to obtain the real centers).
                        auto goTogo0 = go0->position - go->position;
                        float distanceBetweenCircleCenters = goTogo0.magnitude();
                        bool intersection = distanceBetweenCircleCenters <= radius + otherCollider->radius;

                        if (intersection) {
                            RigidBodyComponent *rigidBodyComponent = go->GetComponent<RigidBodyComponent *>();
                            RigidBodyComponent *rigidBodyComponent0 = go0->GetComponent<RigidBodyComponent *>();

                            Vector2D goTogo0Normalized = goTogo0 / distanceBetweenCircleCenters;

                            double dotProduct = rigidBodyComponent->velocity.dotProduct(goTogo0Normalized);
                            double dotProduct0 = rigidBodyComponent0->velocity.dotProduct(goTogo0Normalized);

                            rigidBodyComponent->velocity = goTogo0Normalized * dotProduct0;
                            rigidBodyComponent0->velocity =
                                    rigidBodyComponent0->velocity - goTogo0Normalized * dotProduct;
                        }
                    }
                }
            }
        }
    }
}

double CircleCollideComponent::getRadius() const {
    return radius;
}


void RigidBodyComponent::Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Grid *grid) {
    Component::Create(engine, go, game_objects);
    this->grid = grid;
}

void RigidBodyComponent::Update(float dt) {
    //Forward Euler method:
    go->position = go->position + velocity * dt;
    grid->Update(go, occupiedCells);
}

void RigidBodyComponent::Init() {
    Component::Init();
    SDL_Log("RigidBody::Init");
}
