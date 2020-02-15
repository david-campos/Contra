//
// Created by david on 13/2/20.
//

#ifndef CONTRA_ENEMIES_H
#define CONTRA_ENEMIES_H

#include <random>
#include "component.h"
#include "AnimationRenderer.h"
#include "Player.h"

class Ledder : public GameObject {
public:
    void
    Create(AvancezLib *engine,std::set<GameObject *> **game_objects, ObjectPool<Bullet> *bullet_pool, Player *player,
           std::shared_ptr<Sprite> enemies_spritesheet, float *camera_x, Grid *grid,
           float time_hidden, float time_shown, float cooldown_time, bool show_standing,
           int burst_length, float burst_cooldown, bool horizontally_precise);
};

class Greeder : public GameObject {
public:
    void
    Create(AvancezLib *engine,std::set<GameObject *> **game_objects,
           std::shared_ptr<Sprite> enemies_spritesheet, float *camera_x, Grid *grid,
           const std::weak_ptr<Floor> &the_floor);
};

class GreederSpawner : public Component {
private:
    float* m_cameraX;
    Greeder *m_greeder;
public:
    void Create(AvancezLib *engine, GameObject* go,std::set<GameObject *> **game_objects,
                std::shared_ptr<Sprite> enemies_spritesheet, float *camera_x, Grid *grid,
                const std::weak_ptr<Floor> &the_floor, GameObject* receiver);
    void Update(float dt) override;

    void Destroy() override;
};

class LedderBehaviour : public Component, public CollideComponentListener {
private:
    enum State {
        HIDDEN,
        SHOWING,
        SHOWN,
        HIDING,
        GOING_TO_DIE,
        DYING
    };
    State m_state;
    bool m_showStanding, m_horizontallyPrecise;
    float m_currentStateTime;
    float m_timeHidden, m_timeShown, m_coolDownTime, m_coolDown, m_burstCoolDownTime, m_burstCoolDown;
    int m_firedInBurst, m_burstLength;
    int m_animShow, m_animStanding, m_animShootUp, m_animShootDown, m_animGoingToDie, m_animDying;
    AnimationRenderer *m_animator;
    ObjectPool<Bullet> *m_bulletPool;
    Player *m_player;
public:
    void
    Create(AvancezLib *engine, GameObject *go,std::set<GameObject *> **game_objects, ObjectPool<Bullet> *bullet_pool,
           Player *player, float time_hidden, float time_shown, float cooldown_time, bool show_standing,
           int burst_length, float burst_cooldown, bool horizontally_precise);

    void Init() override {
        Component::Init();
        m_currentStateTime = 0;
        if (!m_animator) {
            m_animator = go->GetComponent<AnimationRenderer *>();
            m_animShow = m_animator->FindAnimation("Showing");
            m_animStanding = m_animator->FindAnimation("Standing");
            m_animShootUp = m_animator->FindAnimation("ShootUp");
            m_animShootDown = m_animator->FindAnimation("ShootDown");
            m_animGoingToDie = m_animator->FindAnimation("GoingToDie");
            m_animDying = m_animator->FindAnimation("Dying");
        }
        if (m_timeHidden > 0) {
            m_state = HIDDEN;
            m_burstCoolDown = 0;
            m_animator->CurrentAndPause(m_animShow, true);
        } else {
            m_state = SHOWN;
            m_burstCoolDown = m_burstCoolDownTime;
            m_animator->CurrentAndPause(m_animStanding, true);
        }
        m_coolDown = 0;
        m_firedInBurst = 0;
    }

    void Fire();

    void Update(float dt) override;

    void OnCollision(const CollideComponent &collider) override;

private:
    void ChangeToState(State state);
};

class GreederBehaviour : public Component, public CollideComponentListener {
private:
    int m_animRunning, m_animJumping, m_animDying, m_animDrowning;
    AnimationRenderer *m_animator;
    bool m_isDeath;
    float m_deathFor;
    short m_direction;
    Gravity *m_gravity;
    std::random_device rd;
    std::mt19937 m_mt = std::mt19937(rd());
    std::uniform_real_distribution<float> m_random_dist = std::uniform_real_distribution<float>(0.f, 1.f);
    std::weak_ptr<Floor> m_floor;
public:
    void Create(AvancezLib *engine, GameObject *go,std::set<GameObject *> **game_objects,
                std::weak_ptr<Floor> the_floor);

    void OnGameObjectDisabled() override {
        Component::OnGameObjectDisabled();
    }

    void Destroy() override {
        Component::Destroy();
    }

    void Init() override {
        Component::Init();
        if (!m_animator) {
            m_animator = go->GetComponent<AnimationRenderer *>();
            m_animRunning = m_animator->FindAnimation("Running");
            m_animJumping = m_animator->FindAnimation("Jumping");
            m_animDrowning = m_animator->FindAnimation("Drowning");
            m_animDying = m_animator->FindAnimation("Dying");
        }
        if (!m_gravity) {
            m_gravity = go->GetComponent<Gravity *>();
        }
        m_isDeath = false;
        m_animator->PlayAnimation(m_animRunning);
        m_gravity->SetVelocity(0);
        m_gravity->SetAcceleration(350 * PIXELS_ZOOM);
        m_direction = -1;
    }

    void Update(float dt) override;

    bool IsAlive() const {
        return !m_isDeath;
    }

    void OnCollision(const CollideComponent &collider) override;
};

#endif //CONTRA_ENEMIES_H
