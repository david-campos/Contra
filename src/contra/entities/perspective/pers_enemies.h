//
// Created by david on 6/3/20.
//

#ifndef CONTRA_PERS_ENEMIES_H
#define CONTRA_PERS_ENEMIES_H

#include "../../level/level_component.h"
#include "../../hittable.h"
#include "../../level/perspective_level.h"
#include "../../components/Gravity.h"
#include "../pickups.h"

class PerspectiveLedderBehaviour : public LevelComponent, public Hittable, public Killable {
private:
    PerspectiveLevel *m_perspectiveLevel;
    AnimationRenderer *m_animator;
    Gravity *m_gravity;
    int m_duckAnim, m_jumpAnim, m_runAnim, m_standAnim, m_dyingAnim;
    bool m_goesJumping;
    float m_speed;
    float m_stopToShootChance;
    float m_timeOnFloor;
    float m_nextShoot;
    float m_timeStanding;
    float m_deadFor;
    PickUp *m_droppedPickup;
    enum MovementState {
        MOVING,
        STANDING,
        DEAD
    };
    MovementState m_state;

    std::random_device rd;
    std::mt19937 m_mt = std::mt19937(rd());
    std::uniform_real_distribution<float> m_random_dist = std::uniform_real_distribution<float>(0.f, 1.f);
public:
    void
    Create(PerspectiveLevel *level, GameObject *go, bool jumps, bool stopToShootChance, float speed,
           PickUp *droppedPickup) {
        LevelComponent::Create(level, go);
        m_perspectiveLevel = level;
        m_goesJumping = jumps;
        m_stopToShootChance = stopToShootChance;
        m_speed = speed;
        m_droppedPickup = droppedPickup;
    }

    void Init() override {
        LevelComponent::Init();
        if (!m_animator) {
            m_animator = GetComponent<AnimationRenderer *>();
            m_duckAnim = m_animator->FindAnimation("Duck");
            m_runAnim = m_animator->FindAnimation("Run");
            m_jumpAnim = m_animator->FindAnimation("Jump");
            m_standAnim = m_animator->FindAnimation("Stand");
            m_dyingAnim = m_animator->FindAnimation("Dying");
        }
        if (!m_gravity) {
            m_gravity = GetComponent<Gravity *>();
        }
        m_state = MOVING;
        m_nextShoot = m_random_dist(m_mt);
        m_deadFor = 0;
        m_animator->mirrorHorizontal = m_speed > 0;
    }

    void Update(float dt) override {
        switch (m_state) {
            case MOVING:
                if (m_goesJumping) {
                    m_animator->PlayAnimation(m_gravity->IsOnFloor() ? m_duckAnim : m_jumpAnim);
                    if (m_gravity->IsOnFloor()) {
                        if (m_timeOnFloor == 0.f) {
                            Fire();
                        }
                        m_timeOnFloor += dt;
                        if (m_timeOnFloor > 0.3f) {
                            m_gravity->AddVelocity(-PLAYER_JUMP * 0.7f * PIXELS_ZOOM);
                        }
                    } else {
                        m_timeOnFloor = 0.f;
                        go->position.x += m_speed * dt;
                    }
                } else {
                    m_animator->PlayAnimation(m_gravity->IsOnFloor() ? m_runAnim : m_jumpAnim);
                    go->position.x += m_speed * dt;
                }
                break;
            case STANDING:
                m_animator->PlayAnimation(m_standAnim);
                m_timeStanding += dt;
                if (m_timeStanding > 0.3f) {
                    m_state = MOVING;
                }
                break;
            case DEAD:
                if (m_deadFor < 0.25f) {
                    m_animator->PlayAnimation(m_standAnim);
                    m_gravity->SetAcceleration(0);
                    m_gravity->SetVelocity(0);
                    go->position = go->position + Vector2D(0, -PLAYER_SPEED * dt);
                    m_deadFor += dt;
                    if (m_deadFor >= 0.25f) {
                        Kill();
                    }
                } else {
                    if (!m_animator->IsPlaying()) {
                        go->Disable();
                        go->MarkToRemove();
                    }
                }
        }
        if (!m_goesJumping) {
            m_nextShoot -= dt;
            if (m_nextShoot < 0.f) {
                Fire();
                m_nextShoot = m_random_dist(m_mt) * 0.5f + 0.2f;
                if (m_state != STANDING && m_random_dist(m_mt) < m_stopToShootChance) {
                    m_timeStanding = 0;
                    m_state = STANDING;
                }
            }
        }
    }

    void Fire() {
        auto *bullet = level->GetEnemyBullets()->FirstAvailable();
        Vector2D fire_pos = go->position;
        Vector2D target = m_perspectiveLevel->ProjectFromBackToFront(fire_pos);
        if (bullet) {
            bullet->Init(fire_pos, target - fire_pos, 0.5 * BULLET_SPEED * PIXELS_ZOOM,
                    -9999, (PERSP_PLAYER_Y - 10) * PIXELS_ZOOM);
            level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
        }
    }

    void Kill() override {
        m_state = DEAD;
        m_animator->PlayAnimation(m_dyingAnim);
        level->GetSound(SOUND_ENEMY_DEATH)->Play(1);
    }

    void Hit() override {
        m_state = DEAD;
        if (m_droppedPickup) {
            m_droppedPickup->position = go->position;
            m_droppedPickup->Init(Vector2D(0.f, 0.f));
            level->AddGameObject(m_droppedPickup, RENDERING_LAYER_ENEMIES);
        }
    }

    bool CanBeHit() override {
        return m_state != DEAD;
    }
};

class PerspectiveLedder : public GameObject {
public:
    void Create(PerspectiveLevel *level, bool jumps, float stopToShootChance, float speed,
                PickUp *dropped = nullptr) {
        GameObject::Create();
        auto *behaviour = new PerspectiveLedderBehaviour();
        behaviour->Create(level, this, jumps, stopToShootChance, speed, dropped);
        auto *renderer = new AnimationRenderer();
        auto shift = dropped ? 110 : 0;
        renderer->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        renderer->AddAnimation({
                2 + shift, 423, 0.1, 1,
                11, 19, 5, 18,
                "Duck", AnimationRenderer::STOP_AND_FIRST
        });
        renderer->AddAnimation({
                15 + shift, 428, 0.1, 1,
                24, 14, 12, 13,
                "Jump", AnimationRenderer::STOP_AND_FIRST
        });
        renderer->AddAnimation({
                40 + shift, 417, 0.1, 3,
                18, 25, 9, 24,
                "Run", AnimationRenderer::DONT_STOP
        });
        renderer->AddAnimation({
                95 + shift, 418, 0.1, 1,
                15, 24, 7, 23,
                "Stand", AnimationRenderer::DONT_STOP
        });
        renderer->AddAnimation({
                186, 610, 0.15, 3,
                34, 34, 17, 26,
                "Dying", AnimationRenderer::STOP_AND_LAST
        });
        auto *gravity = new Gravity();
        gravity->Create(level, this);
        gravity->SetBaseFloor(PERSP_ENEMIES_Y * PIXELS_ZOOM);
        auto *collider = new BoxCollider();
        collider->Create(level, this,
                -7 * PIXELS_ZOOM, (jumps ? -15 : -23) * PIXELS_ZOOM,
                14 * PIXELS_ZOOM, (jumps ? 15 : 23) * PIXELS_ZOOM,
                NPCS_COLLISION_LAYER, -1);

        AddComponent(behaviour);
        AddComponent(gravity);
        AddComponent(collider);
        AddComponent(renderer);
    }

    void Init(const Vector2D &pos) {
        position = pos;
        GameObject::Init();
    }
};

#endif //CONTRA_PERS_ENEMIES_H
