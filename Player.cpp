//
// Created by david on 4/2/20.
//

#include "Player.h"

#include <memory>
#include "consts.h"
#include "enemies.h"
#include "pickups.h"

void PlayerControl::Update(float dt) {
    AvancezLib::KeyStatus keyStatus{};
    engine->getKeyStatus(keyStatus);

    if (keyStatus.debug && !m_previousKeyStatus.debug) {
        m_godMode = !m_godMode;
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "God mode: %s", m_godMode ? "ON" : "OFF");
    }

    if (m_invincibleTime > 0) {
        bool display = (int) round(m_invincibleTime * 10) % 2 == 0;
        m_invincibleTime -= dt;
        if (m_invincibleTime <= 0) display = true;
        m_animator->enabled = display; // It appears and twinkles when invincible
    }

    // If it is death, wait for death animation to end and then decide
    if (m_isDeath) {
        if (m_waitDead <= 0) {
            if (m_remainingLives > 0) {
                m_remainingLives--;
                Respawn();
            } else {
                go->Send(GAME_OVER);
            }
        } else {
            m_waitDead -= dt;
            if (m_animator->IsPlaying(m_dieAnim)) {
                go->position =
                        go->position + Vector2D(PLAYER_SPEED * PIXELS_ZOOM * dt * (m_facingRight ? -1.f : 1.f), 0);
            }
        }
        return;
    }

    if (go->position.y > WINDOW_HEIGHT) {
        Kill();
        m_gravity->SetVelocity(-PLAYER_JUMP * PIXELS_ZOOM * .75);
        return;
    }

    if (m_gravity->IsOnFloor()) {
        m_hasInertia = false;
        if (keyStatus.jump && !m_previousKeyStatus.jump) {
            if (keyStatus.down and not(keyStatus.left or keyStatus.right)) {
                if (m_gravity->CanFall())
                    m_gravity->LetFall();
            } else {
                m_gravity->AddVelocity(-PLAYER_JUMP * PIXELS_ZOOM);
                m_animator->PlayAnimation(m_jumpAnim);
            }
        }
    }
    if (keyStatus.right || (m_gravity->IsOnAir() && m_hasInertia && m_facingRight)) {
        go->position = go->position + Vector2D(PLAYER_SPEED * PIXELS_ZOOM * dt, 0);
        m_hasInertia = true;
        m_facingRight = true;
    }
    if (keyStatus.left || (m_gravity->IsOnAir() && m_hasInertia && !m_facingRight)) {
        go->position = go->position - Vector2D(PLAYER_SPEED * PIXELS_ZOOM * dt, 0);
        // The player can't go back in Contra
        if (go->position.x - 12 < *m_cameraX) {
            go->position.x = *m_cameraX + 12;
        }
        m_hasInertia = true;
        m_facingRight = false;
    }
    // Shooting (we need facing to be calculated already)
    bool shooting = false;
    if (m_currentWeapon->ShouldFire(keyStatus.fire, dt))
        shooting = Fire(keyStatus);

    Box *box = &m_standingBox;
    m_diving = false;
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
                box = &m_crawlingBox;
            } else {
                m_animator->CurrentAndPause(m_idleAnim);
            }
            if (shooting) {
                m_animator->Play(1); // These animations make the shooting when played
            }
        }
    } else if (m_gravity->IsOnWater()) {
        box = &m_swimmingBox;
        if (!m_wasInWater) {
            m_animator->PlayAnimation(m_splashAnim);
        } else if (not(m_animator->IsCurrent(m_splashAnim)
                       and m_animator->IsPlaying())) {
            if (keyStatus.down) {
                m_animator->PlayAnimation(m_diveAnim);
                m_diving = true;
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
        box = &m_jumpBox;
        if (!m_animator->IsCurrent(m_jumpAnim)) {
            m_animator->PlayAnimation(m_fallAnim);
        }
    }
    m_wasInWater = m_gravity->IsOnWater();
    m_animator->mirrorHorizontal = !m_facingRight;
    if (m_facingRight) {
        m_collider->ChangeBox(*box);
    } else {
        Box flipped;
        flipped.top_left_x = -box->bottom_right_x;
        flipped.bottom_right_x = -box->top_left_x;
        flipped.top_left_y = box->top_left_y;
        flipped.bottom_right_y = box->bottom_right_y;
        m_collider->ChangeBox(flipped);
    }

    m_previousKeyStatus = keyStatus;
}

void PlayerControl::Init() {
    m_animator = go->GetComponent<AnimationRenderer *>();
    m_collider = go->GetComponent<BoxCollider *>();
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
    m_jumpBox = {
            -6 * PIXELS_ZOOM, -22 * PIXELS_ZOOM,
            6 * PIXELS_ZOOM, -10 * PIXELS_ZOOM
    };
    m_standingBox = {
            -3 * PIXELS_ZOOM, -33 * PIXELS_ZOOM,
            3 * PIXELS_ZOOM, -1 * PIXELS_ZOOM
    };
    m_crawlingBox = {
            -12 * PIXELS_ZOOM, -10 * PIXELS_ZOOM,
            15 * PIXELS_ZOOM, -1 * PIXELS_ZOOM
    };
    m_swimmingBox = {
            -3 * PIXELS_ZOOM, -10 * PIXELS_ZOOM,
            7 * PIXELS_ZOOM, 0 * PIXELS_ZOOM
    };
    m_currentWeapon = std::make_unique<DefaultWeapon>(m_bulletPool, game_objects);
    m_previousKeyStatus = {false, false, false, false, false, false, false,
                           false};
    m_remainingLives = 2;
    m_hasInertia = false;
    m_facingRight = true;
    m_diving = false;
}

void PlayerControl::Kill() {
    m_isDeath = true;
    m_waitDead = 2.f;
    m_animator->PlayAnimation(m_dieAnim);
    m_gravity->SetFallThoughWater(true);
}

void PlayerControl::Respawn() {
    if (!m_isDeath) return;
    go->position = Vector2D(*m_cameraX + 50 * PIXELS_ZOOM, 0);
    m_gravity->SetFallThoughWater(false);
    m_facingRight = true;
    m_hasInertia = false;
    m_gravity->SetVelocity(0);
    m_invincibleTime = 2.f;
    m_isDeath = false;
}

bool PlayerControl::Fire(const AvancezLib::KeyStatus &keyStatus) {
    Vector2D displacement(
            m_facingRight ? 12 : -12,
            m_gravity->IsOnWater() ? -3 : -21
    );
    Vector2D direction(m_facingRight ? 1 : -1, 0);
    if (keyStatus.up && !keyStatus.down) {
        direction.y = -1;
        displacement.y = m_gravity->IsOnWater() ? -25 : -35;
        if (!keyStatus.right && !keyStatus.left) {
            direction.x = 0;
            displacement.x = m_facingRight ? 3 : -3;
        } else if (m_gravity->IsOnWater()) {
            displacement.y = -20;
        }
        if (m_animator->IsCurrent(m_jumpAnim)) {
            displacement.x = 0;
        }
    } else if (keyStatus.down && !keyStatus.up) {
        if (m_gravity->IsOnWater())
            return false; // Can't shoot down in water, since it is diving
        displacement.y = -9;
        if (keyStatus.right or keyStatus.left or m_animator->IsCurrent(m_jumpAnim)) {
            direction.y = 1; // When jumping or moving you can indeed shoot down
            if (!keyStatus.left && !keyStatus.right) {
                displacement = Vector2D(0, 0);
                direction.x = 0;
            }
        }
    } else if (m_animator->IsCurrent(m_jumpAnim)) {
        displacement.y = -16;
    }
    return m_currentWeapon->Fire(go->position + displacement * PIXELS_ZOOM, direction);
}

void PlayerControl::OnCollision(const CollideComponent &collider) {
    if (m_invincibleTime <= 0 && !m_isDeath && !m_diving && !m_godMode) {
        auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
        if (bullet) {
            Kill();
            m_gravity->AddVelocity(-PLAYER_JUMP / 2.f);
            bullet->Kill();
            return;
        }
        auto *greeder = collider.GetGameObject()->GetComponent<GreederBehaviour *>();
        if (greeder) {
            if (greeder->IsAlive()) {
                Kill();
                m_gravity->AddVelocity(-PLAYER_JUMP / 2.f);
            }
            return;
        }
        auto *pickup = collider.GetGameObject()->GetComponent<PickUpBehaviour*>();
        if (pickup) {
            SDL_Log("PICKED %d", pickup->GetType());
            game_objects[RENDERING_LAYER_ENEMIES]->erase(pickup->GetGameObject());
            pickup->GetGameObject()->Destroy();
        }
    }
}

void
Player::Create(AvancezLib *engine, std::set<GameObject *> **game_objects,
               const std::shared_ptr<Sprite> &spritesheet, const std::weak_ptr<Floor> &floor, float *camera_x,
               ObjectPool<Bullet> *bullet_pool, Grid *grid, int player_collision_layer) {
    position = Vector2D(50 * PIXELS_ZOOM, 0);
    auto *renderer = new AnimationRenderer();
    renderer->Create(engine, this, game_objects, spritesheet, camera_x);
    renderer->AddAnimation({
            0, 8, 0.1, 2,
            24, 34, 8, 33,
            "Idle", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            49, 0, 0.1, 2,
            15, 42, 7, 42,
            "Up", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            79, 25, 0.1, 2,
            33, 17, 16, 17,
            "Crawl", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            0, 43, 0.1, 6,
            20, 35, 10, 35,
            "Run", AnimationRenderer::DONT_STOP
    });
    renderer->AddAnimation({
            80, 43, 1, 1,
            20, 35, 10, 35,
            "Fall", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            0, 79, 0.1, 3,
            25, 34, 12, 34,
            "RunShoot", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            0, 149, 0.1, 3,
            20, 35, 10, 35,
            "RunUp", AnimationRenderer::DONT_STOP
    });
    renderer->AddAnimation({
            0, 221, 0.1, 3,
            22, 35, 11, 35,
            "RunDown", AnimationRenderer::DONT_STOP
    });
    renderer->AddAnimation({
            122, 52, 0.1, 4,
            20, 20, 10, 26,
            "Jump", AnimationRenderer::DONT_STOP
    });
    renderer->AddAnimation({
            0, 307, 0.2, 1,
            18, 16, 9, 13,
            "Splash", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            18, 303, 0.2, 2,
            17, 16, 8, 12,
            "Dive", AnimationRenderer::DONT_STOP
    });
    renderer->AddAnimation({
            52, 303, 0.2, 2,
            17, 16, 8, 12,
            "Swim", AnimationRenderer::DONT_STOP
    });
    renderer->AddAnimation({
            60, 328, 0.2, 2,
            26, 18, 8, 14,
            "SwimShoot", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            90, 299, 0.2, 2,
            20, 20, 10, 15,
            "SwimShootDiagonal", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            130, 290, 0.2, 2,
            18, 29, 9, 25,
            "SwimShootUp", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            61, 161, 0.1, 5,
            32, 23, 16, 23,
            "Die", AnimationRenderer::STOP_AND_LAST
    });

    auto *gravity = new Gravity();
    gravity->Create(engine, this, game_objects, floor);
    auto *playerControl = new PlayerControl();
    playerControl->Create(engine, this, game_objects, camera_x, bullet_pool);
    auto *collider = new BoxCollider();
    collider->Create(engine, this, game_objects, grid, camera_x,
            -3 * PIXELS_ZOOM, -33 * PIXELS_ZOOM,
            6 * PIXELS_ZOOM, 34 * PIXELS_ZOOM, -1, player_collision_layer);
    collider->SetListener(playerControl);
    AddComponent(gravity);
    AddComponent(playerControl);
    AddComponent(renderer);
    AddComponent(collider);
}
