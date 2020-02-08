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
                float *camera_x, const Vector2D &pos, Player* player, ObjectPool<Bullet>* bullet_pool);
};

class TankBehaviour: public Component {
private:
    enum TankState {
        HIDDEN,
        SHOWING,
        SHOWN
    };
    TankState m_state;
    int m_dir;
    float m_currentDirTime;
    Player* m_player;
    AnimationRenderer* m_animator;
    int animHidden, animShowing, animDirsFirst;
    ObjectPool<Bullet>* m_bulletPool;
public:
    void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Player* player, ObjectPool<Bullet>* bullet_pool);
    void Init() {
        m_animator = go->GetComponent<AnimationRenderer*>();
        animHidden = m_animator->FindAnimation("Closed");
        animShowing = m_animator->FindAnimation("Opening");
        animDirsFirst = m_animator->FindAnimation("Dir0");
        m_state = HIDDEN;
    }
    void Update(float dt) override;
private:
    void Fire();
    [[nodiscard]] int DirToInt(const Vector2D& dir) const;
};

#endif //CONTRA_TANK_H
