//
// Created by david on 13/2/20.
//

#ifndef CONTRA_ENEMIES_H
#define CONTRA_ENEMIES_H

#include <random>
#include "component.h"
#include "Player.h"

class Ledder : public GameObject {
public:
    void
    Create(Level* level, float time_hidden, float time_shown, float cooldown_time, bool show_standing,
           int burst_length, float burst_cooldown, bool horizontally_precise);
};

class Greeder : public GameObject {
public:
    void
    Create(Level* level);
};

class GreederSpawner : public Component {
private:
    Greeder *m_greeder;
    float m_randomInterval;
    float m_intervalCount;
public:
    void Create(Level* level, GameObject* go, float random_interval);
    void Update(float dt) override;

    void Destroy() override;
};

class LedderBehaviour : public LevelComponent, public CollideComponentListener {
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
public:
    void
    Create(Level* level, GameObject *go, float time_hidden, float time_shown, float cooldown_time, bool show_standing,
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
            m_burstCoolDown = 0.3; // Do not shoot immediately as it enters the screen, it feels awful
            m_animator->CurrentAndPause(m_animStanding, true);
        }
        m_coolDown = 0;
        m_firedInBurst = m_burstLength; // Wait for cooldown to be zero
    }

    void Fire();

    void Update(float dt) override;

    void OnCollision(const CollideComponent &collider) override;

private:
    void ChangeToState(State state);
};

class GreederBehaviour : public LevelComponent, public CollideComponentListener {
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
public:
    void Create(Level* level, GameObject *go);

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
