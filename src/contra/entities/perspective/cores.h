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
        auto *firingBehaviour = new HiddenDestroyableShootingKillable();
        firingBehaviour->Create(level, this, 2.f);

        AddComponent(render);
        AddComponent(destroyableBehaviour);
        AddComponent(firingBehaviour);
        AddComponent(collider);
    }
};

#endif //CONTRA_CORES_H
