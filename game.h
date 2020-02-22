#pragma once

#include "AnimationRenderer.h"
#include "consts.h"
#include "floor.h"
#include "Player.h"
#include "SimpleRenderer.h"
#include "bullets.h"
#include "canons.h"
#include "enemies.h"

class Game : public GameObject {
public:
    struct PlayerStats {
        int score;
    };
private:
    GameObject *currentScene;
    AvancezLib *engine;
    std::shared_ptr<Sprite> spritesheet;
    std::shared_ptr<Sprite> enemies_spritesheet;
    std::shared_ptr<Sprite> pickups_spritesheet;

    bool game_over;
    bool paused;
    bool pause_pressed_before;
    bool can_continue;
    PlayerStats stats[2];
    unsigned short players;
public:
    virtual void Create(AvancezLib *avancezLib);

    void SetPlayers(unsigned short value) {
        if (value > 2) value = 2;
        if (value < 1) value = 1;
        players = value;
    }

    [[nodiscard]] int GetPlayers() const { return players; }

    [[nodiscard]] const PlayerStats *GetPlayerStats() const {
        return stats;
    }

    void Init() override {
        Enable();
        game_over = false;
        pause_pressed_before = false;
        currentScene->Init();
        players = 1;
        can_continue = true;
        memset(stats, 0, 2 * sizeof(PlayerStats));
    }


    void Update(float dt) override {
        AvancezLib::KeyStatus keyStatus;
        engine->getKeyStatus(keyStatus);
        if (keyStatus.esc) {
            Destroy();
            engine->quit();
        }

        if (keyStatus.pause && !pause_pressed_before) {
            paused = !paused;
        }
        pause_pressed_before = keyStatus.pause;

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

    void Destroy() override {
        SDL_Log("Game::Destroy");
        if (currentScene) currentScene->Destroy();
    }
};