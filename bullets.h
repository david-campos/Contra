//
// Created by david on 6/2/20.
//

#ifndef CONTRA_BULLETS_H
#define CONTRA_BULLETS_H

#include <SDL_log.h>
#include "game_object.h"
#include "component.h"
#include "consts.h"

#define BULLET_SPEED 320

class BulletBehaviour : public Component {
private:
    Vector2D m_direction;
    float *m_cameraX;
    int m_speed;
public:
    void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, float *camera_x) {
        Component::Create(engine, go, game_objects);
        m_cameraX = camera_x;
    }

    void Init(const Vector2D &direction, int speed = BULLET_SPEED) {
        m_direction = direction;
        m_speed = speed;
    }

    void Update(float dt) override {
        go->position = go->position + m_direction * m_speed * dt;
        if ((go->position.x < *m_cameraX or go->position.x > *m_cameraX + WINDOW_WIDTH)
            or (go->position.y < 0 or go->position.y > WINDOW_HEIGHT)) {
            go->enabled = false;
            game_objects->erase(go);
        }
    }
};

class Bullet : public GameObject {
public:
    void Init(const Vector2D &pos, const Vector2D &direction, const int speed = BULLET_SPEED) {
        GameObject::Init();
        position = pos;
        auto *behaviour = GetComponent<BulletBehaviour *>();
        if (!behaviour) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find bullet behaviour.");
        } else {
            behaviour->Init(direction, speed);
        }
    }
};

#endif //CONTRA_BULLETS_H
