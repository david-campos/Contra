//
// Created by david on 8/2/20.
//

#ifndef CONTRA_CANONS_H
#define CONTRA_CANONS_H

#include <memory>

#include "game_object.h"
#include "avancezlib.h"
#include "AnimationRenderer.h"
#include "Player.h"

class RotatingCanon : public GameObject {
public:
    void Create(AvancezLib *engine, std::set<GameObject *> *game_objects,
                const std::shared_ptr<Sprite> &enemies_spritesheet,
                float *camera_x, const Vector2D &pos, Player *player, ObjectPool<Bullet> *bullet_pool,
                Grid *grid, int layer);
};

class Gulcan : public GameObject {
public:
    void Create(AvancezLib *engine, std::set<GameObject *> *game_objects,
                const std::shared_ptr<Sprite> &enemies_spritesheet,
                float *camera_x, const Vector2D &pos, Player *player, ObjectPool<Bullet> *bullet_pool,
                Grid *grid, int layer);
};

class CanonBehaviour : public Component, public CollideComponentListener {
protected:
    enum CanonState {
        HIDDEN,
        SHOWING,
        SHOWN
    };
    CanonState m_state;
    int m_dir, m_minDir, m_maxDir;
    float m_currentDirTime, m_fireCooldown;
    PlayerControl *m_player;
    AnimationRenderer *m_animator;
    int animHidden, animShowing, animDirsFirst, animDie;
    ObjectPool<Bullet> *m_bulletPool;
    int m_life;

    [[nodiscard]] Vector2D GetPlayerDir() const {
        return m_player->GetGameObject()->position - Vector2D(0, 18) - go->position; // Subtract 18 bc position is the feet
    }

    [[nodiscard]] int DirToInt(const Vector2D &dir) const {
        return (12 - // Counter-clockwise, preserving 0
                int(fmod( // Between 0 and 3.1416
                        atan2(-dir.y, dir.x) +
                        6.2832 /*+ 0.2618*/, // we used to add 15º to correct, but the original game does not do it! XD
                        6.2832) / 0.5236)) % 12;  //0.5236rad = 30deg
    }

    void Fire();
public:
    void Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Player *player,
                ObjectPool<Bullet> *bullet_pool, int min_dir, int max_dir);
    void Init() {
        m_animator = go->GetComponent<AnimationRenderer *>();
        animHidden = m_animator->FindAnimation("Closed");
        animShowing = m_animator->FindAnimation("Opening");
        animDirsFirst = m_animator->FindAnimation("Dir0");
        animDie = m_animator->FindAnimation("Dying");
        m_life = 8;
        m_fireCooldown = 0;
        m_state = HIDDEN;
    }
    void Update(float dt) final;
    virtual void UpdateHidden(const Vector2D& player_dir, float dt);
    void UpdateShowing(const Vector2D& player_dir, float dt);
    void UpdateShown(const Vector2D& player_dir, float dt);
    void OnCollision(const CollideComponent &collider) override;
};

class GulcanBehaviour: public CanonBehaviour {
public:
    void UpdateHidden(const Vector2D &player_dir, float dt) override;
};

#endif //CONTRA_CANONS_H
