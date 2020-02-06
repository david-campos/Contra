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
    std::set<GameObject *> *game_objects;    // the global container of game objects
public:
    virtual ~Component() {}

    virtual void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects);

    virtual void Init() {}

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


class CollideComponent : public Component {
protected:
    Grid *grid;
public:
    virtual void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Grid *grid);

    virtual void Update(float dt);
};

class CircleCollideComponent : public CollideComponent {
    double radius;

public:
    virtual void
    Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Grid *grid, double radius);

    virtual void Update(float dt);

    double getRadius() const;
};


class RigidBodyComponent : public Component {
private:
    Grid *grid;
    std::set<GridCell *> occupiedCells;
public:
    Vector2D velocity, acceleration;

    virtual void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Grid *grid);

    void Init() override;

    virtual void Update(float dt);
};
