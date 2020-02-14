//
// Created by david on 14/2/20.
//

#ifndef CONTRA_SCENE_H
#define CONTRA_SCENE_H


#include "game_object.h"
#include "floor.h"
#include "avancezlib.h"
#include "bullets.h"
#include "Player.h"

class Level : public GameObject {
    AvancezLib *engine;
    std::unique_ptr<Sprite> background;
    std::shared_ptr<Sprite> spritesheet;
    std::shared_ptr<Sprite> enemies_spritesheet;
    std::shared_ptr<Floor> level_floor;
    Player *player;
    PlayerControl *playerControl;
    std::set<GameObject*> game_objects[RENDERING_LAYERS];
    ObjectPool<Bullet> *default_bullets, *fire_bullets,
            *machine_gun_bullets, *spread_bullets, *laser_bullets, *enemy_bullets;
    Grid grid;
    float camera_x;
    int level_width;
public:
    void Create(const std::string &folder, const std::shared_ptr<Sprite> &sprite_sheet,
                const std::shared_ptr<Sprite> &enemies_spritesheet, AvancezLib *engine);

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

        // Draw background (smoothing the zoom)
        int camera_without_zoom = int(floor(camera_x / PIXELS_ZOOM));
        int reminder = int(round(camera_x - camera_without_zoom * PIXELS_ZOOM));
        background->draw(-reminder, 0, WINDOW_WIDTH + PIXELS_ZOOM, WINDOW_HEIGHT,
                camera_without_zoom, 0, WINDOW_WIDTH / PIXELS_ZOOM + 1,
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
//        // Debug grid
//        int cells = WINDOW_WIDTH / grid.getCellSize() + 1;
//        int start = floor(camera_x / grid.getCellSize());
//        for (int i = 0; i < grid.getColSize(); i++) {
//            for (int j = start; j < start + cells; j++) {
//                if (!grid.GetCell(j, i)->GetLayer(PLAYER_COLLISION_LAYER)->empty()) {
//                    engine->fillSquare(j * grid.getCellSize() - camera_x, i*grid.getCellSize(), grid.getCellSize(),
//                            {0, 0, 0});
//                } else {
//                    engine->fillSquare(j * grid.getCellSize() - camera_x, i*grid.getCellSize(), grid.getCellSize(),
//                            {255, 255, 255});
//                }
//                engine->strokeSquare(j * grid.getCellSize() - camera_x, i*grid.getCellSize(),
//                        j * grid.getCellSize() - camera_x + grid.getCellSize(), (i+1)*grid.getCellSize(),
//                        {155, 155, 155});
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

        grid.ClearCollisionCache(); // Clear collision cache
        for (const auto &layer: game_objects)
            for (auto *game_object : layer)
                game_object->Update(dt);
    }

    void Destroy() override;
private:
    void CreateBulletPools();
    void CreatePlayer();
};

#endif //CONTRA_SCENE_H
