//
// Created by david on 8/3/20.
//

#ifndef CONTRA_DARR_H
#define CONTRA_DARR_H

#include "../../../kernel/game_object.h"
#include "../../level/level.h"
#include "../../../components/render/SimpleRenderer.h"
#include "../../level/level_component.h"
#include "../../../kernel/box.h"
#include "../../level/perspective_const.h"


class DarrBehaviour : public LevelComponent, public CollideComponentListener, public Killable {
private:
    SimpleRenderer *m_renderer;
    BoxCollider *m_collider;
    constexpr static const Box s_sizes[] = {
            {2,  454, 12, 458},
            {14, 451, 30, 458},
            {32, 450, 54, 458},
            {56, 447, 80, 458}
    };
    Vector2D m_speed;
public:
    void Init(const Vector2D &speed) {
        Component::Init();
        m_speed = speed;
        if (!m_renderer) {
            m_renderer = GetComponent<SimpleRenderer *>();
        }
        if (!m_collider) {
            m_collider = GetComponent<BoxCollider *>();
        }
    }

    void Update(float dt) override {
        go->position = go->position + m_speed * dt;

        if (go->position.y > PERSP_PLAYER_Y * PIXELS_ZOOM) {
            go->Disable();
        } else if (go->position.y > (PERSP_PLAYER_Y - 10) * PIXELS_ZOOM) {
            std::set<CollideComponent *> colliding;
            m_collider->GetCurrentCollisions(&colliding, PLAYER_COLLISION_LAYER);
            Hittable *chosen = nullptr;
            for (auto collider: colliding) {
                auto *hittable = collider->GetGameObject()->GetComponent<Hittable *>();
                if (hittable && hittable->CanBeHit()) {
                    chosen = hittable;
                    break;
                }
            }
            if (chosen) {
                chosen->Hit();
                Kill();
            }
        } else {
            float factor = (go->position.y - PERSP_ENEMIES_Y * PIXELS_ZOOM)
                           / float((PERSP_PLAYER_Y - PERSP_ENEMIES_Y) * PIXELS_ZOOM);

            int size = fmin(4, fmax(0, floor(sqrtf(factor) * 4)));
            int width = s_sizes[size].width();
            int half_width = width / 2;
            int height = s_sizes[size].height();
            m_renderer->ChangeCoords(
                    s_sizes[size].top_left_x, s_sizes[size].top_left_y,
                    width, height, half_width, height
            );
            m_collider->ChangeBox(Box{-half_width, -height, half_width, 0} * PIXELS_ZOOM);
        }
    }

    void Kill() override {
        auto *explosion = new Explosion();
        explosion->Create(level, go->position);
        explosion->Init();
        level->AddGameObject(explosion, RENDERING_LAYER_ENEMIES);
        go->Disable();
    }

    void OnCollision(const CollideComponent &collider) override {
        auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
        if (bullet && !bullet->IsKilled() && PERSP_ENEMIES_Y * PIXELS_ZOOM - bullet->GetMinY() < 10 * PIXELS_ZOOM) {
            Kill();
            bullet->Kill();
        }
    }
};

class Darr : public GameObject {
private:
    DarrBehaviour *m_behaviour;
public:
    void Create(Level *level) {
        GameObject::Create();
        auto *render = new SimpleRenderer();
        render->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
        m_behaviour = new DarrBehaviour();
        m_behaviour->Create(level, this);
        auto *collider = new BoxCollider();
        collider->Create(level, this, 0, 0, 0, 0,
                -1, PERSP_PLAYER_BULLETS_COLLISION_LAYER);
        collider->SetListener(m_behaviour);

        AddComponent(m_behaviour);
        AddComponent(collider);
        AddComponent(render);
    }

    void Init(const Vector2D &pos, const Vector2D &speed) {
        GameObject::Init();
        position = pos;
        m_behaviour->Init(speed);
    }
};

#endif //CONTRA_DARR_H
