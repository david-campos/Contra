//
// Created by david on 9/2/20.
//

#ifndef CONTRA_WEAPONS_H
#define CONTRA_WEAPONS_H

#include "vector2D.h"
#include "bullets.h"

class Weapon {
protected:
    ObjectPool<Bullet> *m_bulletPool;
    std::set<GameObject *> **m_gameObjects;
public:
    explicit Weapon(ObjectPool<Bullet> *mBulletPool, std::set<GameObject *> **game_objects)
            : m_bulletPool(mBulletPool), m_gameObjects(game_objects) {}

    virtual ~Weapon() {}

    /**
     * Called each frame to check if we should fire or not
     * @param fireKey Whether the fire key is pressed or not in the current frame
     * @param dt Time elapsed since last frame
     * @return True if we should fire giving the current conditions
     */
    virtual bool ShouldFire(bool fireKey, float dt) = 0;

    /**
     * Fires the gun
     * @return True if the fire was successful, false if not
     */
    virtual bool Fire(const Vector2D &position, const Vector2D &direction) = 0;

    /**
     * Indicates if the weapon is automatic or not, so the animation of the player does not make weird stuff
     * @return
     */
    virtual bool IsAutomatic() = 0;
};

class DefaultWeapon : public Weapon {
private:
    float m_shootDowntime = 0;
    bool m_hasShot = false;
public:
    DefaultWeapon(ObjectPool<Bullet> *mBulletPool, std::set<GameObject *> **gameObjects) : Weapon(mBulletPool,
            gameObjects) {}

    bool ShouldFire(bool fireKey, float dt) override {
        m_shootDowntime -= dt;
        if (!fireKey) {
            m_hasShot = false;
        }
        return !m_hasShot && fireKey && m_shootDowntime <= 0;
    }

    bool Fire(const Vector2D &position, const Vector2D &direction) override {
        // Grab the bullet from the pool
        auto *bullet = m_bulletPool->FirstAvailable();
        if (bullet != nullptr) {
            bullet->Init(position, direction.normalise());
            m_gameObjects[RENDERING_LAYER_BULLETS]->insert(bullet);
            m_hasShot = true;
            m_shootDowntime = 0.2;
            return true;
        }
        return false;
    }

    bool IsAutomatic() override {
        return false;
    }
};

class MachineGun : public Weapon {
private:
    float m_shootDowntime = 0;
public:
    MachineGun(ObjectPool<Bullet> *mBulletPool, std::set<GameObject *> **gameObjects) : Weapon(mBulletPool,
            gameObjects) {}

    bool ShouldFire(bool fireKey, float dt) override {
        m_shootDowntime -= dt;
        return fireKey && m_shootDowntime <= 0;
    }

    bool Fire(const Vector2D &position, const Vector2D &direction) override {
        // Grab the bullet from the pool
        auto *bullet = m_bulletPool->FirstAvailable();
        if (bullet != nullptr) {
            bullet->Init(position, direction.normalise());
            m_gameObjects[RENDERING_LAYER_BULLETS]->insert(bullet);
            m_shootDowntime = 0.2;
            return true;
        }
        return false;
    }

    bool IsAutomatic() override {
        return true;
    }
};

#endif //CONTRA_WEAPONS_H
