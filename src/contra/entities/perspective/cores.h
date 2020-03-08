//
// Created by david on 4/3/20.
//

#ifndef CONTRA_CORES_H
#define CONTRA_CORES_H

#include "../../../kernel/game_object.h"
#include "hidden_destroyable.h"

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

    std::random_device rd;
    std::mt19937 m_mt = std::mt19937(rd());
    std::uniform_real_distribution<float> m_random_dist = std::uniform_real_distribution<float>(0.f, 1.f);
public:
    void Create(Level *level, GameObject *go, float downtime) {
        LevelComponent::Create(level, go);
        m_shootDowntime = downtime;
    }

    void Init() override {
        if (!m_destroyableBehaviour) {
            m_destroyableBehaviour = GetComponent<HiddenDestroyableBehaviour *>();
        }
        m_untilNextShoot = m_shootDowntime * 0.8f + m_random_dist(m_mt) * m_shootDowntime * 0.2;
    }

    void Kill() override {
        m_destroyableBehaviour->Kill();
    }

    void Update(float dt) override {
        if (m_destroyableBehaviour->GetState() == HiddenDestroyableBehaviour::DEST_STATE_OPEN) {
            m_untilNextShoot -= dt;
            if (m_untilNextShoot < 0) {
                Fire();
                m_untilNextShoot = m_shootDowntime * 0.8f + m_random_dist(m_mt) * m_shootDowntime * 0.2;
            }
        }
    }

    void Fire() {
        Vector2D player_pos = level->GetClosestPlayer(go->position)->position;
        Vector2D dir = player_pos - Vector2D(0, 10 * PIXELS_ZOOM) - go->position;
        auto *bullet = level->GetEnemyBullets()->FirstAvailable();
        if (bullet) {
            bullet->Init(go->position, dir, 0.65f * BULLET_SPEED * PIXELS_ZOOM,
                    -9999, (PERSP_PLAYER_Y - 25) * PIXELS_ZOOM);
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
