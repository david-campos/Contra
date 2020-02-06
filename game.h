#pragma once

#include "AnimationRenderer.h"
#include "consts.h"
#include "floor.h"
#include "Player.h"

class Game : public GameObject {
private:
    void CreatePlayer();

    std::set<GameObject *> game_objects;    // http://www.cplusplus.com/reference/set/set/

    Sprite *background;
    AvancezLib *engine;
    std::shared_ptr<Floor> level_floor;
    Player *player;
    float camera_x;
    bool game_over;
public:
    virtual void Create(AvancezLib *avancezLib) {
        SDL_Log("Game::Create");
        this->engine = avancezLib;
        background = engine->createSprite("data/level1/background.png");
        level_floor = std::make_shared<Floor>("data/level1/mask.bmp");
        CreatePlayer();
    }

    virtual void Init() {
        for (auto *go: game_objects) {
            go->Init();
        }
        enabled = true;
        camera_x = 0;
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

        if (player->position.x > camera_x + WINDOW_WIDTH / 2.) {
            camera_x = (float) player->position.x - WINDOW_WIDTH / 2.;
        }

        // Draw background
        background->draw(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                         (int) round(camera_x) / PIXELS_ZOOM, 0, WINDOW_WIDTH / PIXELS_ZOOM,
                         WINDOW_HEIGHT / PIXELS_ZOOM);

        // Debug floor printing
//        SDL_Color floor{0, 255, 0};
//        SDL_Color water{0, 0, 255};
//        for (int y = 0; y < level_floor->getHeight(); y++) {
//            for (int x = 0; x < level_floor->getWidth(); x++) {
//                if (level_floor->IsFloor(x, y)) {
//                    engine->fillSquare(x * PIXELS_ZOOM - (int) round(camera_x), y * PIXELS_ZOOM, PIXELS_ZOOM, floor);
//                } else if (level_floor->IsWater(x, y)) {
//                    engine->fillSquare(x * PIXELS_ZOOM - (int) round(camera_x), y * PIXELS_ZOOM, PIXELS_ZOOM, water);
//                }
//            }
//        }

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
        delete player;
        player = nullptr;
        delete background;
        background = nullptr;
    }
};