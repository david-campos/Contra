//
// Created by david on 20/2/20.
//

#ifndef CONTRA_DEFENSE_WALL_H
#define CONTRA_DEFENSE_WALL_H

#include "component.h"
#include "bullets.h"
#include "Gravity.h"

class BlasterCanonBehaviour : public Component {
private:
    float m_nextShootWait;
    ObjectPool<Bullet> *m_bulletPool;
    AnimationRenderer *m_animator;

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

    void Destroy() override {
        m_bulletPool->Destroy();
        Component::Destroy();
    }
};

#endif //CONTRA_DEFENSE_WALL_H
