//
// Created by david on 15/2/20.
//

#ifndef CONTRA_PICKUPS_H
#define CONTRA_PICKUPS_H

#include <utility>

#include "component.h"
#include "SimpleRenderer.h"
#include "bullets.h"
#include "Gravity.h"
#include "Player.h"
#include "consts.h"
#include "pickup_types.h"

class PickUpBehaviour : public Component {
private:
    PickUpType m_type;
    Gravity *m_gravity;
public:
    void Create(Level *level, GameObject *go, PickUpType type) {
        Component::Create(level, go);
        m_type = type;
    }

    void Init() override {
        Component::Init();
        if (!m_gravity) {
            m_gravity = go->GetComponent<Gravity *>();
        }
    }

    [[nodiscard]] PickUpType GetType() const {
        return m_type;
    }

    void Update(float dt) override {
        if (!m_gravity->IsOnFloor()) {
            go->position = go->position + Vector2D(PICKUP_SPEED * PIXELS_ZOOM, 0) * dt;
        }
    }
};

class PickUp : public GameObject {
public:
    void Create(Level *level, std::shared_ptr<Sprite> pickups_spritesheet,
                Grid *grid, std::weak_ptr<Floor> level_floor, PickUpType type) {
        GameObject::Create();
        auto *behaviour = new PickUpBehaviour();
        behaviour->Create(level, this, type);
        auto *gravity = new Gravity();
        gravity->Create(level, this);
        gravity->SetFallThoughWater(true);
        auto *renderer = new SimpleRenderer();
        renderer->Create(level, this, std::move(pickups_spritesheet),
                25 * (int) type, 0, 24, 15, 12, 14);
        gravity->SetVelocity(-PLAYER_JUMP * PIXELS_ZOOM);
        auto *collider = new BoxCollider();
        collider->Create(level, this,
                -4 * PIXELS_ZOOM, -10 * PIXELS_ZOOM,
                8 * PIXELS_ZOOM, 11 * PIXELS_ZOOM,
                PLAYER_COLLISION_LAYER, -1);
        AddComponent(gravity);
        AddComponent(behaviour);
        AddComponent(renderer);
        AddComponent(collider);
    }
};

class PickUpHolderBehaviour : public LevelComponent, public CollideComponentListener {
protected:
    AnimationRenderer *m_animator;
    int m_animDying;
    bool m_canBeHit;
    PickUp *m_powerUp;
    short m_lives;
public:
    void Create(Level* level, GameObject *go, PickUp *power_up) {
        LevelComponent::Create(level, go);
        m_powerUp = power_up;
    }

    void Update(float dt) override {
        if (m_lives <= 0) {
            if (!m_animator->IsPlaying()) {
                go->Disable();
            }
            return;
        }
    }

    void Init() override {
        if (!m_animator) {
            m_animator = go->GetComponent<AnimationRenderer *>();
            m_animDying = m_animator->FindAnimation("Dying");
        }
    }

    void OnCollision(const CollideComponent &collider) override {
        if (m_canBeHit && m_lives > 0) {
            auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
            if (bullet && !bullet->IsKilled()) {
                m_lives--;
                bullet->Kill();
                if (m_lives == 0) {
                    m_animator->PlayAnimation(m_animDying);
                    m_powerUp->position = go->position;
                    go->Send(SCORE1_500);
                    m_powerUp->Init();
                    level->AddGameObject(m_powerUp, RENDERING_LAYER_ENEMIES);
                }
            }
        }
    }
};

class CoveredPickUpHolderBehaviour : public PickUpHolderBehaviour {
private:
    float m_waitTime;
    bool m_isOpen;
    bool m_isTransition;
public:
    void Init() override {
        PickUpHolderBehaviour::Init();
        if (!m_animator) {
            m_animator = go->GetComponent<AnimationRenderer *>();
        }
        m_animator->PlayAnimation(0);
        m_isOpen = false;
        m_isTransition = false;
        m_waitTime = 1.5;
        m_lives = 2;
    }

    void Update(float dt) override {
        PickUpHolderBehaviour::Update(dt);
        if (m_lives <= 0) return;
        if (m_isTransition && m_animator->IsPlaying())
            return;
        m_isTransition = false;
        if (m_isOpen) {
            if (!m_animator->IsCurrent(2))
                m_animator->PlayAnimation(2);
        } else m_animator->PlayAnimation(0);

        m_waitTime -= dt;
        if (m_waitTime < 0) {
            m_waitTime = 1.5f;
            m_animator->PlayAnimation(1);
            m_isTransition = true;
            m_isOpen = !m_isOpen;
        }
        m_canBeHit = m_isOpen && !m_isTransition;
    }
};

class FlyingPickupHolderBehaviour : public PickUpHolderBehaviour {
private:
    Vector2D m_initialPosition;
    CollideComponent* m_collider;
    float m_time;
public:
    void Init() override {
        PickUpHolderBehaviour::Init();
        m_initialPosition = go->position;
        m_lives = 1;
        m_canBeHit = true;
        m_time = 0;
        m_collider = go->GetComponent<CollideComponent*>();
    }

    void Update(float dt) override {
        PickUpHolderBehaviour::Update(dt);
        if (m_lives <= 0) return;
        if (m_initialPosition.x > level->GetCameraX() - m_animator->GetCurrentAnimation().frame_w) {
            m_animator->enabled = false;
            if(!m_collider->IsDisabled()) m_collider->Disable();
        } else {
            m_animator->enabled = true;
            if(m_collider->IsDisabled()) m_collider->Enable();
            m_time += dt;
            go->position = m_initialPosition + Vector2D(
                    1.7 * PLAYER_SPEED * m_time * PIXELS_ZOOM, sinf(6 * m_time) * 20 * PIXELS_ZOOM);
            if (go->position.x > level->GetCameraX() + WINDOW_WIDTH + RENDERING_MARGINS) {
                go->MarkToRemove();
            }
        }
    }
};

#endif //CONTRA_PICKUPS_H
