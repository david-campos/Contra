//
// Created by david on 6/3/20.
//

#ifndef CONTRA_EXPLODING_PILL_H
#define CONTRA_EXPLODING_PILL_H

#include "../../../kernel/game_object.h"
#include "../../../components/render/AnimationRenderer.h"
#include "../explosion.h"
#include "../../level/level_component.h"
#include "../../components/Gravity.h"
#include "../../level/perspective_const.h"

class ExplodingPillBehaviour : public LevelComponent {
private:
    AnimationRenderer *m_animator;
    Gravity *m_gravity;
public:
    void Init() override {
        LevelComponent::Init();
        if (!m_animator) {
            m_animator = GetComponent<AnimationRenderer *>();
        }
        if (!m_gravity) {
            m_gravity = GetComponent<Gravity *>();
        }
        m_animator->PlayAnimation(0);
    }

    void Update(float dt) override {
        if (!m_animator->IsPlaying()) m_animator->PlayAnimation(1);
        if (m_gravity->IsOnFloor()) {
            auto *explosion = new Explosion();
            explosion->Create(level, go->position, false);
            explosion->Init();
            level->AddGameObject(explosion, RENDERING_LAYER_BULLETS);

            for (auto *player: level->GetPlayerControls()) {
                if (player->GetGameObject()->position.distance(go->position) < 25 * PIXELS_ZOOM
                    && player->CanBeHit()) {
                    player->Hit();
                }
            }

            go->MarkToRemove();
        }
    }
};

class ExplodingPill : public GameObject {
public:
    void Create(Level *level) {
        GameObject::Create();
        auto *render = new AnimationRenderer();
        render->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        render->AddAnimation({
                125, 445, 0.1, 9,
                13, 13, 6, 7,
                "Small", AnimationRenderer::STOP_AND_LAST
        });
        render->AddAnimation({
                112, 458, 0.1, 8,
                13, 13, 6, 7,
                "Big", AnimationRenderer::DONT_STOP
        });
        auto *behaviour = new ExplodingPillBehaviour();
        behaviour->Create(level, this);
        auto *gravity = new Gravity();
        gravity->Create(level, this);
        gravity->SetAcceleration(300 * PIXELS_ZOOM);
        gravity->SetBaseFloor(PERSP_PLAYER_Y * PIXELS_ZOOM);
        gravity->SetVelocity(-PLAYER_JUMP * PIXELS_ZOOM);

        AddComponent(gravity);
        AddComponent(behaviour);
        AddComponent(render);
    }

    void Init(const Vector2D &pos) {
        position = pos;
        GameObject::Init();
    }
};

#endif //CONTRA_EXPLODING_PILL_H
