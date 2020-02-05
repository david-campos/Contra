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
        m_facingRight = true;
    }
    if (keyStatus.left) {
        go->position = go->position - Vector2D(PLAYER_SPEED * dt, 0);
        m_facingRight = false;
    }
    bool firing = keyStatus.fire && m_shootDowntime <= 0;
    if (firing) {
        m_shootDowntime = 0.3;
    }

    // Animation
    if (keyStatus.right || keyStatus.left) {
        if (keyStatus.up && !keyStatus.down) {
            m_animator->PlayAnimation(m_runUpAnim);
        } else if (keyStatus.down && !keyStatus.up) {
            m_animator->PlayAnimation(m_runDownAnim);
        } else {
            if (firing) {
                m_animator->PlayAnimation(m_runShootAnim);
            } else if(!m_animator->IsPlaying(m_runShootAnim)) {
                m_animator->PlayAnimation(m_runAnim);
            }
        }
    } else if (!keyStatus.right && !keyStatus.left) {
        if (keyStatus.up && !keyStatus.down) {
            m_animator->CurrentAndPause(m_upAnim);
        } else if (keyStatus.down && !keyStatus.up) {
            m_animator->CurrentAndPause(m_crawlAnim);
        } else {
            m_animator->CurrentAndPause(m_idleAnim);
        }
        if (firing) {
            m_animator->Play();
        }
    }
    m_animator->mirrorHorizontal = !m_facingRight;
}

void PlayerControl::Init() {
    m_animator = go->GetComponent<AnimationRenderer *>();
    m_idleAnim = m_animator->FindAnimation("Idle");
    m_jumpAnim = m_animator->FindAnimation("Jump");
    m_runAnim = m_animator->FindAnimation("Run");
    m_upAnim = m_animator->FindAnimation("Up");
    m_crawlAnim = m_animator->FindAnimation("Crawl");
    m_dieAnim = m_animator->FindAnimation("Die");
    m_runUpAnim = m_animator->FindAnimation("RunUp");
    m_runDownAnim = m_animator->FindAnimation("RunDown");
    m_runShootAnim = m_animator->FindAnimation("RunShoot");
    m_splashAnim = m_animator->FindAnimation("Splash");
    m_swimAnim = m_animator->FindAnimation("Swim");
    m_diveAnim = m_animator->FindAnimation("Dive");
    m_swimShootAnim = m_animator->FindAnimation("SwimShoot");
    m_swimShootDiagonalAnim = m_animator->FindAnimation("SwimShootDiagonal");
    m_swimShootUpAnim = m_animator->FindAnimation("SwimShootUp");
    m_facingRight = true;
    m_shootDowntime = 0;
}
