#pragma once

#include "../components/render/AnimationRenderer.h"
#include "../consts.h"
#include "components/floor.h"
#include "entities/Player.h"
#include "../components/render/SimpleRenderer.h"
#include "entities/bullets.h"
#include "entities/canons.h"
#include "entities/enemies.h"
#include "player_stats.h"

class LevelFactory;

class Game : public GameObject {
private:
    GameObject *currentScene;
    AvancezLib *engine;
    LevelFactory *levelFactory;
    std::unordered_map<int, std::shared_ptr<Sprite>> spritesheets;
    AvancezLib::KeyStatus previousKeys;

    bool paused;
    bool can_continue;
    int current_level;
    PlayerStats stats[2];
    PlayerStats lastSavedStats[2];
    unsigned short players;
public:
    virtual void Create(AvancezLib *avancezLib);

    void Reset() {
        paused = false;
        can_continue = true;
        memset(lastSavedStats, 0, 2 * sizeof(PlayerStats));
        lastSavedStats[0].lives = lastSavedStats[1].lives = 2;
        memcpy(stats, lastSavedStats, 2 * sizeof(PlayerStats));
    }

    int GetCurrentLevel() const;

    /**
     * Sets the current level, indexes start in 0
     * @param currentLevel
     */
    void SetCurrentLevel(int currentLevel);

    void SetPlayers(unsigned short value) {
        if (value > 2) value = 2;
        if (value < 1) value = 1;
        players = value;
    }

    [[nodiscard]] int GetPlayers() const { return players; }

    [[nodiscard]] const PlayerStats *GetPlayerStats() const {
        return stats;
    }

    /**
     * Resets the player stats to the last saved ones
     */
    void RollbackPlayerStats() {
        memcpy(stats, lastSavedStats, sizeof(PlayerStats) * 2);
    }

    void Init() override {
        Enable();
        Reset();
        players = 1;
    }


    void Update(float dt) override {
        AvancezLib::KeyStatus keyStatus;
        engine->getKeyStatus(keyStatus);
        if (keyStatus.esc) {
            Destroy();
            engine->quit();
        }

        if (keyStatus.pause && !previousKeys.pause) {
            paused = !paused;
        }
        if (keyStatus.mute && !previousKeys.mute) {
            engine->ToggleSounds();
            engine->ToggleMusic();
        }
        previousKeys = keyStatus;

        if (paused)
            dt = 0.f;

        if (currentScene)
            currentScene->Update(dt);
    }

    void Start(BaseScene *scene) {
        if (currentScene) {
            currentScene->Destroy();
            delete currentScene;
        }
        currentScene = scene;
    }

    virtual void Draw() {
        engine->swapBuffers();
        engine->clearWindow();
    }

    void Receive(Message m) override;

    void Destroy() override;

private:
    BaseScene *InitMainMenu();
};