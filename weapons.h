//
// Created by david on 9/2/20.
//

#ifndef CONTRA_WEAPONS_H
#define CONTRA_WEAPONS_H

#include "level.h"
#include "consts.h"

class Weapon {
protected:
    Level *m_level;
    float m_bulletSpeedMultiplier = 1.f;
public:
    explicit Weapon(Level *level) : m_level(level) {}

    virtual ~Weapon() {}

    float GetBulletSpeedMultiplier() const {
        return m_bulletSpeedMultiplier;
    }

    void SetBulletSpeedMultiplier(float bulletSpeedMultiplier) {
        m_bulletSpeedMultiplier = bulletSpeedMultiplier;
    }

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
protected:
    bool m_hasShot = false;
    float m_shootDowntime = 0;
    int m_bulletSpeed = BULLET_SPEED;
public:
    DefaultWeapon(Level *level) : Weapon(level) {}

    bool ShouldFire(bool fireKey, float dt) override {
        m_shootDowntime -= dt;
        if (!fireKey) {
            m_hasShot = false;
        }
        return !m_hasShot && fireKey && m_shootDowntime <= 0;
    }

    bool Fire(const Vector2D &position, const Vector2D &direction) override {
        // Grab the bullet from the pool
        auto *bullet = GetBulletPool()->FirstAvailable();
        if (bullet != nullptr) {
            bullet->Init(position, direction.normalise(), m_bulletSpeedMultiplier * m_bulletSpeed * PIXELS_ZOOM);
            m_level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
            m_hasShot = true;
            m_shootDowntime = 0.15;
            return true;
        }
        return false;
    }

    virtual ObjectPool<Bullet> *GetBulletPool() const {
        return m_level->GetDefaultBullets();
    }

    bool IsAutomatic() override {
        return false;
    }
};

class FireGun : public DefaultWeapon {
public:
    FireGun(Level *level) : DefaultWeapon(level) {
        m_bulletSpeed = FIRE_BULLET_SPEED;
    }

    ObjectPool<Bullet> *GetBulletPool() const override {
        return m_level->GetFireBullets();
    }
};

class LaserGun : public Weapon {
protected:
    bool m_hasShot = false;
    float m_shootDowntime = 0;
    int m_nextBullet;
    Vector2D m_position, m_direction;
public:
    LaserGun(Level *level) : Weapon(level) {}

    bool IsAutomatic() override {
        return false;
    }

    bool ShouldFire(bool fireKey, float dt) override {
        if (!fireKey) {
            m_hasShot = false;
        }
        if (m_nextBullet < m_level->GetLaserBullets()->pool.size()) {
            m_shootDowntime -= dt;
            if (m_shootDowntime <= 0) {
                ResetNext();
            }
            return false;
        }
        return fireKey && !m_hasShot;
    }

    bool Fire(const Vector2D &position, const Vector2D &direction) override {
        // All bullets are fired and they reset
        m_nextBullet = 0;
        m_position = position;
        m_direction = direction;
        m_hasShot = true;
        ResetNext();
        return true;
    }

    void ResetNext() {
        auto *bullet = m_level->GetLaserBullets()->pool[m_nextBullet];
        bullet->Disable();
        m_level->RemoveImmediately(bullet, RENDERING_LAYER_BULLETS);
        bullet->Init(m_position, m_direction.normalise(), BULLET_SPEED * PIXELS_ZOOM);
        m_level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
        m_nextBullet++;
        m_shootDowntime = 0.02;
    }
};

class MachineGun : public Weapon {
private:
    float m_shootDowntime = 0;
public:
    MachineGun(Level *level) : Weapon(level) {}

    bool ShouldFire(bool fireKey, float dt) override {
        m_shootDowntime -= dt;
        return fireKey && m_shootDowntime <= 0;
    }

    bool Fire(const Vector2D &position, const Vector2D &direction) override {
        // Grab the bullet from the pool
        auto *bullet = m_level->GetMachineGunBullets()->FirstAvailable();
        if (bullet != nullptr) {
            bullet->Init(position, direction.normalise(), m_bulletSpeedMultiplier * BULLET_SPEED * PIXELS_ZOOM);
            m_level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
            m_shootDowntime = 0.15;
            return true;
        }
        return false;
    }

    bool IsAutomatic() override {
        return true;
    }
};

class SpreadGun : public Weapon {
private:
    float m_shootDowntime = 0;
    bool m_hasShot = false;
public:
    SpreadGun(Level *level) : Weapon(level) {}

    bool ShouldFire(bool fireKey, float dt) override {
        m_shootDowntime -= dt;
        if (!fireKey) {
            m_hasShot = false;
        }
        return !m_hasShot && fireKey && m_shootDowntime <= 0;
    }

    bool Fire(const Vector2D &position, const Vector2D &direction) override {
        m_hasShot = false;
        std::vector<Bullet *> bullets = m_level->GetSpreadBullets()->FirstAvailableN(5);
        if (bullets.size() >= 3) {
            for (int i = 0; i < bullets.size(); i++) {
                float angle = 0.2618f - i * 0.5236f / (bullets.size() - 1); // 15deg - (30deg / (num bullets + 1)) * i
                bullets[i]->Init(position,
                        direction.rotate(angle).normalise(),
                        m_bulletSpeedMultiplier * BULLET_SPEED * PIXELS_ZOOM);
                m_level->AddGameObject(bullets[i], RENDERING_LAYER_BULLETS);
            }
            m_hasShot = true;
            m_shootDowntime = 0.15;
            return true;
        }
        return false;
    }

    bool IsAutomatic() override {
        return false;
    }
};

#endif //CONTRA_WEAPONS_H
