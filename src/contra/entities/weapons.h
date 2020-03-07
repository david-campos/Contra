//
// Created by david on 9/2/20.
//

#ifndef CONTRA_WEAPONS_H
#define CONTRA_WEAPONS_H

#include "../../consts.h"
#include "bullets.h"
#include "weapon_types.h"

class Weapon {
protected:
    Level *m_level;
    float m_bulletSpeedMultiplier = 1.f;
    SoundEffect *m_sound;
public:
    explicit Weapon(Level *level, const char *sound_path = "data/sound/rifle.wav") : m_level(level) {
        m_sound = m_level->GetEngine()->createSound(sound_path);
    }

    virtual ~Weapon() {
        delete m_sound;
        m_sound = nullptr;
    }

    float GetBulletSpeedMultiplier() const {
        return m_bulletSpeedMultiplier;
    }

    void SetBulletSpeedMultiplier(float bulletSpeedMultiplier) {
        m_bulletSpeedMultiplier = bulletSpeedMultiplier;
    }

    virtual WeaponType GetWeaponType() = 0;

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
    virtual bool Fire(const Vector2D &position, const Vector2D &direction, const float minY = -9999) = 0;

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
    DefaultWeapon(Level *level, const char *sound = "data/sound/rifle.wav") : Weapon(level, sound) {}

    bool ShouldFire(bool fireKey, float dt) override {
        m_shootDowntime -= dt;
        if (!fireKey) {
            m_hasShot = false;
        }
        return !m_hasShot && fireKey && m_shootDowntime <= 0;
    }

    bool Fire(const Vector2D &position, const Vector2D &direction, const float minY = -9999) override {
        // Grab the bullet from the pool
        auto *bullet = GetBulletPool()->FirstAvailable();
        if (bullet != nullptr) {
            bullet->Init(position, direction.normalise(), m_bulletSpeedMultiplier * m_bulletSpeed * PIXELS_ZOOM,
                    minY);
            m_level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
            m_hasShot = true;
            m_shootDowntime = 0.15;
            m_sound->Play(1);
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

    WeaponType GetWeaponType() override {
        return RIFLE;
    }
};

class FireGun : public DefaultWeapon {
public:
    FireGun(Level *level) : DefaultWeapon(level, "data/sound/flamethrower.wav") {
        m_bulletSpeed = FIRE_BULLET_SPEED;
    }

    ObjectPool<Bullet> *GetBulletPool() const override {
        return m_level->GetFireBullets();
    }

    WeaponType GetWeaponType() override {
        return FIRE_GUN;
    }
};

class LaserGun : public Weapon {
protected:
    bool m_hasShot = false;
    float m_shootDowntime = 0;
    int m_nextBullet;
    Vector2D m_position, m_direction;
    float m_minY;
public:
    LaserGun(Level *level) : Weapon(level, "data/sound/laser.wav") {}

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

    bool Fire(const Vector2D &position, const Vector2D &direction, const float minY = -9999) override {
        // All bullets are fired and they reset
        m_nextBullet = 0;
        m_position = position;
        m_direction = direction;
        m_minY = minY;
        m_hasShot = true;
        m_sound->Play(1);
        ResetNext();
        return true;
    }

    void ResetNext() {
        auto *bullet = m_level->GetLaserBullets()->pool[m_nextBullet];
        bullet->Disable();
        m_level->RemoveImmediately(bullet, RENDERING_LAYER_BULLETS);
        bullet->Init(m_position, m_direction.normalise(), BULLET_SPEED * PIXELS_ZOOM, m_minY);
        m_level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
        m_nextBullet++;
        m_shootDowntime = 0.02;
    }

    WeaponType GetWeaponType() override {
        return LASER_GUN;
    }
};

class MachineGun : public Weapon {
private:
    float m_shootDowntime = 0;
    float m_soundTime = 0;
    bool m_soundStopped = true;
    std::function<void()> m_currentSoundStop = []() {};
public:
    MachineGun(Level *level) : Weapon(level, "data/sound/machine_gun.wav") {}

    bool ShouldFire(bool fireKey, float dt) override {
        m_shootDowntime -= dt;
        m_soundTime += dt;
        if (!m_soundStopped && !fireKey && m_soundTime > 0.15 && m_soundTime < 0.60) {
            m_currentSoundStop();
            m_soundStopped = true;
        }
        return fireKey && m_shootDowntime <= 0;
    }

    bool Fire(const Vector2D &position, const Vector2D &direction, const float minY = -9999) override {
        // Grab the bullet from the pool
        auto *bullet = m_level->GetMachineGunBullets()->FirstAvailable();
        if (bullet != nullptr) {
            bullet->Init(position, direction.normalise(), m_bulletSpeedMultiplier * BULLET_SPEED * PIXELS_ZOOM, minY);
            m_level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
            m_shootDowntime = 0.2;
            if (m_soundStopped || m_soundTime > 0.60) {
                m_currentSoundStop();
                m_currentSoundStop = m_sound->Play(1);
                m_soundTime = 0;
                m_soundStopped = false;
            }
            return true;
        }
        return false;
    }

    bool IsAutomatic() override {
        return true;
    }

    WeaponType GetWeaponType() override {
        return MACHINE_GUN;
    }
};

class SpreadGun : public Weapon {
private:
    float m_shootDowntime = 0;
    bool m_hasShot = false;
public:
    SpreadGun(Level *level) : Weapon(level, "data/sound/spread.wav") {}

    bool ShouldFire(bool fireKey, float dt) override {
        m_shootDowntime -= dt;
        if (!fireKey) {
            m_hasShot = false;
        }
        return !m_hasShot && fireKey && m_shootDowntime <= 0;
    }

    bool Fire(const Vector2D &position, const Vector2D &direction, const float minY = -9999) override {
        m_hasShot = false;
        std::vector<Bullet *> bullets = m_level->GetSpreadBullets()->FirstAvailableN(5);
        if (bullets.size() >= 3) {
            for (int i = 0; i < bullets.size(); i++) {
                float angle = 0.2618f - i * 0.5236f / (bullets.size() - 1); // 15deg - (30deg / (num bullets + 1)) * i
                bullets[i]->Init(position,
                        direction.rotate(angle).normalise(),
                        m_bulletSpeedMultiplier * BULLET_SPEED * PIXELS_ZOOM,
                        minY);
                m_level->AddGameObject(bullets[i], RENDERING_LAYER_BULLETS);
            }
            m_hasShot = true;
            m_sound->Play(1);
            m_shootDowntime = 0.15;
            return true;
        }
        return false;
    }

    bool IsAutomatic() override {
        return false;
    }

    WeaponType GetWeaponType() override {
        return SPREAD_GUN;
    }
};

#endif //CONTRA_WEAPONS_H
