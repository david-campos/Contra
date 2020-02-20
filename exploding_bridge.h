//
// Created by david on 19/2/20.
//

#ifndef CONTRA_EXPLODING_BRIDGE_H
#define CONTRA_EXPLODING_BRIDGE_H

#include "component.h"
#include "level.h"
#include "game_object.h"
#include "Player.h"

class DestroyOnAnimationStop: public Component {
private:
    AnimationRenderer* m_renderer;
public:
    void Init() override {
        Component::Init();
        if (!m_renderer) {
            m_renderer = go->GetComponent<AnimationRenderer*>();
        }
        SDL_Log("DestroyOnAnimationStop::Init");
    }

    void Update(float dt) override {
        if (!m_renderer->IsPlaying()) {
            go->Disable();
            level->RemoveGameObject(go);
            SDL_Log("DestroyOnAnimationStop::Removing");
        }
    }
};

class BridgeExplosion: public GameObject {
public:
    void Create(Level* level) {
        GameObject::Create();
        AnimationRenderer::Animation animation = {
                92, 611, 0.15, 3,
                30, 30, 15, -5,
                "Dying", AnimationRenderer::BOUNCE_AND_STOP};
        auto* renderer = new AnimationRenderer();
        renderer->Create(level, this, level->GetEnemiesSpritesheet());
        renderer->AddAnimation(animation);
        renderer->Play();
        AddComponent(renderer);

        animation.anchor_x = 7;
        renderer = new AnimationRenderer();
        renderer->Create(level, this, level->GetEnemiesSpritesheet());
        renderer->AddAnimation(animation);
        renderer->Play();
        AddComponent(renderer);

        animation.anchor_x = 22;
        renderer = new AnimationRenderer();
        renderer->Create(level, this, level->GetEnemiesSpritesheet());
        renderer->AddAnimation(animation);
        renderer->Play();
        AddComponent(renderer);

        animation.anchor_x = 15;
        animation.anchor_y = 15;
        renderer = new AnimationRenderer();
        renderer->Create(level, this, level->GetEnemiesSpritesheet());
        renderer->AddAnimation(animation);
        renderer->Play();
        AddComponent(renderer);

        auto* self_destroy = new DestroyOnAnimationStop();
        self_destroy->Create(level, this);
        AddComponent(self_destroy);
    }
};

class ExplodingBridgeBehaviour : public Component {
private:
    float m_explodingTime;
    bool m_exploding;
    SimpleRenderer *m_renderer;
public:
    void Init() override {
        Component::Init();
        m_exploding = false;
        m_explodingTime = 0;
        if (!m_renderer) {
            m_renderer = go->GetComponent<SimpleRenderer *>();
        }
    }

    void Update(float dt) override {
        if (level->GetPlayer()->position.x > go->position.x) {
            m_exploding = true;
        }
        if (!m_exploding) return;
        m_explodingTime -= dt;
        if (m_explodingTime <= 0 && m_renderer->GetSrcY() < 124) {
            DeleteFloor();

            m_renderer->ChangeCoords(
                    0, m_renderer->GetSrcY() + m_renderer->GetHeight(),
                    m_renderer->GetWidth(), m_renderer->GetHeight(),
                    m_renderer->GetAnchorX(), m_renderer->GetAnchorY());

            auto* exp = new BridgeExplosion();
            exp->Create(level);
            exp->position = go->position + Vector2D(m_renderer->GetSrcY() * PIXELS_ZOOM, 0);
            exp->Init();
            level->AddGameObject(exp, RENDERING_LAYER_BULLETS);

            m_explodingTime = 0.85f;
        }
    }
private:
    void DeleteFloor() {
        auto level_floor = level->GetLevelFloor().lock();
        if (!level_floor) return;
        // Not a mistake to have height two times, it is an square of height*height
        level_floor->SetAir(
                floor(go->position.x / PIXELS_ZOOM + m_renderer->GetSrcY()),
                floor(go->position.y / PIXELS_ZOOM),
                m_renderer->GetHeight(), m_renderer->GetHeight());
    }
};

#endif //CONTRA_EXPLODING_BRIDGE_H
