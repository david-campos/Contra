//
// Created by david on 4/2/20.
//

#ifndef CONTRA_PLAYER_H
#define CONTRA_PLAYER_H

#include <utility>
#include "../../kernel/game_object.h"
#include "../level/level_component.h"
#include "weapons.h"
#include "../../components/collision/BoxCollider.h"
#include "../components/Gravity.h"

class Player : public GameObject {
public:
    void Create(Level *level, short index);
};

class PlayerControl : public LevelComponent, public CollideComponentListener {
public:
    void Create(Level *level, GameObject *go, short index, int lives, Weapon *weapon);

    void Init() override;

    void Update(float dt) final;

    void PickUp(PickUpType type);

    void Kill();

    void Respawn();

    void OnCollision(const CollideComponent &collider) override;

    [[nodiscard]] short getRemainingLives() const { return m_remainingLives; }

    [[nodiscard]] short IsAlive() const { return !m_isDeath; }

protected:
    AnimationRenderer *m_animator;
    BoxCollider *m_collider;
    AvancezLib::KeyStatus m_previousKeyStatus;
    Gravity *m_gravity;
    bool m_hasInertia;
    bool m_godMode;
    short m_index;
    float m_waitDead, m_invincibleTime;
    bool m_isDeath;
    bool m_facingRight;
    bool m_wasInWater;
    short m_remainingLives;
    int m_idleAnim, m_upAnim, m_crawlAnim,
            m_runAnim, m_runUpAnim, m_runDownAnim,
            m_jumpAnim, m_dieAnim, m_runShootAnim,
            m_splashAnim, m_swimAnim, m_diveAnim,
            m_swimShootAnim, m_swimShootDiagonalAnim,
            m_swimShootUpAnim, m_fallAnim, m_persIdleAnim,
            m_persCrawlAnim, m_persRunAnim;
    Box m_standingBox, m_crawlingBox, m_swimmingBox, m_jumpBox;
    bool m_diving;
    std::unique_ptr<Weapon> m_currentWeapon;

    virtual bool Fire(const AvancezLib::KeyStatus &keyStatus) = 0;

    /**
     * Normalises the key status depending on the player index so
     * all players can check the key status as if they were the
     * player 1
     * @param status
     */
    void NormaliseKeyStatus(AvancezLib::KeyStatus &status);

    virtual void AnimationUpdate(bool shooting, const AvancezLib::KeyStatus &keyStatus, Box **collider_box_out) = 0;

    struct PlayerBoundaries {
        float min_x;
        float max_x;
    };

    virtual PlayerBoundaries GetPlayerMovementBoundaries() = 0;
};

class PlayerControlScrolling : public PlayerControl {
    void AnimationUpdate(bool shooting, const AvancezLib::KeyStatus &keyStatus, Box **collider_box_out) override;

protected:
    PlayerBoundaries GetPlayerMovementBoundaries() override;

    bool Fire(const AvancezLib::KeyStatus &keyStatus) override;
};

class PlayerControlPerspective : public PlayerControl {
public:
    void AnimationUpdate(bool shooting, const AvancezLib::KeyStatus &keyStatus, Box **collider_box_out) override;

protected:
    PlayerBoundaries GetPlayerMovementBoundaries() override;

    bool Fire(const AvancezLib::KeyStatus &keyStatus) override;
};

#endif //CONTRA_PLAYER_H
