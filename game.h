#pragma once

#include "AnimationRenderer.h"
#include "consts.h"

class Game : public GameObject {
private:
    void CreatePlayer();
    std::set<GameObject *> game_objects;    // http://www.cplusplus.com/reference/set/set/

    Sprite *background;
    AvancezLib *engine;
    bool game_over;
public:
    virtual void Create(AvancezLib *avancezLib) {
        SDL_Log("Game::Create");
        this->engine = avancezLib;
        background = engine->createSprite("data/level1/background.png");
        CreatePlayer();
    }

    virtual void Init() {
        for (auto *go: game_objects) {
            go->Init();
        }
        enabled = true;
        game_over = false;
    }

    virtual void Update(float dt) {
        AvancezLib::KeyStatus keys{};
        engine->getKeyStatus(keys);
        if (keys.esc) {
            destroy();
            engine->quit();
        }

        if (IsGameOver())
            dt = 0.f;

        // Draw background
        background->draw(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                         0, 0, WINDOW_WIDTH / PIXELS_ZOOM, WINDOW_HEIGHT / PIXELS_ZOOM);

        for (auto game_object : game_objects)
            game_object->Update(dt);
    }

    virtual void Draw() {
        engine->swapBuffers();
        engine->clearWindow();
    }

    virtual void Receive(Message m) {
    }


    bool IsGameOver() {
        return game_over;
    }

    virtual void destroy() {
        SDL_Log("Game::Destroy");
        for (auto game_object : game_objects)
            game_object->Destroy();
    }
};