//
// Created by david on 4/2/20.
//

#ifndef CONTRA_PLAYER_H
#define CONTRA_PLAYER_H

#define PLAYER_SPEED 55
#define PLAYER_JUMP 200

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
    void Create(AvancezLib *engine,std::set<GameObject *> **game_objects,
                const std::shared_ptr<Sprite> &spritesheet, const std::weak_ptr<Floor> &floor, float *camera_x,
                ObjectPool<Bullet> *default_bullets, ObjectPool<Bullet> *fire_bullets,
                ObjectPool<Bullet> *machine_gun_bullets,  ObjectPool<Bullet> *spread_bullets,
                ObjectPool<Bullet> *laser_bullets, Grid* grid, int player_layer);
};

class PlayerControl : public Component, public CollideComponentListener {
public:
    void Init() override;

    void Update(float dt) override;

    void Create(AvancezLib *engine, GameObject *go,std::set<GameObject *> **game_objects,
                float *camera_x,
                ObjectPool<Bullet> *default_bullets, ObjectPool<Bullet> *fire_bullets,
                ObjectPool<Bullet> *machine_gun_bullets,  ObjectPool<Bullet> *spread_bullets,
                ObjectPool<Bullet> *laser_bullets) {
        Component::Create(engine, go, game_objects);
        m_cameraX = camera_x;
        m_defaultBullets = default_bullets;
        m_fireBullets = fire_bullets;
        m_machineGunBullets = machine_gun_bullets;
        m_spreadBullets = spread_bullets;
        m_laserBullets = laser_bullets;
    }

    void PickUp(PickUpType type);

    void Kill();

    void Respawn();

    void OnCollision(const CollideComponent &collider) override;

    [[nodiscard]] short getRemainingLives() const { return m_remainingLives; }
    [[nodiscard]] short IsAlive() const { return !m_isDeath; }

private:
    AnimationRenderer *m_animator;
    BoxCollider *m_collider;
    ObjectPool<Bullet> *m_defaultBullets, *m_fireBullets,
            *m_machineGunBullets, *m_spreadBullets, *m_laserBullets;
    AvancezLib::KeyStatus m_previousKeyStatus;
    Gravity *m_gravity;
    float *m_cameraX;
    bool m_hasInertia;
    bool m_godMode;
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
};

#endif //CONTRA_PLAYER_H
