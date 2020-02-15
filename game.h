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
        currentLevel->Init();
    }


    void Update(float dt) override {
        if (IsGameOver())
            dt = 0.f;

        currentLevel->Update(dt);
    }

    virtual void Draw() {
        engine->swapBuffers();
        engine->clearWindow();
    }

    void Receive(Message m) override {
        if (m == GAME_OVER) {
            game_over = true;
        }
    }

    bool IsGameOver() {
        return game_over;
    }

    void Destroy() override {
        currentLevel->Destroy();
    }
};