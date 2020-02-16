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
    int m_damage;
    AnimationRenderer *m_renderer;
    CollideComponent *m_collider;
    int m_animBullet, m_animKill;
    float m_destroyIn;
protected:
    Vector2D m_direction;
    int m_speed;
public:
    void Create(Level *level, GameObject *go) override {
        Component::Create(level, go);
    }

    virtual void Init(const Vector2D &direction, int speed = BULLET_SPEED, int damage = 1) {
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
            UpdatePosition(dt);
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

    virtual void UpdatePosition(float dt) = 0;

    [[nodiscard]] int GetDamage() const { return m_damage; }
};

class BulletStraightMovement : public BulletBehaviour {
public:
    virtual void UpdatePosition(float dt) {
        go->position = go->position + m_direction * m_speed * dt;
    }
};

class BulletCirclesMovement : public BulletBehaviour {
private:
    Vector2D m_theoricalPos;
    float m_currentAngle;
public:
    void Init(const Vector2D &direction, int speed, int damage) override {
        BulletBehaviour::Init(direction, speed, damage);
        m_theoricalPos = go->position;
        m_currentAngle = 3.1416;
    }

    void UpdatePosition(float dt) override {
        m_theoricalPos = m_theoricalPos + m_direction * m_speed * dt;
        m_currentAngle += 4 * 6.28 * (m_direction.x >= 0 ? 1 : -1) * dt;
        go->position = m_theoricalPos + (m_direction *FIRE_BULLET_MOVEMENT_RADIUS * PIXELS_ZOOM).rotate(m_currentAngle)
                + m_direction * FIRE_BULLET_MOVEMENT_RADIUS * PIXELS_ZOOM;
    }
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
