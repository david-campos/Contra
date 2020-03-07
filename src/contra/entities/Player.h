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
#include "../level/perspective_level.h"

class Player : public GameObject {
public:
    void Create(Level *level, short index);
};

class PlayerControl : public LevelComponent, public CollideComponentListener, public Hittable {
public:
    virtual void Create(Level *level, GameObject *go, short index, const PlayerStats &stats);

    void Init() override;

    void Update(float dt) override;

    void PickUp(PickUpType type);

    void Kill();

    void Hit() override;

    bool CanBeHit() override;

    void Respawn();

    void OnCollision(const CollideComponent &collider) override;

    [[nodiscard]] short getRemainingLives() const { return m_remainingLives; }

    [[nodiscard]] short IsAlive() const { return !m_isDeath; }

    [[nodiscard]] bool IsOnFloor() const { return m_gravity->IsOnFloor(); }

    void SetBaseFloor(float floor) { m_gravity->SetBaseFloor(floor); }

    WeaponType GetWeaponType() { return m_currentWeapon->GetWeaponType(); }

    float GetBulletSpeedMultiplier() { return m_currentWeapon->GetBulletSpeedMultiplier(); }

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
            m_persCrawlAnim, m_persRunAnim, m_persFryingAnim,
            m_persDyingAnim, m_persForward;
    Box m_standingBox, m_standingBoxPerspective, m_crawlingBox, m_swimmingBox, m_jumpBox;
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

    virtual void AnimationUpdate(bool shooting, const AvancezLib::KeyStatus &keyStatus, Box **collider_box_out,
                                 float dt) = 0;

    virtual bool IsBlocked() { return false; }

    struct PlayerBoundaries {
        float min_x;
        float max_x;
    };

    virtual PlayerBoundaries GetPlayerMovementBoundaries() = 0;

    virtual void OnSpawn() = 0;
    virtual void VerticalMovementUpdate(const AvancezLib::KeyStatus &keyStatus, float dt) {}

    [[nodiscard]] virtual int PickDieAnimation() const { return m_dieAnim; }
};

class PlayerControlScrolling : public PlayerControl {
protected:
    PlayerBoundaries GetPlayerMovementBoundaries() override;

    void OnSpawn() override;

    void
    AnimationUpdate(bool shooting, const AvancezLib::KeyStatus &keyStatus, Box **collider_box_out, float dt) override;

    bool Fire(const AvancezLib::KeyStatus &keyStatus) override;
};

class PlayerControlPerspective : public PlayerControl {
public:
    void Create(Level *level, GameObject *go, short index, const PlayerStats &stats) override {
        PlayerControl::Create(level, go, index, stats);
        m_perspectiveLevel = dynamic_cast<PerspectiveLevel *>(level);
        if (!m_perspectiveLevel) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "PlayerControlPerspective::Create: the level is not an instance of PerspectiveLevel");
        }
    }

protected:
    PlayerBoundaries GetPlayerMovementBoundaries() override;

    bool Fire(const AvancezLib::KeyStatus &keyStatus) override;

    void OnSpawn() override;

    int PickDieAnimation() const override {
        return m_persDyingAnim;
    }

    void
    AnimationUpdate(bool shooting, const AvancezLib::KeyStatus &keyStatus, Box **collider_box_out, float dt) override;

    void VerticalMovementUpdate(const AvancezLib::KeyStatus &keyStatus, float dt) override;

    bool IsBlocked() override;

private:
    float m_fryingFor;
    PerspectiveLevel *m_perspectiveLevel;
};

#endif //CONTRA_PLAYER_H
