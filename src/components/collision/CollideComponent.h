//
// Created by david on 3/3/20.
//

#ifndef CONTRA_COLLIDECOMPONENT_H
#define CONTRA_COLLIDECOMPONENT_H

#include <set>
#include "../../kernel/component.h"
#include "grid.h"

class CollideComponent;

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

    virtual void Update(float dt) override;

    void GetCurrentCollisions(std::set<CollideComponent *> *out_set);

    void SendCollision(const CollideComponent &other) {
        if (listener) listener->OnCollision(other);
    }
};

#endif //CONTRA_COLLIDECOMPONENT_H
