//
// Created by david on 6/2/20.
//

#ifndef CONTRA_BULLETS_H
#define CONTRA_BULLETS_H

#include <SDL_log.h>
#include "game_object.h"
#include "component.h"
#include "consts.h"
#include "SimpleRenderer.h"

#define BULLET_SPEED 160

class BulletBehaviour : public Component {
public:
    enum BulletType {
        PLAYER_BULLET_DEFAULT,
        PLAYER_BULLET_FLAME,
        PLAYER_BULLET_MACHINE_GUN,
        PLAYER_BULLET_LASER,
        ENEMY_BULLET_DEFAULT
    };
private:
    Vector2D m_direction;
    float *m_cameraX;
    int m_speed;
    int m_damage;
    SimpleRenderer *m_renderer;
    CollideComponent *m_collider;
    float m_destroyIn;
    BulletType m_type;
public:
    void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, float *camera_x) {
        Component::Create(engine, go, game_objects);
        m_cameraX = camera_x;
    }

    void Init(const Vector2D &direction, const BulletType &type, int speed = BULLET_SPEED, int damage = 1) {
        m_direction = direction;
        m_speed = speed;
        m_damage = damage;
        m_type = type;
        m_destroyIn = -1;
        if (!m_renderer) m_renderer = go->GetComponent<SimpleRenderer *>();
        if (!m_collider) m_collider = go->GetComponent<CollideComponent *>();
        RestoreRender();
        m_collider->Enable();
    }

    void Update(float dt) override {
        if (m_destroyIn > 0) {
            m_destroyIn -= dt;
            if (m_destroyIn < 0) {
                game_objects[RENDERING_LAYER_BULLETS].erase(go);
                go->Disable();
            }
        } else {
            go->position = go->position + m_direction * m_speed * dt;
            if ((go->position.x < *m_cameraX or go->position.x > *m_cameraX + WINDOW_WIDTH)
                or (go->position.y < 0 or go->position.y > WINDOW_HEIGHT)) {
                game_objects[RENDERING_LAYER_BULLETS].erase(go);
                go->Disable();
            }
        }
    }

    void Kill() {
        m_collider->Disable();
        switch (m_type) {
            case ENEMY_BULLET_DEFAULT:
                game_objects[RENDERING_LAYER_BULLETS].erase(go);
                go->Disable();
                break;
            default:
                m_renderer->ChangeCoords(104, 0, 7, 7, 3, 3);
                m_destroyIn = 0.1f;
                break;
        }
    }

    [[nodiscard]] int GetDamage() const { return m_damage; }

private:
    void RestoreRender() {
        switch (m_type) {
            case ENEMY_BULLET_DEFAULT:
                m_renderer->ChangeCoords(199, 72, 3, 3, 1, 1);
                break;
            case PLAYER_BULLET_DEFAULT:
            case PLAYER_BULLET_FLAME:
            case PLAYER_BULLET_LASER:
            case PLAYER_BULLET_MACHINE_GUN:
                m_renderer->ChangeCoords(82, 10, 3, 3, 1, 1);
                break;
        }
    }
};

class Bullet : public GameObject {
public:
    void Init(const Vector2D &pos, BulletBehaviour::BulletType type, const Vector2D &direction,
            const int speed = BULLET_SPEED * PIXELS_ZOOM) {
        GameObject::Init();
        position = pos;
        auto *behaviour = GetComponent<BulletBehaviour *>();
        if (!behaviour) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find bullet behaviour.");
        } else {
            behaviour->Init(direction, type, speed);
        }
    }
};

#endif //CONTRA_BULLETS_H
