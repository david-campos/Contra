//
// Created by david on 4/3/20.
//

#ifndef CONTRA_EXPLOSION_H
#define CONTRA_EXPLOSION_H

#include "../../components/render/AnimationRenderer.h"
#include "../../kernel/game_object.h"
#include "../../components/scene.h"
#include "../level/level.h"
#include "../../components/sound_effect.h"

class DestroyOnAnimationStop : public Component {
private:
    AnimationRenderer *m_renderer;
    Message m_onDestroyMessage;
    bool m_hasMessage = false;
public:
    void Init() {
        Component::Init();
        if (!m_renderer) {
            m_renderer = go->GetComponent<AnimationRenderer *>();
        }
    }

    void SendOnDestroy(Message message) {
        m_onDestroyMessage = message;
        m_hasMessage = true;
    }

    void Update(float dt) override {
        if (!m_renderer->IsPlaying()) {
            go->Disable();
            scene->RemoveGameObject(go);
            if (m_hasMessage) {
                go->Send(m_onDestroyMessage);
            }
        }
    }
};

class Explosion : public GameObject {
public:
    void Create(Level *level, const Vector2D &pos) {
        GameObject::Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        renderer->AddAnimation({
                92, 611, 0.15, 3,
                30, 30, 15, 15,
                "Explosion", AnimationRenderer::BOUNCE_AND_STOP});
        renderer->Play();
        auto *self_destroy = new DestroyOnAnimationStop();
        self_destroy->Create(level, this);
        auto *sound = new SoundEffectComponent();
        sound->Create(level, this, level->GetSound(SOUND_EXPLOSION));

        AddComponent(renderer);
        AddComponent(sound);
        AddComponent(self_destroy);

        position = pos;
    }

    void SendOnDestroy(Message message) {
        GetComponent<DestroyOnAnimationStop *>()->SendOnDestroy(message);
    }
};

#endif //CONTRA_EXPLOSION_H
