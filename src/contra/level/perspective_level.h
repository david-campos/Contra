//
// Created by david on 3/3/20.
//

#ifndef CONTRA_PERSPECTIVE_LEVEL_H
#define CONTRA_PERSPECTIVE_LEVEL_H

#include "level.h"
#include "../hittable.h"
#include "perspective_const.h"

struct PerspectiveLedderSpawn {
    enum Entrance {
        LEFT, RIGHT
    };
    bool jumps;
    float stopToShootChance;
    float changeDirectionChance;
    float speedFactor;
    bool doesDrop;
    PickUpType pickupToDrop;
    Entrance entrance;
    bool shootsPills;
    float cooldownMin;
    float cooldownMax;
    float secsUntilNext;
    unsigned timesUsed;
};

struct DarrSpawn {
    float start;
    float interval;
};


class PerspectiveLevel : public Level {
public:
    PerspectiveLevel() : Level() {
        // We adjust the collision layers as bullets here work by the minimum Y instead of the
        // objects checking collision
        m_playerBulletsCollisionLayer = PERSP_PLAYER_BULLETS_COLLISION_LAYER;
        m_playerBulletsCollisionCheckLayer = NPCS_COLLISION_LAYER;
        m_enemyBulletsCollisionLayer = -1;
        m_enemyBulletsCollisionCheckLayer = PLAYER_COLLISION_LAYER;
    }

    void Create(const std::string &folder, const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets,
                YAML::Node scene_root, short num_players, PlayerStats *stats, AvancezLib *engine) override;

    void Init() override;

    void InitScreen();

    void KillScreen();

    void ClearScreen();

    void SubUpdate(float dt) override;

    void Receive(Message m) override;

    [[nodiscard]] bool IsLaserOn() const {
        return m_laserOn;
    }

    [[nodiscard]] bool IsInBossBattle() const {
        return m_currentScreen == m_screenCount - 1 && m_onTransition < 0;
    }

    void Destroy() override;

    Vector2D ProjectFromFrontToBack(const Vector2D &point) {
        if (IsInBossBattle()) {
            return Vector2D(point.x, PERSP_BACK_Y_START * PIXELS_ZOOM + point.y - PERSP_FRONT_Y_START * PIXELS_ZOOM);
        } else {
            return Vector2D(
                    (point.x - m_camera.x - PERSP_FRONT_X_START * PIXELS_ZOOM) * PERSP_BACK_X_RANGE /
                    PERSP_FRONT_X_RANGE
                    + PERSP_BACK_X_START * PIXELS_ZOOM + m_camera.x,
                    (point.y - PERSP_FRONT_Y_START * PIXELS_ZOOM) * PERSP_BACK_Y_RANGE / PERSP_FRONT_Y_RANGE +
                    PERSP_BACK_Y_START * PIXELS_ZOOM);
        }
    }

    Vector2D ProjectFromBackToFront(const Vector2D &point) {
        if (IsInBossBattle()) {
            return Vector2D(point.x, PERSP_FRONT_Y_START * PIXELS_ZOOM + point.y - PERSP_BACK_Y_START * PIXELS_ZOOM);
        } else {
            return Vector2D(
                    (point.x - m_camera.x - PERSP_BACK_X_START * PIXELS_ZOOM) * PERSP_FRONT_X_RANGE / PERSP_BACK_X_RANGE
                    + PERSP_FRONT_X_START * PIXELS_ZOOM + m_camera.x,
                    (point.y - PERSP_BACK_Y_START * PIXELS_ZOOM) * PERSP_FRONT_Y_RANGE / PERSP_BACK_Y_RANGE +
                    PERSP_FRONT_Y_START * PIXELS_ZOOM);
        }
    }

    void AddToScreens(GameObject *object) {
        int screen_idx = floor(object->position.x / WINDOW_WIDTH) - 4; // First 4 are the transition screens

        std::vector<GameObject *> *screen = nullptr;
        if (m_screens.count(screen_idx) == 0) {
            m_screens.insert({screen_idx, {object}});
        } else {
            m_screens[screen_idx].push_back(object);
        }
    }

protected:
    Player *CreatePlayer(int index, PlayerStats *stats) override;

    float SpawnLedders();

    void SpawnDarrs();

    bool m_laserOn;
    short m_currentScreen = 0;
    short m_onTransition = -1;
    int m_screenCount;
    std::unordered_map<int, std::vector<GameObject *>> m_screens;
    std::set<GameObject *> m_onScreen;
    std::unordered_map<int, std::vector<PerspectiveLedderSpawn>> m_spawnPatterns;
    std::unordered_map<int, float> m_pretimes;
    std::unordered_map<int, DarrSpawn> m_darrs;
    Music *m_bossMusic;
    int m_currentSpawn;
    float m_nextSpawn;
    int m_nextDarrsStart, m_nextDarrsEnd;
    float m_nextDarrs;

    bool AllPlayersOnFloor();
};

#endif //CONTRA_PERSPECTIVE_LEVEL_H
