//
// Created by david on 14/2/20.
//

#ifndef CONTRA_LEVEL_H
#define CONTRA_LEVEL_H


#include <queue>
#include "game_object.h"
#include "floor.h"
#include "avancezlib.h"
#include "bullets.h"
#include "Player.h"

class Level : public GameObject {
    struct game_objects_comp_x {
        bool operator()(const GameObject *lhs, const GameObject *rhs) const {
            return lhs->position.x > rhs->position.x;
        }
    };

    AvancezLib *engine;
    std::unique_ptr<Sprite> background;
    std::shared_ptr<Sprite> spritesheet;
    std::shared_ptr<Sprite> enemies_spritesheet;
    std::shared_ptr<Sprite> pickups_spritesheet;
    std::shared_ptr<Floor> level_floor;
    Player *player;
    PlayerControl *playerControl;
    std::set<GameObject *> *game_objects[RENDERING_LAYERS];
    std::priority_queue<GameObject *, std::deque<GameObject *>, game_objects_comp_x> not_found_enemies;
    float next_enemy_x;
    ObjectPool<Bullet> *default_bullets, *fire_bullets,
            *machine_gun_bullets, *spread_bullets, *laser_bullets, *enemy_bullets;
    Grid grid;
    float camera_x;
    int level_width;
public:
    Level() {
        for (int i = 0; i < RENDERING_LAYERS; i++) {
            game_objects[i] = new std::set<GameObject *>();
        }
    }

    ~Level() {
        for (int i = 0; i < RENDERING_LAYERS; i++) {
            delete game_objects[i];
        }
    }

    void Create(const std::string &folder, const std::shared_ptr<Sprite> &sprite_sheet,
                const std::shared_ptr<Sprite> &enemies_spritesheet, const std::shared_ptr<Sprite> &pickups_spritesheet,
                AvancezLib *engine);

    void Init() override;

    void Update(float dt) override {
        AvancezLib::KeyStatus keys{};
        engine->getKeyStatus(keys);
        if (keys.esc) {
            Destroy();
            engine->quit();
        }

        if (player->position.x < level_width - WINDOW_WIDTH) {
            if (player->position.x > camera_x + WINDOW_WIDTH / 2.)
                camera_x = (float) player->position.x - WINDOW_WIDTH / 2.;
        } else if (camera_x < level_width - WINDOW_WIDTH)
            camera_x += PLAYER_SPEED * PIXELS_ZOOM * 2 * dt;
        else
            camera_x = level_width - WINDOW_WIDTH;

        // We progressively init the enemies in front of the camera
        while (camera_x + WINDOW_WIDTH + RENDERING_MARGINS > next_enemy_x && !not_found_enemies.empty()) {
            auto *enemy = not_found_enemies.top();
            game_objects[RENDERING_LAYER_ENEMIES]->insert(enemy);
            enemy->Init();
            not_found_enemies.pop();
            next_enemy_x = not_found_enemies.top()->position.x;
        }

        // And eliminate the enemies behind the camera
        std::set<GameObject *>::iterator it = game_objects[RENDERING_LAYER_ENEMIES]->begin();
        while (it != game_objects[RENDERING_LAYER_ENEMIES]->end()) {
            auto *game_object = *it;
            if (game_object->position.x < camera_x - RENDERING_MARGINS) {
                game_object->Disable();
                it = game_objects[RENDERING_LAYER_ENEMIES]->erase(it);
                if (game_object->onOutOfScreen == DISABLE_AND_DESTROY) {
                    game_object->Destroy();
                }
            } else {
                it++;
            }
        }

        // Draw background (smoothing the zoom)
        int camera_without_zoom = int(floor(camera_x / PIXELS_ZOOM));
        int reminder = int(round(camera_x - camera_without_zoom * PIXELS_ZOOM));
        background->draw(-reminder, 0, WINDOW_WIDTH + PIXELS_ZOOM, WINDOW_HEIGHT,
                camera_without_zoom, 0, WINDOW_WIDTH / PIXELS_ZOOM + 1,
                WINDOW_HEIGHT / PIXELS_ZOOM);

        for (int i = 1; i <= playerControl->getRemainingLives(); i++) {
            spritesheet->draw(
                    ((3 - i) * (LIFE_SPRITE_WIDTH + LIFE_SPRITE_MARGIN) + LIFE_SPRITE_MARGIN) * PIXELS_ZOOM,
                    9 * PIXELS_ZOOM,
                    LIFE_SPRITE_WIDTH * PIXELS_ZOOM, LIFE_SPRITE_HEIGHT * PIXELS_ZOOM,
                    LIFE_SPRITE_X, LIFE_SPRITE_Y, LIFE_SPRITE_WIDTH, LIFE_SPRITE_HEIGHT
            );
        }

        grid.ClearCollisionCache(); // Clear collision cache
        for (const auto &layer: game_objects)
            for (auto *game_object : *layer)
                game_object->Update(dt);
    }

    void Destroy() override;

private:
    void CreateBulletPools();

    void CreatePlayer();
};

#endif //CONTRA_LEVEL_H
