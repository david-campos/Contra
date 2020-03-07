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

class PerspectiveLevel : public Level {
public:
    PerspectiveLevel() : Level() {
        // We adjust the collision layers as bullets here work by the minimum Y instead of the
        // objects checking collision
        m_playerBulletsCollisionLayer = -1;
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

    void Destroy() override;

    Vector2D ProjectFromFrontToBack(Vector2D &point) {
        return Vector2D(
                (point.x - m_camera.x - PERSP_FRONT_X_START * PIXELS_ZOOM) * PERSP_BACK_X_RANGE / PERSP_FRONT_X_RANGE
                + PERSP_BACK_X_START * PIXELS_ZOOM + m_camera.x,
                (point.y - PERSP_FRONT_Y_START * PIXELS_ZOOM) * PERSP_BACK_Y_RANGE / PERSP_FRONT_Y_RANGE +
                PERSP_BACK_Y_START * PIXELS_ZOOM);
    }

    Vector2D ProjectFromBackToFront(Vector2D &point) {
        return Vector2D(
                (point.x - m_camera.x - PERSP_BACK_X_START * PIXELS_ZOOM) * PERSP_FRONT_X_RANGE / PERSP_BACK_X_RANGE
                + PERSP_FRONT_X_START * PIXELS_ZOOM + m_camera.x,
                (point.y - PERSP_BACK_Y_START * PIXELS_ZOOM) * PERSP_FRONT_Y_RANGE / PERSP_BACK_Y_RANGE +
                PERSP_FRONT_Y_START * PIXELS_ZOOM);
    }

protected:
    Player *CreatePlayer(int index, PlayerStats *stats) override;

    bool m_laserOn;
    short m_currentScreen = 0;
    short m_onTransition = -1;
    std::multimap<int, GameObject *> m_screens;
    std::unordered_map<int, std::vector<PerspectiveLedderSpawn>> m_spawnPatterns;
    std::unordered_map<int, float> m_pretimes;
    int m_currentSpawn;
    float m_nextSpawn;

    bool AllPlayersOnFloor();
};

#endif //CONTRA_PERSPECTIVE_LEVEL_H
