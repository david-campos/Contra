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
public:
    void Init() override {
        Component::Init();
        if (!m_behaviour) {
            m_behaviour = GetComponent<Hittable *>();
        }
    }

    void OnCollision(const CollideComponent &collider) override {
        auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
        if (bullet && !bullet->IsKilled() && m_behaviour) {
            m_behaviour->Hit();
            bullet->Kill();
        }
    }

    void Update(float dt) override {}
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

class GarmakilmaCanon : public GameObject {
public:
    void Create(Level *level) {
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

class GarmakilmaEye : public GameObject {
public:
    void Create(Level *level) {
        GameObject::Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        renderer->AddAnimation({
                103, 651, 0.1, 3,
                25, 25, 12, 12,
                "Glowing", AnimationRenderer::BOUNCE
        });
        auto *behaviour = new HiddenDestroyableBehaviour();
        behaviour->Create(level, this, 8, true, 2.f);
        auto *collider = new BoxCollider();
        collider->Create(level, this,
                -12 * PIXELS_ZOOM, -12 * PIXELS_ZOOM,
                25 * PIXELS_ZOOM, 25 * PIXELS_ZOOM,
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

class Garmakilma : public GameObject {
private:
    std::vector<GameObject *> m_cores;
    std::vector<GameObject *> m_canons;
    GarmakilmaEye *m_eye;
public:
    void Create(PerspectiveLevel *level, Vector2D position) {
        GameObject::Create();
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
            canon->Create(level);
            canon->position = position + pos * PIXELS_ZOOM;
            canon->AddReceiver(this);
            m_canons.push_back(canon);
        }
        m_eye = new GarmakilmaEye();
        m_eye->Create(level);
        m_eye->position = position + Vector2D(65, 24) * PIXELS_ZOOM;
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
    }

    void Destroy() override {
        GameObject::Destroy();
        for (auto *core: m_cores) {
            core->Destroy();
        }
        for (auto *canon: m_canons) {
            canon->Destroy();
        }
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
            if (!m_eye->IsEnabled())
                m_eye->Init();
            else
                m_eye->Update(dt);
        }
    }
};

#endif //CONTRA_GARMAKILMA_H
