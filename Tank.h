//
// Created by david on 8/2/20.
//

#ifndef CONTRA_TANK_H
#define CONTRA_TANK_H

#include <memory>

#include "game_object.h"
#include "avancezlib.h"
#include "AnimationRenderer.h"
#include "Player.h"

class Tank: public GameObject {
public:
    void Create(AvancezLib *engine, std::set<GameObject *> *game_objects,
                const std::shared_ptr<Sprite> &enemies_spritesheet,
                float *camera_x, const Vector2D &pos, Player* player, ObjectPool<Bullet>* bullet_pool,
                Grid* grid, int layer);
};

class TankBehaviour: public Component, public CollideComponentListener {
private:
    enum TankState {
        HIDDEN,
        SHOWING,
        SHOWN
    };
    TankState m_state;
    int m_dir;
    float m_currentDirTime;
    PlayerControl* m_player;
    AnimationRenderer* m_animator;
    int animHidden, animShowing, animDirsFirst, animDie;
    ObjectPool<Bullet>* m_bulletPool;
    int m_life;
public:
    void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Player* player, ObjectPool<Bullet>* bullet_pool);
    void Init() {
        m_animator = go->GetComponent<AnimationRenderer*>();
        animHidden = m_animator->FindAnimation("Closed");
        animShowing = m_animator->FindAnimation("Opening");
        animDirsFirst = m_animator->FindAnimation("Dir0");
        animDie = m_animator->FindAnimation("Dying");
        m_life = 8;
        m_state = HIDDEN;
    }

    void OnCollision(const CollideComponent &collider) override;
    void Update(float dt) override;
private:
    void Fire();
    [[nodiscard]] int DirToInt(const Vector2D& dir) const;
};

#endif //CONTRA_TANK_H
