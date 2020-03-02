//
// Created by david on 4/2/20.
//

#ifndef CONTRA_PLAYER_H
#define CONTRA_PLAYER_H

#include <utility>

#include "component.h"
#include "game_object.h"
#include "AnimationRenderer.h"
#include "floor.h"
#include "Gravity.h"
#include "bullets.h"
#include "weapons.h"
#include "pickups.h"

class Player : public GameObject {
public:
    void Create(Level* level, short index, PlayerStats playerStats);
};

class PlayerControl : public LevelComponent, public CollideComponentListener {
public:
    void Create(Level* level, GameObject* go, short index, int lives, Weapon* weapon);

    void Init() override;

    void Update(float dt) override;

    void PickUp(PickUpType type);

    void Kill();

    void Respawn();

    void OnCollision(const CollideComponent &collider) override;

    [[nodiscard]] short getRemainingLives() const { return m_remainingLives; }
    [[nodiscard]] short IsAlive() const { return !m_isDeath; }

private:
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
            m_swimShootUpAnim, m_fallAnim;
    Box m_standingBox, m_crawlingBox, m_swimmingBox, m_jumpBox;
    bool m_diving;
    std::unique_ptr<Weapon> m_currentWeapon;

    bool Fire(const AvancezLib::KeyStatus &keyStatus);

    /**
     * Normalises the key status depending on the player index so
     * all players can check the key status as if they were the
     * player 1
     * @param status
     */
    void NormaliseKeyStatus(AvancezLib::KeyStatus &status);
};

#endif //CONTRA_PLAYER_H
