//
// Created by david on 8/3/20.
//

#ifndef CONTRA_HIDDEN_DESTROYABLE_H
#define CONTRA_HIDDEN_DESTROYABLE_H

#include "../../../components/render/AnimationRenderer.h"
#include "../../level/level.h"
#include "../explosion.h"

class HiddenDestroyableBehaviour : public LevelComponent, public Hittable {
public:
    enum State {
        DEST_STATE_CLOSED,
        DEST_STATE_OPENING_CLOSING,
        DEST_STATE_OPEN,
        DEST_STATE_DEAD
    };
private:
    int m_lives;
    int m_maxLives;
    BoxCollider *m_collider;
    AnimationRenderer *m_animator;
    State m_state;
    float m_stateTime;
    int m_animOpen, m_animGlowing, m_animGlowingClosed, m_animDead;
    float m_timeOpen;
    bool m_doesClearScreen;
public:
    void Create(Level *scene, GameObject *go, int lives, bool does_clear_screen, float time_open) {
        LevelComponent::Create(scene, go);
        m_maxLives = m_lives = lives;
        m_timeOpen = time_open;
        m_doesClearScreen = does_clear_screen;
    }

    [[nodiscard]] State GetState() const {
        return m_state;
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
            m_animDead = m_animator->FindAnimation("Dead");
            m_animGlowingClosed = m_animator->FindAnimation("GlowingClosed");
        }
        m_lives = m_maxLives;
        if (m_animOpen >= 0) {
            m_collider->Disable();
            m_state = DEST_STATE_CLOSED;
            if (m_animGlowingClosed >= 0) {
                m_animator->PlayAnimation(m_animGlowingClosed);
            } else {
                m_animator->CurrentAndPause(m_animOpen);
            }
        } else {
            m_collider->Enable();
            m_state = DEST_STATE_OPEN;
            m_animator->PlayAnimation(m_animGlowing);
        }
    }

    void Update(float dt) override {
        switch (m_state) {
            case DEST_STATE_CLOSED:
            case DEST_STATE_OPEN: {
                if (m_animOpen < 0) break;

                m_stateTime += dt;
                if (m_stateTime > (m_state == DEST_STATE_OPEN ? m_timeOpen : 2.f)) {
                    m_stateTime = 0.f;
                    m_animator->PlayAnimation(m_animOpen, m_state == DEST_STATE_CLOSED);
                    m_state = DEST_STATE_OPENING_CLOSING;
                    m_collider->Disable();
                }
                break;
            }
            case DEST_STATE_OPENING_CLOSING: {
                if (!m_animator->IsPlaying()) {
                    m_state = m_animator->IsGoingBackwards() ? DEST_STATE_CLOSED : DEST_STATE_OPEN;
                    if (m_animator->IsGoingBackwards()) {
                        if (m_animGlowingClosed >= 0) m_animator->PlayAnimation(m_animGlowingClosed);
                        else m_animator->CurrentAndPause(m_animOpen);
                    } else {
                        m_animator->PlayAnimation(m_animGlowing);
                        m_collider->Enable();
                    }
                }
                break;
            }
            case DEST_STATE_DEAD:
                break;
        }
    }

    bool CanBeHit() override {
        return m_lives > 0;
    }

    bool HitLast() override {
        return true; // Give preference to enemies in front and so
    }

    void Kill() {
        if (m_lives > 0) {
            m_lives = 1;
            Hit();
        }
    }

    void Hit() override {
        if (--m_lives <= 0) {
            m_state = DEST_STATE_DEAD;
            m_collider->Disable();
            auto *explosion = new Explosion();
            explosion->Create(level, go->position);
            explosion->Init();
            explosion->AddReceiver(level);
            if (m_doesClearScreen) {
                auto *persLevel = dynamic_cast<PerspectiveLevel *>(level);
                if (persLevel) {
                    persLevel->KillScreen();
                }
                explosion->SendOnDestroy(SCREEN_CLEARED);
            }
            level->AddGameObject(explosion, RENDERING_LAYER_BULLETS);
            if (m_animDead >= 0) {
                m_animator->PlayAnimation(m_animDead);
            } else {
                go->Disable();
            }
        } else {
            level->GetSound(SOUND_ENEMY_HIT)->Play(1);
        }
    }
};

#endif //CONTRA_HIDDEN_DESTROYABLE_H
