//
// Created by david on 13/2/20.
//

#ifndef CONTRA_ENEMIES_H
#define CONTRA_ENEMIES_H

#include "component.h"
#include "AnimationRenderer.h"
#include "Player.h"

class Ledder : public GameObject {
public:
    void
    Create(AvancezLib *engine, std::set<GameObject *> *game_objects, ObjectPool<Bullet> *bullet_pool, Player *player,
           std::shared_ptr<Sprite> enemies_spritesheet, float *camera_x, Grid* grid, int layer,
           float time_hidden, float time_shown, float cooldown_time, bool show_standing,
           int burst_length, float burst_cooldown, bool horizontally_precise);
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
    Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, ObjectPool<Bullet> *bullet_pool,
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


#endif //CONTRA_ENEMIES_H
