//
// Created by david on 19/2/20.
//

#ifndef CONTRA_EXPLODING_BRIDGE_H
#define CONTRA_EXPLODING_BRIDGE_H

#include "component.h"
#include "level.h"
#include "game_object.h"
#include "Player.h"

class DestroyOnAnimationStop : public Component {
private:
    AnimationRenderer *m_renderer;
public:
    void Init() override {
        Component::Init();
        if (!m_renderer) {
            m_renderer = go->GetComponent<AnimationRenderer *>();
        }
    }

    void Update(float dt) override {
        if (!m_renderer->IsPlaying()) {
            go->Disable();
            scene->RemoveGameObject(go);
        }
    }
};

class BridgeExplosion : public GameObject {
public:
    void Create(Level *level) {
        GameObject::Create();
        AnimationRenderer::Animation animation = {
                92, 611, 0.15, 3,
                30, 30, 15, -5,
                "Dying", AnimationRenderer::BOUNCE_AND_STOP};
        auto *renderer = new AnimationRenderer();
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

        auto *self_destroy = new DestroyOnAnimationStop();
        self_destroy->Create(level, this);
        AddComponent(self_destroy);
    }
};

class ExplodingBridgeBehaviour : public LevelComponent {
private:
    float m_explodingTime;
    bool m_exploding;
    AnimationRenderer *m_renderer;
    int m_animState0, current;
public:
    void Init() override {
        Component::Init();
        m_exploding = false;
        m_explodingTime = 0;
        if (!m_renderer) {
            m_renderer = go->GetComponent<AnimationRenderer *>();
            m_animState0 = m_renderer->FindAnimation("BridgeState0");
        }
        m_renderer->PlayAnimation(current = m_animState0, true);
    }

    void Update(float dt) override {
        if (level->GetClosestPlayer(go->position)->position.x > go->position.x) {
            m_exploding = true;
        }
        if (!m_exploding) return;
        m_explodingTime -= dt;
        if (m_explodingTime <= 0 && current < m_animState0 + 4) {
            DeleteFloor();
            current++;
            m_renderer->PlayAnimation(current);

            auto *exp = new BridgeExplosion();
            exp->Create(level);
            exp->position = go->position + Vector2D(current * m_renderer->GetCurrentAnimation().frame_h * PIXELS_ZOOM, 0);
            exp->Init();
            level->AddGameObject(exp, RENDERING_LAYER_BULLETS);

            level->GetSound(SOUND_EXPLOSION)->Play(1);

            m_explodingTime = 0.85f;
        }
    }

private:
    void DeleteFloor() {
        auto level_floor = level->GetLevelFloor().lock();
        if (!level_floor) return;
        int height = m_renderer->GetCurrentAnimation().frame_h;
        // Not a mistake to have height two times, it is an square of height*height
        level_floor->SetAir(
                floor(go->position.x / PIXELS_ZOOM + current * height),
                floor(go->position.y / PIXELS_ZOOM),
                height, height);
    }
};

#endif //CONTRA_EXPLODING_BRIDGE_H
