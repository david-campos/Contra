#pragma once

#include "AnimationRenderer.h"
#include "consts.h"
#include "floor.h"
#include "Player.h"
#include "SimpleRenderer.h"
#include "bullets.h"
#include "canons.h"
#include "enemies.h"
#include "level.h"

class Game : public GameObject {
private:
    Level *currentLevel;
    AvancezLib *engine;
    std::shared_ptr<Sprite> spritesheet;
    std::shared_ptr<Sprite> enemies_spritesheet;
    std::shared_ptr<Sprite> pickups_spritesheet;

    bool game_over;
    bool paused;
    bool pause_pressed_before;
public:
    virtual void Create(AvancezLib *avancezLib) {
        SDL_Log("Game::Create");
        this->engine = avancezLib;
        spritesheet.reset(engine->createSprite("data/spritesheet.png"));
        enemies_spritesheet.reset(engine->createSprite("data/enemies_spritesheet.png"));
        pickups_spritesheet.reset(engine->createSprite("data/pickups.png"));

        currentLevel = new Level();
        currentLevel->Create("data/level1/", spritesheet, enemies_spritesheet, pickups_spritesheet, engine);
        currentLevel->AddReceiver(this);
    }

    void Init() override {
        Enable();
        game_over = false;
        pause_pressed_before = false;
        currentLevel->Init();
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

        if (IsGameOver() || paused)
            dt = 0.f;

        if (currentLevel)
            currentLevel->Update(dt);
    }

    virtual void Draw() {
        engine->swapBuffers();
        engine->clearWindow();
    }

    void Receive(Message m) override {
        switch (m) {
            case GAME_OVER:
                SDL_Log("GAME OVER");
                game_over = true;
                break;
            case LEVEL_END:
                SDL_Log("LEVEL END");
                currentLevel->Destroy();
                delete currentLevel;
                currentLevel = nullptr;
                break;
        }
    }

    bool IsGameOver() {
        return game_over;
    }

    void Destroy() override {
        if (currentLevel) currentLevel->Destroy();
    }
};