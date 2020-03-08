//
// Created by david on 8/3/20.
//

#ifndef CONTRA_GARMAKILMA_H
#define CONTRA_GARMAKILMA_H

#include "../../../kernel/game_object.h"
#include "hidden_destroyable.h"
#include "../../level/perspective_level.h"
#include "../../../components/collision/BoxCollider.h"

class GarmakilmaCore : public GameObject {
public:
    void Create(Level *level) {
        GameObject::Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        renderer->AddAnimation({
                4, 745, 0.1, 3,
                33, 30, 0, 0,
                "Glowing", AnimationRenderer::BOUNCE
        });
        renderer->Play();

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
                33, 30, 0, 0,
                "Glowing", AnimationRenderer::BOUNCE
        });
        renderer->AddAnimation({
                4, 682, 0.1, 3,
                33, 30, 0, 0,
                "Open", AnimationRenderer::STOP_AND_LAST
        });
        renderer->AddAnimation({
                4, 651, 0.1, 3,
                33, 30, 0, 0,
                "GlowingClosed", AnimationRenderer::BOUNCE
        });
        auto *behaviour = new HiddenDestroyableBehaviour();
        behaviour->Create(level, this, 8, true, 4.f);
        auto *collider = new BoxCollider();
        collider->Create(level, this, 0, 0, 32, 30,
                NPCS_COLLISION_LAYER, -1);
        AddComponent(collider);
        AddComponent(behaviour);
        AddComponent(renderer);
    }
};

class Garmakilma : public GameObject {
private:
    std::vector<GameObject *> m_cores;
    std::vector<GameObject *> m_canons;
    GameObject *m_garmakilma;
public:
    void Create(PerspectiveLevel *level, Vector2D position) {
        GameObject::Create();
        this->position = position;
        Vector2D core_positions[] = {{48, 40},
                                     {0,  88},
                                     {48, 88},
                                     {96, 88}};
        Vector2D canon_positions[] = {{0,  40},
                                      {96, 40}};

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
        for (auto *core: m_cores) {
            if (core->IsEnabled())
                core->Update(dt);
        }
        for (auto *canon: m_canons) {
            if (canon->IsEnabled())
                canon->Update(dt);
        }
    }
};

#endif //CONTRA_GARMAKILMA_H
