//
// Created by david on 13/2/20.
//

#include "enemies.h"

#include <utility>
#include "AnimationRenderer.h"
#include "consts.h"

#define FIRE_SHIFT (Vector2D(-13, -11) * PIXELS_ZOOM)
#define FIRE_SHIFT_MIRROR (Vector2D(13, -11) * PIXELS_ZOOM)

void Ledder::Create(AvancezLib *engine,std::set<GameObject *> **game_objects, ObjectPool<Bullet> *bullet_pool,
                    Player *player, std::shared_ptr<Sprite> enemies_spritesheet, float *camera_x, Grid *grid,
                    float time_hidden, float time_shown, float cooldown_time, bool show_standing,
                    int burst_length, float burst_cooldown, bool horizontally_precise) {
    GameObject::Create();
    auto *renderer = new AnimationRenderer();
    renderer->Create(engine, this, game_objects, std::move(enemies_spritesheet), camera_x);
    renderer->AddAnimation({
            214, 59, 0.2, 3,
            25, 16, 19, 16,
            "Showing", AnimationRenderer::STOP_AND_LAST
    });
    renderer->AddAnimation({
            1, 43, 0.2, 2,
            26, 32, 17, 31,
            "Standing", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            59, 36, 0.2, 2,
            20, 39, 12, 38,
            "ShootUp", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            106, 44, 0.2, 2,
            25, 31, 17, 30,
            "ShootDown", AnimationRenderer::STOP_AND_FIRST
    });
    renderer->AddAnimation({
            179, 36, 0.25, 1,
            18, 39, 6, 26,
            "GoingToDie", AnimationRenderer::STOP_AND_LAST
    });
    renderer->AddAnimation({
            186, 610, 0.15, 3,
            34, 34, 17, 26,
            "Dying", AnimationRenderer::STOP_AND_LAST
    });
    if (time_hidden > 0) renderer->CurrentAndPause(0);

    auto *behaviour = new LedderBehaviour();
    behaviour->Create(engine, this, game_objects, bullet_pool, player,
            time_hidden, time_shown, cooldown_time, show_standing, burst_length, burst_cooldown, horizontally_precise);

    auto *collider = new BoxCollider();
    if (show_standing) {
        collider->Create(engine, this, game_objects, grid, camera_x,
                -6 * PIXELS_ZOOM, -27 * PIXELS_ZOOM,
                12 * PIXELS_ZOOM, 28 * PIXELS_ZOOM,
                -1, NPCS_COLLISION_LAYER);
    } else {
        collider->Create(engine, this, game_objects, grid, camera_x,
                -6 * PIXELS_ZOOM, -10 * PIXELS_ZOOM,
                10 * PIXELS_ZOOM, 15 * PIXELS_ZOOM,
                -1, NPCS_COLLISION_LAYER);
    }
    collider->SetListener(behaviour);

    AddComponent(behaviour);
    AddComponent(collider);
    AddComponent(renderer);
}

void LedderBehaviour::Update(float dt) {
    m_currentStateTime += dt;
    switch (m_state) {
        case SHOWN:
            // If m_timeHidden is 0 or less, we do not transition back
            if (m_currentStateTime > m_timeShown && m_timeHidden > 0) {
                m_animator->PlayAnimation(m_animShow, false);
                ChangeToState(HIDING);
            } else {
                if (m_burstCoolDown > 0) m_burstCoolDown -= dt;
                if (m_coolDown > 0) m_coolDown -= dt;
                if (m_coolDown <= 0 && (m_firedInBurst < m_burstLength || m_burstCoolDown <= 0)) {
                    if (m_showStanding or (go->position.y < m_player->position.y
                                           && go->position.y > m_player->position.y - 33 * PIXELS_ZOOM)) {
                        if (m_burstCoolDown <= 0) {
                            m_burstCoolDown = m_burstCoolDownTime;
                            m_firedInBurst = 0; // New burst
                        }
                        Fire();
                        m_coolDown = m_coolDownTime;
                        m_firedInBurst++;
                    }
                }
            }
            break;
        case HIDDEN:
            if (m_currentStateTime > m_timeHidden) {
                m_animator->PlayAnimation(m_animShow, true);
                ChangeToState(SHOWING);
            }
            break;
        case HIDING:
        case SHOWING:
            if (!m_animator->IsPlaying())
                ChangeToState(m_state == HIDING ? HIDDEN : SHOWN);
            break;
        case GOING_TO_DIE:
            if (!m_animator->IsPlaying()) {
                m_animator->PlayAnimation(m_animDying);
                ChangeToState(DYING);
            } else {
                go->position = go->position + Vector2D(m_animator->mirrorHorizontal ? -0.5f : 0.5f, -1);
            }
            break;
        case DYING:
            if (!m_animator->IsPlaying()) {
                go->Disable();
                game_objects[RENDERING_LAYER_ENEMIES]->erase(go);
            }
            break;
    }
}

void LedderBehaviour::Create(AvancezLib *engine, GameObject *go,std::set<GameObject *> **game_objects,
                             ObjectPool<Bullet> *bullet_pool, Player *player, float time_hidden,
                             float time_shown, float cooldown_time, bool show_standing,
                             int burst_length, float burst_cooldown, bool horizontally_precise) {
    Component::Create(engine, go, game_objects);
    m_timeHidden = time_hidden;
    m_timeShown = time_shown;
    m_coolDownTime = cooldown_time;
    m_bulletPool = bullet_pool;
    m_burstCoolDownTime = burst_cooldown;
    m_burstLength = burst_length;
    m_showStanding = show_standing;
    m_horizontallyPrecise = horizontally_precise;
    m_player = player;
}

void LedderBehaviour::ChangeToState(LedderBehaviour::State state) {
    m_state = state;
    m_currentStateTime = 0;
}

void LedderBehaviour::Fire() {
    // Grab the bullet from the pool
    auto *bullet = m_bulletPool->FirstAvailable();
    if (bullet != nullptr) {
        Vector2D shift, direction;
        // All position changes occur when firing
        bool mirrored = go->position.x < m_player->position.x;
        m_animator->mirrorHorizontal = mirrored;
        if (m_showStanding) {
            // In the original game for NES they shoot like the canons, limited to some angles,
            // however, despite we respect in many things the original game, we consider this a consequence
            // of the limitations in the time, and we want to give them more precision for this one to make
            // it a bit harder. Still, if we shoot horizontal we want to shoot only horizontal
            // like in the original game cause it seems to cause interesting gameplay.
            auto player_y = (float) m_player->position.y;
            if (player_y - 33 * PIXELS_ZOOM > go->position.y) {
                m_animator->PlayAnimation(m_animShootDown, true, 1);
                direction = m_player->position - Vector2D(0, 16 * PIXELS_ZOOM) - go->position - shift;
                shift = Vector2D(mirrored ? 16 : -16, -12) * PIXELS_ZOOM;
            } else if (player_y < go->position.y - 33 * PIXELS_ZOOM) {
                shift = Vector2D(mirrored ? 11 : -11, -36) * PIXELS_ZOOM;
                direction = m_player->position - Vector2D(0, 16 * PIXELS_ZOOM) - go->position - shift;
                m_animator->PlayAnimation(m_animShootUp, true, 1);
            } else {
                shift = Vector2D(mirrored ? 16 : -16, -25) * PIXELS_ZOOM;
                if (m_horizontallyPrecise) {
                    direction = m_player->position - Vector2D(0, 16 * PIXELS_ZOOM) - go->position - shift;
                } else {
                    direction = Vector2D(mirrored ? 1 : -1, 0);
                }
                m_animator->PlayAnimation(m_animStanding, true, 1);
            }
        } else {
            shift = (mirrored ? FIRE_SHIFT_MIRROR : FIRE_SHIFT);
            if (m_horizontallyPrecise) {
                direction = m_player->position - Vector2D(0, 16) - go->position - shift;
            } else {
                direction = Vector2D(mirrored ? 1 : -1, 0);
            }
            m_animator->CurrentAndPause(m_animShow, false);
        }

        bullet->Init(go->position + shift, direction, 80 * PIXELS_ZOOM);
        game_objects[RENDERING_LAYER_BULLETS]->insert(bullet);
    }
}

void LedderBehaviour::OnCollision(const CollideComponent &collider) {
    if (m_state == SHOWN || m_state == HIDING) {
        auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
        if (bullet) {
            bullet->Kill();

            m_animator->PlayAnimation(m_animGoingToDie);
            ChangeToState(GOING_TO_DIE);
        }
    }
}

void Greeder::Create(AvancezLib *engine,std::set<GameObject *> **game_objects,
                     std::shared_ptr<Sprite> enemies_spritesheet, float *camera_x, Grid *grid,
                     const std::weak_ptr<Floor> &the_floor) {
    GameObject::Create();
    auto *renderer = new AnimationRenderer();
    renderer->Create(engine, this, game_objects, std::move(enemies_spritesheet), camera_x);
    renderer->AddAnimation({
            1, 2, 0.1, 3,
            17, 32, 9, 32,
            "Running", AnimationRenderer::DONT_STOP
    });
    renderer->AddAnimation({
            109, 6, 0.2, 1,
            17, 32, 9, 32,
            "Jumping", AnimationRenderer::DONT_STOP
    });
    renderer->AddAnimation({
            286, 11, 0.2, 2,
            18, 15, 9, 14,
            "Drowning", AnimationRenderer::STOP_AND_LAST
    });
    renderer->AddAnimation({
            186, 610, 0.15, 3,
            34, 34, 17, 26,
            "Dying", AnimationRenderer::STOP_AND_LAST
    });
    renderer->Play();
    auto *gravity = new Gravity();
    gravity->Create(engine, this, game_objects, the_floor);
    auto *behaviour = new GreederBehaviour();
    behaviour->Create(engine, this, game_objects, the_floor);
    auto *collider = new BoxCollider();
    collider->Create(engine, this, game_objects, grid, camera_x,
            -5 * PIXELS_ZOOM, -32 * PIXELS_ZOOM,
            9 * PIXELS_ZOOM, 32 * PIXELS_ZOOM,
            PLAYER_COLLISION_LAYER, NPCS_COLLISION_LAYER);
    collider->SetListener(behaviour);

    AddComponent(behaviour);
    AddComponent(gravity);
    AddComponent(collider);
    AddComponent(renderer);
}

void GreederBehaviour::Update(float dt) {
    auto level_floor = m_floor.lock();
    if (!level_floor) return;

    if (m_isDeath) {
        if (m_deathFor < 0.25) {
            m_deathFor += dt;
            if (m_deathFor >= 0.25) {
                m_animator->PlayAnimation(m_animDying);
                m_gravity->SetAcceleration(0);
                m_gravity->SetVelocity(0);
            }
        } else {
            if (!m_animator->IsPlaying()) {
                go->Disable();
                game_objects[RENDERING_LAYER_ENEMIES]->erase(go);
                if (go->onOutOfScreen == GameObject::DISABLE_AND_DESTROY) {
                    go->Destroy();
                }
            }
        }
    } else if (m_gravity->IsOnFloor()) {
        go->position = go->position + Vector2D(PLAYER_SPEED * PIXELS_ZOOM, 0) * m_direction * dt;
        m_animator->PlayAnimation(m_animRunning);
        float x = std::floor((go->position.x + 4 * PIXELS_ZOOM * m_direction) / PIXELS_ZOOM);
        float y = std::floor(go->position.y / PIXELS_ZOOM);
        if (!level_floor->IsFloor(x, y)) {
            if (m_direction == 1 || m_random_dist(m_mt) <= 0.2f) {
                m_direction *= -1;
            } else {
                m_gravity->SetVelocity(-PLAYER_JUMP / 1.3 * PIXELS_ZOOM);
            }
        }
    } else if (m_gravity->IsOnWater()) {
        m_isDeath = true;
        m_deathFor = 10.f; // Just enough so it dies as soon as the animation is ended
        m_gravity->SetVelocity(0);
        m_gravity->SetAcceleration(0);
        m_animator->PlayAnimation(m_animDrowning);
    } else {
        go->position = go->position + Vector2D(PLAYER_SPEED * PIXELS_ZOOM * 1.1, 0) * m_direction * dt;
        m_animator->PlayAnimation(m_animJumping);
    }
    m_animator->mirrorHorizontal = m_direction == 1;
}

void GreederBehaviour::OnCollision(const CollideComponent &collider) {
    if (!m_isDeath) {
        auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
        if (bullet) {
            bullet->Kill();
            m_animator->PlayAnimation(m_animJumping);
            m_gravity->SetVelocity(-3 * PLAYER_SPEED * PIXELS_ZOOM);
            m_deathFor = 0;
            m_isDeath = true;
        }
    }
}

void GreederBehaviour::Create(AvancezLib *engine, GameObject *go,std::set<GameObject *> **game_objects,
                              std::weak_ptr<Floor> the_floor) {
    Component::Create(engine, go, game_objects);
    m_floor = std::move(the_floor);
}

void GreederSpawner::Create(AvancezLib *engine, GameObject* go,std::set<GameObject *> **game_objects,
                            std::shared_ptr<Sprite> enemies_spritesheet, float *camera_x, Grid *grid,
                            const std::weak_ptr<Floor> &the_floor, GameObject* receiver, float random_interval) {
    Component::Create(engine, go, game_objects);
    m_greeder = new Greeder();
    this->m_cameraX = camera_x;
    m_greeder->Create(engine, game_objects, std::move(enemies_spritesheet), camera_x, grid, the_floor);
    m_greeder->onOutOfScreen = GameObject::JUST_DISABLE;
    m_greeder->AddReceiver(receiver);
    m_randomInterval = random_interval;
}

void GreederSpawner::Update(float dt) {
    m_intervalCount -= dt;
    if (m_intervalCount > m_randomInterval)
        return;
    m_intervalCount = m_randomInterval;

    if (go->IsEnabled()) {
        if(*m_cameraX + WINDOW_WIDTH + 8 * PIXELS_ZOOM <= go->position.x) { // If the spawn is visible, avoid spawning
            if (!m_greeder->IsEnabled()) {
                m_greeder->position = go->position;
                m_greeder->Init();
                game_objects[RENDERING_LAYER_ENEMIES]->insert(m_greeder);
            }
        } else {
            go->Disable();
        }
    }
}

void GreederSpawner::Destroy() {
    Component::Destroy();
    game_objects[RENDERING_LAYER_ENEMIES]->erase(m_greeder);
    m_greeder->Destroy();
}

