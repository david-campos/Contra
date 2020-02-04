//
// Created by david on 4/2/20.
//

#include "Player.h"

void PlayerControl::Update(float dt) {
    m_shootDowntime -= dt;

    AvancezLib::KeyStatus keyStatus{};
    engine->getKeyStatus(keyStatus);

    if (keyStatus.right) {
        go->position = go->position + Vector2D(PLAYER_SPEED * dt, 0);
        m_animator->PlayAnimation(m_runAnim);
        m_facingRight = true;
    }
    if (keyStatus.left) {
        go->position = go->position - Vector2D(PLAYER_SPEED * dt, 0);
        m_facingRight = false;
        m_animator->PlayAnimation(m_runAnim);
    }

    if (!keyStatus.right && !keyStatus.left) {
        if (keyStatus.up && !keyStatus.down) {
            m_animator->CurrentAndPause(m_upAnim);
        } else if (keyStatus.down && !keyStatus.up) {
            m_animator->CurrentAndPause(m_crawlAnim);
        } else {
            m_animator->CurrentAndPause(m_idleAnim);
        }
    }


    if (keyStatus.fire && m_shootDowntime <= 0) {
        m_animator->Play(); // The idle ones have a firing animation when playing them
        m_shootDowntime = 0.3;
    }
    m_animator->mirrorHorizontal = !m_facingRight;
}

void PlayerControl::Init() {
    m_animator = go->GetComponent<AnimationRenderer*>();
    m_idleAnim = m_animator->FindAnimation("Idle");
    m_jumpAnim = m_animator->FindAnimation("Jump");
    m_runAnim = m_animator->FindAnimation("Run");
    m_upAnim = m_animator->FindAnimation("Up");
    m_crawlAnim = m_animator->FindAnimation("Crawl");
    m_dieAnim = m_animator->FindAnimation("Die");
    m_runUpAnim = m_animator->FindAnimation("RunUp");
    m_runDownAnim = m_animator->FindAnimation("RunDown");
    m_facingRight = true;
    m_shootDowntime = 0;
}
