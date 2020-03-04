//
// Created by david on 4/3/20.
//

#ifndef CONTRA_CORES_H
#define CONTRA_CORES_H

#include "../../kernel/game_object.h"
#include "../../components/render/AnimationRenderer.h"
#include "../level/level.h"
#include "explosion.h"

class CoreBehaviour : public LevelComponent, public CollideComponentListener {
private:
    int m_lives;
    int m_maxLives;
    BoxCollider *m_collider;
    AnimationRenderer *m_animator;
    enum State {
        STATE_CLOSED,
        STATE_OPENING_CLOSING,
        STATE_OPEN
    };
    State m_state;
    float m_stateTime;
    int m_animOpen, m_animGlowing;
public:
    void Create(Level *scene, GameObject *go, int lives) {
        LevelComponent::Create(scene, go);
        m_maxLives = m_lives = lives;
    }

    void Init() override {
        LevelComponent::Init();
        if (!m_collider) {
            m_collider = go->GetComponent<BoxCollider *>();
        }
        if (!m_animator) {
            m_animator = go->GetComponent<AnimationRenderer *>();
            m_animOpen = m_animator->FindAnimation("Open");
            m_animGlowing = m_animator->FindAnimation("Glowing");
        }
        m_lives = m_maxLives;
        m_state = STATE_OPEN;
    }

    void Update(float dt) override {
        switch (m_state) {
            case STATE_CLOSED:
            case STATE_OPEN: {
                m_stateTime += dt;
                if (m_stateTime > 2.f) {
                    m_stateTime = 0.f;
                    m_animator->PlayAnimation(m_animOpen, m_state == STATE_CLOSED);
                    m_state = STATE_OPENING_CLOSING;
                    m_collider->Disable();
                }
                break;
            }
            case STATE_OPENING_CLOSING: {
                if (!m_animator->IsPlaying()) {
                    m_state = m_animator->IsGoingBackwards() ? STATE_CLOSED : STATE_OPEN;
                    if (m_animator->IsGoingBackwards()) m_animator->CurrentAndPause(m_animOpen);
                    else {
                        m_animator->PlayAnimation(m_animGlowing);
                        m_collider->Enable();
                    }
                }
            }
        }
    }

    void OnCollision(const CollideComponent &collider) override {
        auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
        if (bullet && !bullet->IsKilled() && bullet->GetMinY() > m_collider->AbsoluteTopLeftY()) {
            bullet->Kill();
            if (--m_lives <= 0) {
                go->MarkToRemove();
                auto *explosion = new Explosion();
                explosion->Create(level, go->position);
                explosion->Init();
                explosion->AddReceiver(level);
                explosion->SendOnDestroy(SCREEN_CLEARED);
                level->AddGameObject(explosion, RENDERING_LAYER_ENEMIES);
            }
        }
    }
};

class WeakCore : public GameObject {
public:
    void Create(Level *level, Vector2D position) {
        GameObject::Create();
        this->position = position;

        auto *render = new AnimationRenderer();
        render->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        render->AddAnimation({
                1, 332, 0.2, 3,
                18, 18, 9, 9,
                "Glowing", AnimationRenderer::BOUNCE
        });
        render->AddAnimation({
                1, 314, 0.2, 2,
                18, 18, 9, 9,
                "Open", AnimationRenderer::STOP_AND_LAST
        });
        render->Play();
        auto *collider = new BoxCollider();
        collider->Create(level, this,
                -8 * PIXELS_ZOOM, -8 * PIXELS_ZOOM,
                16 * PIXELS_ZOOM, 10 * PIXELS_ZOOM, -1,
                NPCS_COLLISION_LAYER);

        auto *behaviour = new CoreBehaviour();
        behaviour->Create(level, this, 10);

        collider->SetListener(behaviour);

        AddComponent(render);
        AddComponent(behaviour);
        AddComponent(collider);
    }
};

#endif //CONTRA_CORES_H
