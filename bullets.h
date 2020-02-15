//
// Created by david on 6/2/20.
//

#ifndef CONTRA_BULLETS_H
#define CONTRA_BULLETS_H

#include <SDL_log.h>
#include "game_object.h"
#include "component.h"
#include "AnimationRenderer.h"
#include "consts.h"

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
    AnimationRenderer *m_renderer;
    CollideComponent *m_collider;
    int m_animBullet, m_animKill;
    float m_destroyIn;
public:
    void Create(AvancezLib *engine, GameObject *go,std::set<GameObject *> **game_objects, float *camera_x) {
        Component::Create(engine, go, game_objects);
        m_cameraX = camera_x;
    }

    void Init(const Vector2D &direction, int speed = BULLET_SPEED, int damage = 1) {
        m_direction = direction.normalise();
        m_speed = speed;
        m_damage = damage;
        m_destroyIn = -1;
        if (!m_renderer) {
            m_renderer = go->GetComponent<AnimationRenderer *>();
            m_animBullet = m_renderer->FindAnimation("Bullet");
            m_animKill = m_renderer->FindAnimation("Kill");
        }
        if (!m_collider) m_collider = go->GetComponent<CollideComponent *>();
        m_renderer->PlayAnimation(m_animBullet);
        m_collider->Enable();
    }

    void Update(float dt) override {
        if (m_destroyIn > 0) {
            m_destroyIn -= dt;
            if (m_destroyIn < 0) {
                game_objects[RENDERING_LAYER_BULLETS]->erase(go);
                go->Disable();
            }
        } else {
            go->position = go->position + m_direction * m_speed * dt;
            if ((go->position.x < *m_cameraX or go->position.x > *m_cameraX + WINDOW_WIDTH)
                or (go->position.y < 0 or go->position.y > WINDOW_HEIGHT)) {
                game_objects[RENDERING_LAYER_BULLETS]->erase(go);
                go->Disable();
            }
        }
    }

    void Kill() {
        m_collider->Disable();
        if (m_animKill) {
            m_renderer->PlayAnimation(m_animKill);
            m_destroyIn = 0.1f;
        } else {
            game_objects[RENDERING_LAYER_BULLETS]->erase(go);
            go->Disable();
        }
    }

    [[nodiscard]] int GetDamage() const { return m_damage; }
};

class Bullet : public GameObject {
public:
    void Init(const Vector2D &pos, const Vector2D &direction,
            const int speed = BULLET_SPEED * PIXELS_ZOOM) {
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
