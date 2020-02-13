//
// Created by david on 13/2/20.
//

#include "enemies.h"

#include <utility>
#include "AnimationRenderer.h"

#define FIRE_SHIFT (Vector2D(-13, -11) * PIXELS_ZOOM)
#define FIRE_SHIFT_MIRROR (Vector2D(13, -11) * PIXELS_ZOOM)

void Ledder::Create(AvancezLib *engine, std::set<GameObject *> *game_objects, ObjectPool<Bullet> *bullet_pool,
                    Player *player, std::shared_ptr<Sprite> enemies_spritesheet, float *camera_x, Grid* grid,
                    int layer, float time_hidden, float time_shown, float cooldown_time) {
    GameObject::Create();
    auto *renderer = new AnimationRenderer();
    renderer->Create(engine, this, game_objects, std::move(enemies_spritesheet), camera_x);
    renderer->AddAnimation({
            214, 59, 0.2, 3,
            25, 16, 19, 16,
            "Showing", AnimationRenderer::STOP_AND_LAST
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
    renderer->CurrentAndPause(0);

    auto *behaviour = new LedderBehaviour();
    behaviour->Create(engine, this, game_objects, bullet_pool, player,
            time_hidden, time_shown, cooldown_time);

    auto *collider = new BoxCollider();
    collider->Create(engine, this, game_objects, grid, camera_x,
            -6 * PIXELS_ZOOM, -10 * PIXELS_ZOOM,
            10 * PIXELS_ZOOM, 10 * PIXELS_ZOOM, -1, layer);
    collider->SetListener(behaviour);

    AddComponent(behaviour);
    AddComponent(collider);
    AddComponent(renderer);
}

void LedderBehaviour::Update(float dt) {
    m_currentStateTime += dt;
    switch (m_state) {
        case SHOWN:
            m_animator->mirrorHorizontal = go->position.x < m_playerCollider->AbsoluteTopLeftX();
            if (m_currentStateTime > m_timeShown) {
                m_animator->Play(m_animator->GetCurrentAnimation().frames - 1, false);
                ChangeToState(HIDING);
            } else if (m_coolDown <= 0) {
                if (go->position.y < m_playerCollider->AbsoluteBottomRightY()
                    && go->position.y > m_playerCollider->AbsoluteTopLeftY()) {
                    Fire();
                    m_coolDown = m_coolDownTime;
                }
            } else {
                m_coolDown -= dt;
            }
            break;
        case HIDDEN:
            if (m_currentStateTime > m_timeHidden) {
                m_animator->Play(0, true);
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
                m_animator->PlayAnimation(2);
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
                             float time_shown, float cooldown_time) {
    Component::Create(engine, go, game_objects);
    m_timeHidden = time_hidden;
    m_timeShown = time_shown;
    m_coolDownTime = cooldown_time;
    m_bulletPool = bullet_pool;
    m_playerCollider = player->GetComponent<BoxCollider *>();
}

void LedderBehaviour::ChangeToState(LedderBehaviour::State state) {
    m_state = state;
    m_currentStateTime = 0;
}

void LedderBehaviour::Fire() {
    // Grab the bullet from the pool
    auto *bullet = m_bulletPool->FirstAvailable();
    bool mirrored = m_animator->mirrorHorizontal;
    if (bullet != nullptr) {
        bullet->Init(go->position + (mirrored ? FIRE_SHIFT_MIRROR : FIRE_SHIFT),
                BulletBehaviour::ENEMY_BULLET_DEFAULT,
                Vector2D(mirrored ? 1 : -1, 0));
        game_objects[RENDERING_LAYER_BULLETS].insert(bullet);
    }
}

void LedderBehaviour::OnCollision(const CollideComponent &collider) {
    if (m_state == SHOWN || m_state == HIDING) {
        auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
        if (bullet) {
            bullet->Kill();

            m_animator->PlayAnimation(1);
            ChangeToState(GOING_TO_DIE);
        }
    }
}
