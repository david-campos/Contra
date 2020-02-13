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
           float time_hidden, float time_shown, float cooldown_time);
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
    float m_currentStateTime;
    float m_timeHidden, m_timeShown, m_coolDownTime, m_coolDown;
    AnimationRenderer *m_animator;
    ObjectPool<Bullet> *m_bulletPool;
    BoxCollider *m_playerCollider;
public:
    void
    Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, ObjectPool<Bullet> *bullet_pool,
           Player *player, float time_hidden, float time_shown, float cooldown_time);

    void Init() override {
        Component::Init();
        m_coolDown = 0;
        m_state = HIDDEN;
        m_currentStateTime = 0;
        if (!m_animator) {
            m_animator = go->GetComponent<AnimationRenderer *>();
        }
        m_animator->PlayAnimation(0);
        m_animator->GoToFrame(0);
        m_animator->Pause();
    }

    void Fire();

    void Update(float dt) override;

    void OnCollision(const CollideComponent &collider) override;

private:
    void ChangeToState(State state);
};


#endif //CONTRA_ENEMIES_H
