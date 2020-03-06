//
// Created by david on 4/3/20.
//

#ifndef CONTRA_CORES_H
#define CONTRA_CORES_H

#include "../../../kernel/game_object.h"
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
    int m_animOpen, m_animGlowing, m_animDead;
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
        }
        m_lives = m_maxLives;
        if (m_animOpen >= 0) {
            m_collider->Disable();
            m_state = DEST_STATE_CLOSED;
        } else {
            m_collider->Enable();
            m_state = DEST_STATE_OPEN;
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
                    if (m_animator->IsGoingBackwards()) m_animator->CurrentAndPause(m_animOpen);
                    else {
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

    void Kill() {
        m_lives = 1;
        Hit();
    }

    void Hit() override {
        if (--m_lives <= 0) {
            m_state = DEST_STATE_DEAD;
            m_animator->PlayAnimation(m_animDead);
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
        } else {
            level->GetSound(SOUND_ENEMY_HIT)->Play(1);
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
        render->AddAnimation({
                145, 314, 0.2, 3,
                18, 18, 9, 9,
                "Dead", AnimationRenderer::BOUNCE
        });
        render->Play();
        auto *collider = new BoxCollider();
        collider->Create(level, this,
                -8 * PIXELS_ZOOM, -8 * PIXELS_ZOOM,
                16 * PIXELS_ZOOM, 16 * PIXELS_ZOOM, NPCS_COLLISION_LAYER,
                -1);

        auto *behaviour = new HiddenDestroyableBehaviour();
        behaviour->Create(level, this, 8, true, 2.f);

        AddComponent(render);
        AddComponent(behaviour);
        AddComponent(collider);
    }
};

class StrongCore : public GameObject {
public:
    void Create(Level *level, Vector2D position) {
        GameObject::Create();
        this->position = position;

        auto *render = new AnimationRenderer();
        render->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        render->AddAnimation({
                55, 332, 0.2, 3,
                24, 24, 12, 12,
                "Glowing", AnimationRenderer::BOUNCE
        });
        render->AddAnimation({
                145, 314, 0.2, 3,
                18, 18, 9, 9,
                "Dead", AnimationRenderer::BOUNCE
        });
        render->Play();
        auto *collider = new BoxCollider();
        collider->Create(level, this,
                -11 * PIXELS_ZOOM, -11 * PIXELS_ZOOM,
                22 * PIXELS_ZOOM, 22 * PIXELS_ZOOM, NPCS_COLLISION_LAYER,
                -1);

        auto *behaviour = new HiddenDestroyableBehaviour();
        behaviour->Create(level, this, 16, true, 2.f);

        AddComponent(render);
        AddComponent(behaviour);
        AddComponent(collider);
    }
};

class CoreCannonBehaviour : public LevelComponent, public Killable {
private:
    HiddenDestroyableBehaviour *m_destroyableBehaviour;
    float m_shootDowntime, m_untilNextShoot;
public:
    void Create(Level *level, GameObject *go, float downtime) {
        LevelComponent::Create(level, go);
        m_shootDowntime = downtime;
    }

    void Init() override {
        if (!m_destroyableBehaviour) {
            m_destroyableBehaviour = GetComponent<HiddenDestroyableBehaviour *>();
        }
        m_untilNextShoot = m_shootDowntime;
    }

    void Kill() override {
        m_destroyableBehaviour->Kill();
    }

    void Update(float dt) override {
        if (m_destroyableBehaviour->GetState() == HiddenDestroyableBehaviour::DEST_STATE_OPEN) {
            m_untilNextShoot -= dt;
            if (m_untilNextShoot < 0) {
                Fire();
                m_untilNextShoot = m_shootDowntime;
            }
        }
    }

    void Fire() {
        Vector2D player_pos = level->GetClosestPlayer(go->position)->position;
        Vector2D dir = player_pos - Vector2D(0, 10 * PIXELS_ZOOM) - go->position;
        auto *bullet = level->GetEnemyBullets()->FirstAvailable();
        if (bullet) {
            bullet->Init(go->position, dir, 0.5 * BULLET_SPEED * PIXELS_ZOOM,
                    -9999, player_pos.y - 10 * PIXELS_ZOOM);
            level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
        }
    }
};

class CoreCannon : public GameObject {
public:
    void Create(Level *level, Vector2D position) {
        GameObject::Create();
        this->position = position;

        auto *render = new AnimationRenderer();
        render->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        render->AddAnimation({
                91, 314, 0.2, 3,
                18, 18, 9, 9,
                "Glowing", AnimationRenderer::BOUNCE
        });
        render->AddAnimation({
                1, 314, 0.2, 5,
                18, 18, 9, 9,
                "Open", AnimationRenderer::STOP_AND_LAST
        });
        render->AddAnimation({
                145, 314, 0.2, 3,
                18, 18, 9, 9,
                "Dead", AnimationRenderer::BOUNCE
        });
        render->Play();
        auto *collider = new BoxCollider();
        collider->Create(level, this,
                -8 * PIXELS_ZOOM, -8 * PIXELS_ZOOM,
                16 * PIXELS_ZOOM, 16 * PIXELS_ZOOM, NPCS_COLLISION_LAYER, -1);

        auto *destroyableBehaviour = new HiddenDestroyableBehaviour();
        destroyableBehaviour->Create(level, this, 4, false, 4.f);
        auto *firingBehaviour = new CoreCannonBehaviour();
        firingBehaviour->Create(level, this, 2.f);

        AddComponent(render);
        AddComponent(destroyableBehaviour);
        AddComponent(firingBehaviour);
        AddComponent(collider);
    }
};

#endif //CONTRA_CORES_H
