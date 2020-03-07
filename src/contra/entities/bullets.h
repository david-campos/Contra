//
// Created by david on 6/2/20.
//

#ifndef CONTRA_BULLETS_H
#define CONTRA_BULLETS_H

#include <SDL_log.h>
#include "../components/Gravity.h"
#include "../../components/collision/CollideComponent.h"
#include "../hittable.h"
#include "../level/perspective_level.h"

class BulletBehaviour : public Component {
private:
    int m_damage;
    CollideComponent *m_collider;
    int m_animBullet, m_animKill;
    PerspectiveLevel *perspectiveLevel;
protected:
    AnimationRenderer *m_renderer;
    Vector2D m_direction;
    int m_speed;
    bool m_kill;
    float m_minY, m_maxY;
public:
    void Create(Level *level, GameObject *go) {
        Component::Create(level, go);
        // Notice it can be null!
        perspectiveLevel = dynamic_cast<PerspectiveLevel *>(level);
    }

    virtual void Init(const Vector2D &direction, int speed = BULLET_SPEED, int damage = 1,
                      float y_min = -9999, float y_max = 9999) {
        m_direction = direction.normalise();
        m_speed = speed;
        m_damage = damage;
        m_kill = false;
        m_minY = y_min;
        m_maxY = y_max;
        if (!m_renderer) {
            m_renderer = go->GetComponent<AnimationRenderer *>();
            m_animBullet = m_renderer->FindAnimation("Bullet");
            m_animKill = m_renderer->FindAnimation("Kill");
        }
        if (!m_collider) m_collider = go->GetComponent<CollideComponent *>();
        m_renderer->PlayAnimation(m_animBullet);
        m_renderer->GoToFrame(0);
        m_collider->Enable();
    }

    bool IsKilled() const {
        return m_kill;
    }

    void Update(float dt) override {
        if (m_kill) {
            if (!m_renderer->IsPlaying()) {
                go->Disable();
                go->MarkToRemove();
            }
        } else {
            UpdatePosition(dt);
            if ((go->position.x < scene->GetCameraX() or go->position.x > scene->GetCameraX() + WINDOW_WIDTH)
                or (go->position.y < 0 or go->position.y > WINDOW_HEIGHT)) {
                go->Disable();
                go->MarkToRemove();
            }
            // The minY and maxY are used in the "perspective" type of game,
            // in this mode, bullets check collision when they reach their Y limits
            // instead of the objects checking collision with bullets all the time
            // (so they do not get hit by 2D superposition)
            bool hits_min = go->position.y < m_minY;
            if ((hits_min || go->position.y > m_maxY) && !IsKilled()) {
                std::set<CollideComponent *> colliding;
                m_collider->GetCurrentCollisions(&colliding);
                // Kill the first destroyable we found, if HitLast reserve as last option
                Hittable *chosen = nullptr;
                for (auto collider: colliding) {
                    auto *hittable = collider->GetGameObject()->GetComponent<Hittable *>();
                    if (hittable && hittable->CanBeHit()) {
                        chosen = hittable;
                        if (!hittable->HitLast()) {
                            break;
                        }
                    }
                }
                if (chosen) {
                    chosen->Hit();
                }
                // When hitting max we give it some margin in which it still can kill you
                if (hits_min || chosen || go->position.y > m_maxY + 20 * PIXELS_ZOOM) {
                    // If we hit something it shows the explosion animation,
                    // if we don't but we are hitting the min with the laser on we show it too
                    // bc it means we hit the "wall".
                    Kill((hits_min && perspectiveLevel && perspectiveLevel->IsLaserOn()) || chosen);
                }
            }
        }
    }

    void Kill(bool with_animation = true) {
        m_collider->Disable();
        m_kill = true;
        if (with_animation && m_animKill >= 0) {
            m_renderer->PlayAnimation(m_animKill);
        } else {
            m_renderer->Pause();
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
    void Init(const Vector2D &direction, int speed, int damage, float y_min = -9999, float y_max = 9999) override {
        BulletBehaviour::Init(direction, speed, damage, y_min);
        m_theoricalPos = go->position;
        m_currentAngle = 3.1416;
    }

    void UpdatePosition(float dt) override {
        m_theoricalPos = m_theoricalPos + m_direction * m_speed * dt;
        m_currentAngle += 4 * 6.28 * (m_direction.x >= 0 ? 1 : -1) * dt;
        go->position = m_theoricalPos + (m_direction * FIRE_BULLET_MOVEMENT_RADIUS * PIXELS_ZOOM).rotate(m_currentAngle)
                       + m_direction * FIRE_BULLET_MOVEMENT_RADIUS * PIXELS_ZOOM;
    }
};

class LaserBulletBehaviour : public BulletStraightMovement {
public:
    void Init(const Vector2D &direction, int speed, int damage, float y_min = -9999, float y_max = 9999) override {
        BulletBehaviour::Init(direction, speed, damage, y_min, y_max);
        if (abs(direction.x) <= 0.0001) {
            m_renderer->CurrentAndPause(0); // 0 = vertical
        } else if (abs(direction.y) <= 0.0001) {
            m_renderer->CurrentAndPause(3); // 3 = horizontal
        } else {
            m_renderer->CurrentAndPause(2); // 2 = diagonal
            m_renderer->mirrorHorizontal = (direction.x * direction.y > 0);
        }
    }
};

class BlastBulletBehaviour : public BulletBehaviour {
private:
    Gravity *m_gravity;
public:
    void Init(const Vector2D &direction, int speed, int damage, float y_min = -9999, float y_max = 9999) override {
        BulletBehaviour::Init(direction, speed, damage, y_min, y_max);
        if (!m_gravity) {
            m_gravity = go->GetComponent<Gravity *>();
        }
    }

    void UpdatePosition(float dt) override {
        if (m_gravity->IsOnAir()) {
            go->position = go->position + m_direction * m_speed * dt;
        } else {
            Kill();
        }
    }
};

class Bullet : public GameObject {
public:
    void Init(const Vector2D &pos, const Vector2D &direction,
              const int speed = BULLET_SPEED * PIXELS_ZOOM, const float min_y = -9999, const float max_y = 9999) {
        GameObject::Init();
        position = pos;
        auto *behaviour = GetComponent<BulletBehaviour *>();
        if (!behaviour) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find bullet behaviour.");
        } else {
            behaviour->Init(direction, speed, 1, min_y, max_y);
        }
    }
};

#endif //CONTRA_BULLETS_H
