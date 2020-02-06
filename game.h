#pragma once

#include "AnimationRenderer.h"
#include "consts.h"
#include "floor.h"
#include "Player.h"
#include "SimpleRenderer.h"
#include "bullets.h"

#define LIFE_SPRITE_WIDTH 8
#define LIFE_SPRITE_HEIGHT 16
#define LIFE_SPRITE_MARGIN 2
#define LIFE_SPRITE_X 193
#define LIFE_SPRITE_Y 0

#define MAX_BULLETS 20

class Game : public GameObject {
private:
    void CreatePlayer();

    std::set<GameObject *> game_objects;    // http://www.cplusplus.com/reference/set/set/

    Sprite *background;
    AvancezLib *engine;
    std::shared_ptr<Floor> level_floor;
    std::shared_ptr<Sprite> spritesheet;
    ObjectPool<Bullet>* bullets;
    Player *player;
    PlayerControl *playerControl;
    float camera_x;
    bool game_over;
public:
    virtual void Create(AvancezLib *avancezLib) {
        SDL_Log("Game::Create");
        this->engine = avancezLib;
        background = engine->createSprite("data/level1/background.png");
        level_floor = std::make_shared<Floor>("data/level1/mask.bmp");
        spritesheet.reset(engine->createSprite("data/spritesheet.png"));
        bullets = new ObjectPool<Bullet>();
        bullets->Create(MAX_BULLETS);
        for (auto* bullet: bullets->pool) {
            bullet->Create();
            auto* renderer = new SimpleRenderer();
            renderer->Create(engine, bullet, &game_objects, spritesheet, &camera_x,
                    82, 10, 3, 3, 1, 1);
            bullet->AddComponent(renderer);
            auto* behaviour = new BulletBehaviour();
            behaviour->Create(engine, bullet, &game_objects, &camera_x);
            bullet->AddComponent(behaviour);
        }
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
        for (int i = 1; i <= playerControl->getRemainingLives(); i++) {
            spritesheet->draw(
                    ((3 - i) * (LIFE_SPRITE_WIDTH + LIFE_SPRITE_MARGIN) + LIFE_SPRITE_MARGIN) * PIXELS_ZOOM,
                    9 * PIXELS_ZOOM,
                    LIFE_SPRITE_WIDTH * PIXELS_ZOOM, LIFE_SPRITE_HEIGHT * PIXELS_ZOOM,
                    LIFE_SPRITE_X, LIFE_SPRITE_Y, LIFE_SPRITE_WIDTH, LIFE_SPRITE_HEIGHT
            );
        }

        for (auto game_object : game_objects)
            game_object->Update(dt);
    }

    virtual void Draw() {
        engine->swapBuffers();
        engine->clearWindow();
    }

    virtual void Receive(Message m) {
        if (m == GAME_OVER) {
            game_over = true;
        }
    }


    bool IsGameOver() {
        return game_over;
    }

    virtual void destroy() {
        SDL_Log("Game::Destroy");
        for (auto game_object : game_objects)
            game_object->Destroy();
        bullets->Destroy();
        delete bullets;
        bullets = nullptr;
        delete player;
        player = nullptr;
        delete background;
        background = nullptr;
    }
};