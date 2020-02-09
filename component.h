#pragma once

#include <set>
#include <memory>
#include "object_pool.h"
#include "game_object.h"
#include "grid_cell.h"

class GameObject;

class AvancezLib;

class Sprite;


class Component {
protected:
    AvancezLib *engine;    // used to access the engine
    GameObject *go;        // the game object this component is part of
    std::set<GameObject *> *game_objects;    // the global container of game objects (pointer to the first layer)
public:
    virtual ~Component() {}

    virtual void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects);

    virtual void Init() {}

    virtual void OnGameObjectEnabled() {}

    virtual void OnGameObjectDisabled() {}

    [[nodiscard]] GameObject *GetGameObject() const { return go; }

    virtual void Update(float dt) = 0;

    virtual void Receive(int message) {}

    virtual void Destroy() {}
};


class RenderComponent : public Component {
protected:
    std::shared_ptr<Sprite> sprite;
    float *camera_x;
public:
    virtual void
    Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, const char *sprite_name,
           float *camera_x);

    virtual void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects,
                        std::shared_ptr<Sprite> sprite, float *camera_x);

    void Destroy() override;

    std::weak_ptr<Sprite> GetSprite() { return sprite; }
};

class CollideComponentListener {
public:
    virtual void OnCollision(const CollideComponent &collider) = 0;
};

class CollideComponent : public Component {
protected:
    Grid *grid;
    Grid::CellsSquare is_occupying;
    int m_layer, m_checkLayer;
    CollideComponentListener *listener = nullptr;
    bool m_disabled;
public:
    /**
     * Creates the collide component
     * @param engine
     * @param go
     * @param game_objects
     * @param grid
     * @param layer Indicates the layer to place the collider in, -1 if you don't want it to be placed anywhere
     * @param checkLayer Indicates the layer for the collider to check collisions with, -1 to avoid checking collisions with any
     */
    void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Grid *grid,
                int layer, int checkLayer);

    /**
     * Changes the listener for the collisions of the collider, be aware if there was a previous
     * one assigned that one will be discarded
     * @param collideComponentListener
     */
    void SetListener(CollideComponentListener *collideComponentListener) { this->listener = collideComponentListener; }

    void Destroy() override;

    void OnGameObjectDisabled() override;
    void Disable();
    void Enable();

    [[nodiscard]] int GetLayer() const { return m_layer; }

    void GetPreviouslyOccupiedCells(Grid::CellsSquare &square) {
        square = is_occupying;
        GetOccupiedCells(is_occupying); // Update for next time
    }

    /**
     * Colliders must implement this function to get the cells the collider occupies
     * @param square
     */
    virtual void GetOccupiedCells(Grid::CellsSquare &square) = 0;

    /**
     * Colliders must implement this function to check if they are colliding with
     * the given collider.
     * @param other
     */
    virtual bool IsColliding(const CollideComponent &other) = 0;

    void Update(float dt) override;

    void SendCollision(const CollideComponent &other) {
        if (listener) listener->OnCollision(other);
    }
};

struct Box {
    int top_left_x;
    int top_left_y;
    int bottom_right_x;
    int bottom_right_y;
};

class BoxCollider : public CollideComponent {
protected:
    Box m_box;

    void GetOccupiedCells(Grid::CellsSquare &square) override;

    bool IsColliding(const CollideComponent &other) override;

    float *m_camera_x;
public:
    virtual void
    Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Grid *grid, float *camera_x,
           int local_top_left_x, int local_top_left_y, int width, int height, int layer, int checkLayer) {
        Create(engine, go, game_objects, grid, camera_x, {
                local_top_left_x,
                local_top_left_y,
                local_top_left_x + width,
                local_top_left_y + height
        }, layer, checkLayer);
    }

    virtual void
    Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Grid *grid, float *camera_x,
           Box box, int layer, int checkLayer) {
        CollideComponent::Create(engine, go, game_objects, grid, layer, checkLayer);
        m_box = box;
        m_camera_x = camera_x;
    }

    void Update(float dt) override {
        CollideComponent::Update(dt);
        engine->strokeSquare(AbsoluteTopLeftX() - *m_camera_x,
                AbsoluteTopLeftY(), AbsoluteBottomRightX() - *m_camera_x, AbsoluteBottomRightY(),
                {0, 0, 255});
    }

    void ChangeBox(const Box& box) {
        m_box = box;
    }

    [[nodiscard]] float AbsoluteTopLeftX() const {
        return float(go->position.x) + float(m_box.top_left_x);
    }

    [[nodiscard]] float AbsoluteTopLeftY() const {
        return float(go->position.y) + float(m_box.top_left_y);
    }

    [[nodiscard]] float AbsoluteBottomRightX() const {
        return float(go->position.x) + float(m_box.bottom_right_x);
    }

    [[nodiscard]] float AbsoluteBottomRightY() const {
        return float(go->position.y) + float(m_box.bottom_right_y);
    }
};

