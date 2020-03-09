//
// Created by david on 8/3/20.
//

#ifndef CONTRA_GARMAKILMA_H
#define CONTRA_GARMAKILMA_H

#include "../../../kernel/game_object.h"
#include "hidden_destroyable.h"
#include "../../level/perspective_level.h"
#include "../../../components/collision/BoxCollider.h"

class GarmakilmaBulletListener : public LevelComponent, public CollideComponentListener {
private:
    Hittable *m_behaviour;
    AnimationRenderer *m_animator;
    int m_animHit;
    float m_stopAnimHit;
public:
    void Init() override {
        Component::Init();
        if (!m_behaviour) {
            m_behaviour = GetComponent<Hittable *>();
        }
        if (!m_animator) {
            m_animator = GetComponent<AnimationRenderer *>();
            m_animHit = m_animator->FindAnimation("GlowingRed");
        }
    }

    void OnCollision(const CollideComponent &collider) override {
        auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
        if (bullet && !bullet->IsKilled() && m_behaviour) {
            m_animator->PlayAnimation(m_animHit);
            m_stopAnimHit = 2.f;
            m_behaviour->Hit();
            bullet->Kill();
        }
    }

    void Update(float dt) override {
        if (m_stopAnimHit > 0) {
            m_stopAnimHit -= dt;
            if (m_stopAnimHit <= 0) {
                m_animator->PlayAnimation(0);
            }
        }
    }
};

class GarmakilmaCore : public GameObject {
public:
    void Create(Level *level) {
        GameObject::Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        renderer->AddAnimation({
                4, 745, 0.1, 3,
                33, 30, 16, 15,
                "Glowing", AnimationRenderer::BOUNCE
        });
        renderer->Play();
        auto *behaviour = new HiddenDestroyableBehaviour();
        behaviour->Create(level, this, 8, false, 4.f);
        auto *collider = new BoxCollider();
        collider->Create(level, this,
                -16 * PIXELS_ZOOM, -15 * PIXELS_ZOOM,
                32 * PIXELS_ZOOM, 30 * PIXELS_ZOOM,
                -1, PERSP_PLAYER_BULLETS_COLLISION_LAYER);
        auto *listener = new GarmakilmaBulletListener();
        listener->Create(level, this);
        collider->SetListener(listener);

        AddComponent(behaviour);
        AddComponent(listener);
        AddComponent(collider);
        AddComponent(renderer);
    }
};

class GarmakilmaCanonShooting : public HiddenDestroyableShootingKillable {
private:
    ObjectPool<Bullet> *m_bulletPool;
public:
    void Create(Level *level, GameObject *go, float downtime, ObjectPool<Bullet> *bullet_pool) {
        HiddenDestroyableShootingKillable::Create(level, go, downtime, 0.f);
        m_bulletPool = bullet_pool;
    }

    void Init() override {
        HiddenDestroyableShootingKillable::Init();
        m_untilNextShoot = 0.1f;
    }

    void Fire() override {
        auto bullets = m_bulletPool->FirstAvailableN(3);
        for (int i = 0; i < bullets.size(); i++) {
            float angle = (-120 + i * 30) / 180.0 * 3.1416;
            Vector2D dir(cosf(angle), -sinf(angle)); // Notice our coords system has inverted Y
            Vector2D pos(go->position.x + 10 * (i - 1) * PIXELS_ZOOM, go->position.y);
            bullets[i]->Init(pos, dir, 0.65f * BULLET_SPEED * PIXELS_ZOOM);
            level->AddGameObject(bullets[i], RENDERING_LAYER_BULLETS);
        }
    }
};

class GarmakilmaCanon : public GameObject {
public:
    void Create(Level *level, ObjectPool<Bullet> *bullet_pool) {
        GameObject::Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        renderer->AddAnimation({
                4, 714, 0.1, 3,
                33, 30, 16, 15,
                "Glowing", AnimationRenderer::BOUNCE
        });
        renderer->AddAnimation({
                4, 682, 0.1, 3,
                33, 30, 16, 15,
                "Open", AnimationRenderer::STOP_AND_LAST
        });
        renderer->AddAnimation({
                4, 651, 0.1, 3,
                33, 30, 16, 15,
                "GlowingClosed", AnimationRenderer::BOUNCE
        });
        auto *behaviour = new HiddenDestroyableBehaviour();
        behaviour->Create(level, this, 8, false, 2.f);
        auto *collider = new BoxCollider();
        collider->Create(level, this,
                -16 * PIXELS_ZOOM, -15 * PIXELS_ZOOM,
                32 * PIXELS_ZOOM, 30 * PIXELS_ZOOM,
                -1, PERSP_PLAYER_BULLETS_COLLISION_LAYER);
        auto *shooting = new GarmakilmaCanonShooting();
        shooting->Create(level, this, 2.0f, bullet_pool);
        auto *listener = new GarmakilmaBulletListener();
        listener->Create(level, this);
        collider->SetListener(listener);

        AddComponent(behaviour);
        AddComponent(shooting);
        AddComponent(listener);
        AddComponent(collider);
        AddComponent(renderer);
    }
};

class BackAndForthMovement : public Component {
private:
    Vector2D m_start, m_max;
    float m_time, m_period, m_startFactor;
public:
    void Create(BaseScene *scene, GameObject *go, const Vector2D &start, const Vector2D &end, float half_period,
                float starting_factor) {
        Component::Create(scene, go);
        m_start = start;
        m_max = Vector2D(end.x - start.x, end.y - start.y);
        m_period = half_period * 2;
        m_startFactor = starting_factor;
    }

    void Init() override {
        Component::Init();
        m_time = m_period * (m_startFactor + 0.5f);
    }

    void Update(float dt) override {
        m_time = fmod(m_time + dt, m_period);
        float param = 2 * fabs(m_time / m_period - 0.5f);
        go->position = m_start + m_max * param;
    }
};

class GarmakilmaEyeBulletBehaviour : public Component, public CollideComponentListener {
private:
    AnimationRenderer *m_animator;
    BulletBehaviour *m_bulletBehaviour;
    BoxCollider *m_collider;
public:
    void Init() override {
        Component::Init();
        if (!m_animator) {
            m_animator = GetComponent<AnimationRenderer *>();
        }
        if (!m_collider) {
            m_collider = GetComponent<BoxCollider *>();
        }
        if (!m_bulletBehaviour) {
            m_bulletBehaviour = GetComponent<BulletBehaviour *>();
        }
        m_animator->PlayAnimation(0);
        m_collider->ChangeBox(Box{-7, -6, 7, 6} * PIXELS_ZOOM);
    }

    void Update(float dt) override {
        if (!m_bulletBehaviour->IsKilled() && !m_animator->IsPlaying()) {
            m_animator->PlayAnimation(1);
            m_collider->ChangeBox(Box{-16, -15, 16, 15} * PIXELS_ZOOM);
        }
    }

    void OnCollision(const CollideComponent &collider) override {
        auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
        if (bullet && !bullet->IsKilled()) {
            m_bulletBehaviour->Kill();
            bullet->Kill();
        }
    }
};

class GarmakilmaEyeShooting : public HiddenDestroyableShootingKillable {
private:
    ObjectPool<Bullet> m_bulletPool;
public:
    void Create(Level *level, GameObject *go, float downtime) {
        HiddenDestroyableShootingKillable::Create(level, go, downtime);

        m_bulletPool.Create(6);
        for (auto *bullet: m_bulletPool.pool) {
            bullet->Create();
            auto *renderer = new AnimationRenderer();
            renderer->Create(level, bullet, level->GetSpritesheet(SPRITESHEET_ENEMIES));
            renderer->AddAnimation({
                    103, 714, 0.1, 4,
                    14, 13, 7, 6,
                    "Bullet", AnimationRenderer::STOP_AND_LAST
            });
            renderer->AddAnimation({
                    103, 745, 0.1, 4,
                    32, 31, 16, 15,
                    "BulletBig", AnimationRenderer::DONT_STOP
            });
            renderer->AddAnimation({
                    92, 611, 0.15, 3,
                    30, 30, 15, 15,
                    "Kill", AnimationRenderer::BOUNCE_AND_STOP});
            auto *movement = new BulletStraightMovement();
            movement->Create(level, bullet);
            auto *behaviour = new GarmakilmaEyeBulletBehaviour();
            behaviour->Create(level, bullet);
            auto *box_collider = new BoxCollider();
            box_collider->Create(level, bullet,
                    0, 0, 0, 0, // Modified by the behaviour dynamically
                    PLAYER_COLLISION_LAYER, PERSP_PLAYER_BULLETS_COLLISION_LAYER);
            box_collider->SetListener(behaviour);

            bullet->AddComponent(behaviour);
            bullet->AddComponent(movement);
            bullet->AddComponent(renderer);
            bullet->AddComponent(box_collider);
            bullet->AddReceiver(level);

            bullet->onRemoval = GameObject::DO_NOT_DESTROY;
        }
    }

    void Init() override {
        HiddenDestroyableShootingKillable::Init();
        m_untilNextShoot = 0.1f;
    }

    void Fire() override {
        auto bullet = m_bulletPool.FirstAvailable();
        auto *closest = level->GetClosestPlayer(go->position);
        if (bullet && closest) {
            Vector2D dir = closest->position - go->position;
            bullet->Init(go->position, dir, 0.65f * BULLET_SPEED * PIXELS_ZOOM);
            level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
        }
    }

    void Destroy() override {
        HiddenDestroyableShootingKillable::Destroy();
        m_bulletPool.Destroy();
    }
};

class GarmakilmaEye : public GameObject {
public:
    void Create(Level *level, const Vector2D &start, const Vector2D &end) {
        GameObject::Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        renderer->AddAnimation({
                103, 651, 0.1, 3,
                25, 25, 12, 12,
                "Glowing", AnimationRenderer::BOUNCE
        });
        renderer->AddAnimation({
                103, 677, 0.1, 3,
                25, 25, 12, 12,
                "GlowingRed", AnimationRenderer::BOUNCE
        });
        auto *behaviour = new HiddenDestroyableBehaviour();
        behaviour->Create(level, this, 16, true, 2.f);
        auto *collider = new BoxCollider();
        collider->Create(level, this,
                -12 * PIXELS_ZOOM, -12 * PIXELS_ZOOM,
                25 * PIXELS_ZOOM, 25 * PIXELS_ZOOM,
                -1, PERSP_PLAYER_BULLETS_COLLISION_LAYER);
        auto *listener = new GarmakilmaBulletListener();
        listener->Create(level, this);
        collider->SetListener(listener);
        auto *movement = new BackAndForthMovement();
        movement->Create(level, this, start, end, 1.5f, 0.5f);
        auto *shooting = new GarmakilmaEyeShooting();
        shooting->Create(level, this, 2.f);

        AddComponent(behaviour);
        AddComponent(movement);
        AddComponent(listener);
        AddComponent(collider);
        AddComponent(shooting);
        AddComponent(renderer);
    }
};

class Garmakilma : public GameObject {
private:
    std::vector<GameObject *> m_cores;
    std::vector<GameObject *> m_canons;
    ObjectPool<Bullet> m_canonBulletPool;
    GarmakilmaEye *m_eye;
    bool m_eyeSpawned;
public:
    void Create(PerspectiveLevel *level, Vector2D position) {
        GameObject::Create();
        m_canonBulletPool.Create(6);
        for (auto *bullet: m_canonBulletPool.pool) {
            bullet->Create();
            auto *renderer = new AnimationRenderer();
            renderer->Create(level, bullet, level->GetSpritesheet(SPRITESHEET_ENEMIES));
            renderer->AddAnimation({
                    204, 67, 0.1, 1,
                    8, 8, 4, 4,
                    "Bullet", AnimationRenderer::STOP_AND_LAST
            });
            auto *behaviour = new BulletStraightMovement();
            behaviour->Create(level, bullet);
            auto *box_collider = new BoxCollider();
            box_collider->Create(level, bullet,
                    Box{-4, -4, 4, 4} * PIXELS_ZOOM,
                    PLAYER_COLLISION_LAYER, -1);
            bullet->AddComponent(behaviour);
            bullet->AddComponent(renderer);
            bullet->AddComponent(box_collider);
            bullet->AddReceiver(this);

            bullet->onRemoval = DO_NOT_DESTROY;
        }
        this->position = position;
        Vector2D core_positions[] = {{64,  55},
                                     {16,  103},
                                     {64,  103},
                                     {112, 103}};
        Vector2D canon_positions[] = {{16,  55},
                                      {112, 55}};

        for (auto &pos: core_positions) {
            auto *core = new GarmakilmaCore();
            core->Create(level);
            core->position = position + pos * PIXELS_ZOOM;
            core->AddReceiver(this);
            m_cores.push_back(core);
        }
        for (auto &pos: canon_positions) {
            auto *canon = new GarmakilmaCanon();
            canon->Create(level, &m_canonBulletPool);
            canon->position = position + pos * PIXELS_ZOOM;
            canon->AddReceiver(this);
            m_canons.push_back(canon);
        }
        m_eye = new GarmakilmaEye();
        m_eye->Create(level,
                position + Vector2D(20, 24) * PIXELS_ZOOM,
                position + Vector2D(108, 24) * PIXELS_ZOOM);
        m_eye->AddReceiver(this);
    }

    void Init() override {
        GameObject::Init();
        for (auto *core: m_cores) {
            core->Init();
        }
        for (auto *canon: m_canons) {
            canon->Init();
        }
        m_eyeSpawned = false;
    }

    void Destroy() override {
        GameObject::Destroy();
        for (auto *core: m_cores) {
            core->Destroy();
        }
        for (auto *canon: m_canons) {
            canon->Destroy();
        }
        m_canonBulletPool.Destroy();
    }

    void Update(float dt) override {
        GameObject::Update(dt);
        short alive_cores = 0;
        for (auto *core: m_cores) {
            if (core->IsEnabled()) {
                core->Update(dt);
                alive_cores++;
            }
        }
        for (auto *canon: m_canons) {
            if (canon->IsEnabled())
                canon->Update(dt);
        }
        if (alive_cores == 0) {
            if (m_eye->IsEnabled()) {
                m_eye->Update(dt);
            }
            if (!m_eyeSpawned) {
                m_eye->Init();
                m_eyeSpawned = true;
            }
        }
    }
};

#endif //CONTRA_GARMAKILMA_H
