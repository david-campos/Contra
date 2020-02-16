//
// Created by david on 6/2/20.
//

#ifndef CONTRA_BULLETS_H
#define CONTRA_BULLETS_H

#include <SDL_log.h>
#include "game_object.h"
#include "AnimationRenderer.h"
#include "component.h"
#include "consts.h"
#include "level.h"

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
    int m_speed;
    int m_damage;
    AnimationRenderer *m_renderer;
    CollideComponent *m_collider;
    int m_animBullet, m_animKill;
    float m_destroyIn;
public:
    void Create(Level* level, GameObject *go) {
        Component::Create(level, go);
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
                go->Disable();
                go->MarkToRemove();
            }
        } else {
            go->position = go->position + m_direction * m_speed * dt;
            if ((go->position.x < level->GetCameraX() or go->position.x > level->GetCameraX() + WINDOW_WIDTH)
                or (go->position.y < 0 or go->position.y > WINDOW_HEIGHT)) {
                go->Disable();
                go->MarkToRemove();
            }
        }
    }

    void Kill() {
        m_collider->Disable();
        if (m_animKill >= 0) {
            m_renderer->PlayAnimation(m_animKill);
            m_destroyIn = 0.1f;
        } else {
            go->Disable();
            go->MarkToRemove();
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
