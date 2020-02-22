#pragma once

#include <set>
#include <memory>
#include "object_pool.h"
#include "game_object.h"
#include "grid.h"
#include "scene.h"

class GameObject;

class Level;

class AvancezLib;

class Sprite;


class Component {
protected:
    BaseScene *scene; // the scene reference
    GameObject *go;        // the game object this component is part of
public:
    virtual ~Component() {}

    virtual void Create(BaseScene *scene, GameObject *go);

    virtual void Init() {
        if (!go) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "Component::Init: No GameObject assigned, probably a missing Component::Create call?");
        }
    }

    virtual void OnGameObjectEnabled() {}

    virtual void OnGameObjectDisabled() {}

    [[nodiscard]] GameObject *GetGameObject() const { return go; }

    virtual void Update(float dt) = 0;

    virtual void Receive(int message) {}

    virtual void Destroy() {}
};

class LevelComponent : public Component {
protected:
    Level *level;
public:
    virtual void Create(Level* level, GameObject *go);
};


class RenderComponent : public Component {
protected:
    std::shared_ptr<Sprite> sprite;
public:
    virtual void Create(BaseScene *scene, GameObject *go, std::shared_ptr<Sprite> sprite);

    void Destroy() override;

    std::shared_ptr<Sprite> GetSprite() { return sprite; }
};

class CollideComponentListener {
public:
    virtual void OnCollision(const CollideComponent &collider) = 0;
};

class CollideComponent : public Component {
protected:
    Grid::CellsSquare is_occupying;
    int m_layer, m_checkLayer;
    CollideComponentListener *listener = nullptr;
    bool m_disabled;
public:
    /**
     * Creates the collide component
     * @param layer Indicates the layer to place the collider in, -1 if you don't want it to be placed anywhere
     * @param checkLayer Indicates the layer for the collider to check collisions with, -1 to avoid checking collisions with any
     */
    void Create(BaseScene *scene, GameObject *go, int layer, int checkLayer);

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

    bool IsDisabled() const {
        return m_disabled;
    }

    [[nodiscard]] int GetLayer() const { return m_layer; }

    void GetPreviouslyOccupiedCells(Grid::CellsSquare &square, const bool update = true) {
        square = is_occupying;
        if (update) {
            GetOccupiedCells(is_occupying);
        }
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
    virtual bool IsColliding(const CollideComponent *other) = 0;

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

    Box operator*(const int &rhs) const {
        return Box{
                top_left_x * rhs,
                top_left_y * rhs,
                bottom_right_x * rhs,
                bottom_right_y * rhs
        };
    }
};

class BoxCollider : public CollideComponent {
protected:
    Box m_box;

    void GetOccupiedCells(Grid::CellsSquare &square) override;

    bool IsColliding(const CollideComponent *other) override;

public:
    virtual void
    Create(BaseScene *scene, GameObject *go,
           int local_top_left_x, int local_top_left_y, int width, int height, int layer, int checkLayer) {
        Create(scene, go, {
                local_top_left_x,
                local_top_left_y,
                local_top_left_x + width,
                local_top_left_y + height
        }, layer, checkLayer);
    }

    virtual void
    Create(BaseScene *scene, GameObject *go, Box box, int layer, int checkLayer) {
        CollideComponent::Create(scene, go, layer, checkLayer);
        m_box = box;
    }

    void ChangeBox(const Box &box) {
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

