//
// Created by david on 13/2/20.
//

#include "enemies.h"

#include <utility>
#include "AnimationRenderer.h"

#define FIRE_SHIFT (Vector2D(-13, -11) * PIXELS_ZOOM)
#define FIRE_SHIFT_MIRROR (Vector2D(13, -11) * PIXELS_ZOOM)

void Ledder::Create(AvancezLib *engine, std::set<GameObject *> *game_objects, ObjectPool<Bullet> *bullet_pool,
                    Player *player, std::shared_ptr<Sprite> enemies_spritesheet, float *camera_x, Grid *grid,
                    int layer, float time_hidden, float time_shown, float cooldown_time, bool show_standing,
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
                -1, layer);
    } else {
        collider->Create(engine, this, game_objects, grid, camera_x,
                -6 * PIXELS_ZOOM, -10 * PIXELS_ZOOM,
                10 * PIXELS_ZOOM, 15 * PIXELS_ZOOM,
                -1, layer);
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
                game_objects->erase(go);
            }
            break;
    }
}

void LedderBehaviour::Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects,
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

        bullet->Init(go->position + shift, BulletBehaviour::ENEMY_BULLET_DEFAULT, direction, 80 * PIXELS_ZOOM);
        game_objects[RENDERING_LAYER_BULLETS].insert(bullet);
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
