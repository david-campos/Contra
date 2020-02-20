//
// Created by david on 20/2/20.
//

#ifndef CONTRA_DEFENSE_WALL_H
#define CONTRA_DEFENSE_WALL_H

#include "component.h"
#include "bullets.h"
#include "Gravity.h"

#define EXPLOSION_STEPS 4
#define TIME_BETWEEN_EXPLOSIONS 0.6f

class BlasterCanonBehaviour : public Component, public CollideComponentListener {
private:
    float m_nextShootWait;
    ObjectPool<Bullet> *m_bulletPool;
    AnimationRenderer *m_animator;
    int m_lives;

    std::random_device rd;
    std::mt19937 m_mt = std::mt19937(rd());
    std::uniform_real_distribution<float> m_random_dist = std::uniform_real_distribution<float>(0.f, 1.f);
public:
    void Create(Level *level, GameObject *go, ObjectPool<Bullet> *bullet_pool) {
        Component::Create(level, go);
        m_bulletPool = bullet_pool;
    }

    void Init() override {
        Component::Init();
        if (!m_animator) {
            m_animator = go->GetComponent<AnimationRenderer *>();
        }
        m_lives = 16;
    }

    void Update(float dt) override {
        if (m_nextShootWait > 0) {
            m_nextShootWait -= dt;
            return;
        }
        auto *bullet = m_bulletPool->FirstAvailable();
        if (bullet != nullptr) {
            Vector2D dir = Vector2D(-1, 0);
            bullet->Init(go->position + Vector2D(2, 3) * PIXELS_ZOOM,
                    dir, (MIN_BLAST_X_SPEED + m_random_dist(m_mt) * (MAX_BLAST_X_SPEED - MIN_BLAST_X_SPEED))
                         * PIXELS_ZOOM);
            level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
            bullet->GetComponent<Gravity *>()->SetVelocity(
                    -(MIN_BLAST_Y_SPEED + m_random_dist(m_mt) * (MAX_BLAST_Y_SPEED - MIN_BLAST_Y_SPEED))
                    * PIXELS_ZOOM);
            m_animator->Play(1);
            m_nextShootWait = MIN_BLAST_WAIT + m_random_dist(m_mt) * (MAX_BLAST_WAIT - MIN_BLAST_WAIT);
        }
    }

    void OnCollision(const CollideComponent &collider) override {
        if (m_lives > 0) {
            auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
            if (bullet && !bullet->IsKilled()) {
                bullet->Kill();

                m_lives--;
                if (m_lives == 0) {
                    auto *explosion = new GameObject();
                    explosion->Create();
                    auto *renderer = new AnimationRenderer();
                    renderer->Create(level, explosion, level->GetEnemiesSpritesheet());
                    renderer->AddAnimation({
                            92, 611, 0.15, 3,
                            30, 30, 15, 15,
                            "Explosion", AnimationRenderer::BOUNCE_AND_STOP});
                    renderer->Play();
                    auto *self_destroy = new DestroyOnAnimationStop();
                    self_destroy->Create(level, explosion);
                    explosion->AddComponent(renderer);
                    explosion->AddComponent(self_destroy);
                    explosion->position = go->position + Vector2D(3, 3) * PIXELS_ZOOM;

                    explosion->Init();
                    level->AddGameObject(explosion, RENDERING_LAYER_BULLETS);
                    level->RemoveGameObject(go);
                }
            }
        }
    }

    void Destroy() override {
        m_bulletPool->Destroy();
        Component::Destroy();
    }
};

class DefenseDoorBehaviour : public Component, public CollideComponentListener {
private:
    int m_lives, m_explosionSteps;
    float m_nextExplosion;
    AnimationRenderer *m_doorAnimator;
    SimpleRenderer *m_background;
public:
    void Init() override {
        Component::Init();
        m_lives = 32;
        if (!m_doorAnimator) {
            m_doorAnimator = go->GetComponent<AnimationRenderer *>();
        }
        if (!m_background) {
            m_background = go->GetComponent<SimpleRenderer *>();
        }
    }

    void Update(float dt) override {
        if (m_lives <= 0) {
            m_nextExplosion -= dt;
            if (m_nextExplosion <= 0.f) {
                m_explosionSteps--;
                int x = round(111 * (1 - m_explosionSteps / float(EXPLOSION_STEPS + 1)));
                m_background->ChangeCoords(
                        x, m_background->GetSrcY(), 111 - x, m_background->GetHeight(),
                        -x, m_background->GetAnchorY());
                CreateExplosion(go->position + Vector2D(x, 21) * PIXELS_ZOOM);
                CreateExplosion(go->position + Vector2D(x, 43) * PIXELS_ZOOM);
                m_nextExplosion = TIME_BETWEEN_EXPLOSIONS;

                if (m_explosionSteps <= 0) {
                    level->RemoveGameObject(go);
                }
            }
        }
    }

    void OnCollision(const CollideComponent &collider) override {
        if (m_lives > 0) {
            auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
            if (bullet && !bullet->IsKilled()) {
                bullet->Kill();

                m_lives--;
                if (m_lives == 0) {
                    CreateExplosion(go->position + Vector2D(18, 32) * PIXELS_ZOOM);
                    m_nextExplosion = 2 * TIME_BETWEEN_EXPLOSIONS;
                    m_explosionSteps = EXPLOSION_STEPS;
                    m_doorAnimator->enabled = false;
                }
            }
        }
    }

    void CreateExplosion(const Vector2D &pos) {
        auto *explosion = new GameObject();
        explosion->Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(level, explosion, level->GetEnemiesSpritesheet());
        renderer->AddAnimation({
                92, 611, 0.15, 3,
                30, 30, 15, 15,
                "Explosion", AnimationRenderer::BOUNCE_AND_STOP});
        renderer->Play();
        auto *self_destroy = new DestroyOnAnimationStop();
        self_destroy->Create(level, explosion);
        explosion->AddComponent(renderer);
        explosion->AddComponent(self_destroy);
        explosion->position = pos;

        explosion->Init();
        level->AddGameObject(explosion, RENDERING_LAYER_BULLETS);
    }
};

#endif //CONTRA_DEFENSE_WALL_H
