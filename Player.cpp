//
// Created by david on 4/2/20.
//

#include "Player.h"
#include "consts.h"

void PlayerControl::Update(float dt) {
    m_shootDowntime -= dt;

    AvancezLib::KeyStatus keyStatus{};
    engine->getKeyStatus(keyStatus);

    // If it is death, wait for death animation to end and then decide
    if (m_isDeath) {
        if (!m_animator->IsPlaying()) {
            // Respawn or end game
            printf("TODO: Respawn or end game\n");
        }
        return;
    }

    if (go->position.y > WINDOW_HEIGHT) {
        Kill();
        m_gravity->SetVelocity(-PLAYER_JUMP * .75);
        return;
    }

    if (m_gravity->IsOnFloor()) {
        m_hasInertia = false;
        if (keyStatus.jump) {
            if (keyStatus.down and not (keyStatus.left or keyStatus.right)) {
                if (m_gravity->CanFall())
                    m_gravity->LetFall();
            } else {
                m_gravity->AddVelocity(-PLAYER_JUMP);
                m_animator->PlayAnimation(m_jumpAnim);
            }
        }
    }
    if (keyStatus.right || (m_gravity->IsOnAir() && m_hasInertia && m_facingRight)) {
        go->position = go->position + Vector2D(PLAYER_SPEED * dt, 0);
        m_hasInertia = true;
        m_facingRight = true;
    }
    if (keyStatus.left || (m_gravity->IsOnAir() && m_hasInertia && !m_facingRight)) {
        go->position = go->position - Vector2D(PLAYER_SPEED * dt, 0);
        // The player can't go back in Contra
        if (go->position.x - 12 < *m_cameraX) {
            go->position.x = *m_cameraX + 12;
        }
        m_hasInertia = true;
        m_facingRight = false;
    }
    // Shooting
    bool shooting = !m_hasShot && keyStatus.fire && m_shootDowntime <= 0;
    if (!keyStatus.fire) {
        m_hasShot = false;
    }
    if (shooting) {
        m_hasShot = true;
        m_shootDowntime = 0.3;
    }

    // Animation
    if (m_gravity->IsOnFloor()) {
        if (keyStatus.right || keyStatus.left) {
            if (keyStatus.up && !keyStatus.down) {
                m_animator->PlayAnimation(m_runUpAnim);
            } else if (keyStatus.down && !keyStatus.up) {
                m_animator->PlayAnimation(m_runDownAnim);
            } else {
                if (shooting) {
                    m_animator->PlayAnimation(m_runShootAnim);
                } else if (!m_animator->IsPlaying(m_runShootAnim)) {
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
            if (shooting) {
                m_animator->Play(); // These animations make the shooting when played
            }
        }
    } else if (m_gravity->IsOnWater()) {
        if (!m_wasInWater) {
            m_animator->PlayAnimation(m_splashAnim);
        } else if (not(m_animator->IsCurrent(m_splashAnim)
                       and m_animator->IsPlaying())) {
            if (keyStatus.down) {
                m_animator->PlayAnimation(m_diveAnim);
            } else if (shooting) {
                if (keyStatus.up) {
                    if (keyStatus.right || keyStatus.left) {
                        m_animator->PlayAnimation(m_swimShootDiagonalAnim);
                    } else {
                        m_animator->PlayAnimation(m_swimShootUpAnim);
                    }
                } else {
                    m_animator->PlayAnimation(m_swimShootAnim);
                }
            } else if (not(m_animator->IsCurrent(m_swimShootAnim) // Do not interrupt firing animations for swimming
                           or m_animator->IsCurrent(m_swimShootUpAnim)
                           or m_animator->IsCurrent(m_swimShootDiagonalAnim))
                       or !m_animator->IsPlaying()) {
                m_animator->PlayAnimation(m_swimAnim);
            }
        }
    } else {
        if (!m_animator->IsCurrent(m_jumpAnim)) {
            m_animator->PlayAnimation(m_fallAnim);
        }
    }
    m_wasInWater = m_gravity->IsOnWater();
    m_animator->mirrorHorizontal = !m_facingRight;
}

void PlayerControl::Init() {
    m_animator = go->GetComponent<AnimationRenderer *>();
    m_gravity = go->GetComponent<Gravity *>();
    m_idleAnim = m_animator->FindAnimation("Idle");
    m_jumpAnim = m_animator->FindAnimation("Jump");
    m_runAnim = m_animator->FindAnimation("Run");
    m_upAnim = m_animator->FindAnimation("Up");
    m_fallAnim = m_animator->FindAnimation("Fall");
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
    m_animator->PlayAnimation(m_jumpAnim); // Start jumping
    m_facingRight = true;
    m_hasShot = false;
    m_shootDowntime = 0;
}

void PlayerControl::Kill() {
    m_isDeath = true;
    m_animator->PlayAnimation(m_dieAnim);
}
