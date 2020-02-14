//
// Created by david on 14/2/20.
//

#include "scene.h"
#include "canons.h"
#include "enemies.h"
#include <SDL_log.h>
#include "yaml_converters.h"

void Level::Create(const std::string &folder, const std::shared_ptr<Sprite> &player_spritesheet,
                   const std::shared_ptr<Sprite> &enemy_spritesheet,
                   AvancezLib *avancezLib) {
    SDL_Log("Level::Create");
    this->engine = avancezLib;
    spritesheet = player_spritesheet;
    enemies_spritesheet = enemy_spritesheet;
    try {
        YAML::Node scene_root = YAML::LoadFile(folder + "/level.yaml");

        background.reset(engine->createSprite((folder + scene_root["background"].as<std::string>()).data()));
        level_width = background->getWidth() * PIXELS_ZOOM;
        level_floor = std::make_shared<Floor>((folder + scene_root["floor_mask"].as<std::string>()).data());
        grid.Create(34 * PIXELS_ZOOM, level_width, WINDOW_HEIGHT);

        CreateBulletPools();
        CreatePlayer();

        for (const auto &rc_node: scene_root["rotating_canons"]) {
            auto *tank = new RotatingCanon();
            tank->Create(engine, &game_objects[0], enemies_spritesheet, &camera_x,
                    rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM, player, enemy_bullets, &grid,
                    rc_node["burst_length"].as<int>());
            tank->AddReceiver(this);
            not_found_enemies.push(tank);
        }
        for (const auto &rc_node: scene_root["gulcans"]) {
            auto *gulcan = new Gulcan();
            gulcan->Create(engine, &game_objects[0], enemies_spritesheet, &camera_x,
                    rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM, player,
                    enemy_bullets, &grid);
            gulcan->AddReceiver(this);
            not_found_enemies.push(gulcan);
        }
        for (const auto &rc_node: scene_root["ledders"]) {
            auto *ledder = new Ledder();
            ledder->Create(engine, &game_objects[0], enemy_bullets, player,
                    enemies_spritesheet, &camera_x, &grid,
                    rc_node["time_hidden"].as<float>(),
                    rc_node["time_shown"].as<float>(),
                    rc_node["cooldown_time"].as<float>(),
                    rc_node["show_standing"].as<bool>(),
                    rc_node["burst_length"].as<int>(),
                    rc_node["burst_cooldown"].as<float>(),
                    rc_node["horizontally_precise"].as<bool>()
            );
            ledder->position = rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM;
            ledder->AddReceiver(this);
            not_found_enemies.push(ledder);
        }
        for (const auto &rc_node: scene_root["greeders"]) {
            auto *spawner = new GameObject();
            spawner->position = rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM;
            auto *greeder_spawner = new GreederSpawner();
            greeder_spawner->Create(engine, spawner, &game_objects[0],
                    enemies_spritesheet, &camera_x,
                    &grid, level_floor, this
            );
            spawner->AddComponent(greeder_spawner);
            spawner->AddReceiver(this);
            not_found_enemies.push(spawner);
        }
    } catch (YAML::BadFile &exception) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load level file: %s/level.yaml", &folder[0]);
    }
}

void Level::Destroy() {
    SDL_Log("Level::Destroy");
    GameObject::Destroy();
    for (const auto &layer: game_objects) {
        for (auto game_object : layer)
            game_object->Destroy();
    }
    while (!not_found_enemies.empty()) {
        not_found_enemies.top()->Destroy();
        not_found_enemies.pop();
    }
    default_bullets->Destroy();
    delete default_bullets;
    default_bullets = nullptr;
    enemy_bullets->Destroy();
    delete enemy_bullets;
    enemy_bullets = nullptr;
    delete player;
    player = nullptr;
    background.reset();
}

void Level::CreateBulletPools() {
    // Create bullet pools for the player
    default_bullets = new ObjectPool<Bullet>();
    default_bullets->Create(MAX_DEFAULT_BULLETS);
    for (auto *bullet: default_bullets->pool) {
        bullet->Create();
        auto *renderer = new SimpleRenderer();
        renderer->Create(engine, bullet, &game_objects[0], spritesheet, &camera_x,
                82, 10, 3, 3, 1, 1);
        auto *behaviour = new BulletBehaviour();
        behaviour->Create(engine, bullet, &game_objects[0], &camera_x);
        auto *box_collider = new BoxCollider();
        box_collider->Create(engine, bullet, &game_objects[0], &grid, &camera_x,
                -1 * PIXELS_ZOOM, -1 * PIXELS_ZOOM,
                3 * PIXELS_ZOOM, 3 * PIXELS_ZOOM, NPCS_COLLISION_LAYER, -1);
        bullet->AddComponent(behaviour);
        bullet->AddComponent(renderer);
        bullet->AddComponent(box_collider);
        bullet->AddReceiver(this);
    }

    // Create bullet pool for the npcs
    enemy_bullets = new ObjectPool<Bullet>();
    enemy_bullets->Create(MAX_NPC_BULLETS);
    for (auto *bullet: enemy_bullets->pool) {
        bullet->Create();
        auto *renderer = new SimpleRenderer();
        renderer->Create(engine, bullet, &game_objects[0], enemies_spritesheet, &camera_x,
                199, 72, 3, 3, 1, 1);
        auto *behaviour = new BulletBehaviour();
        behaviour->Create(engine, bullet, &game_objects[0], &camera_x);
        auto *box_collider = new BoxCollider();
        box_collider->Create(engine, bullet, &game_objects[0], &grid, &camera_x,
                -1 * PIXELS_ZOOM, -1 * PIXELS_ZOOM,
                3 * PIXELS_ZOOM, 3 * PIXELS_ZOOM, PLAYER_COLLISION_LAYER, -1);
        bullet->AddComponent(behaviour);
        bullet->AddComponent(renderer);
        bullet->AddComponent(box_collider);
        bullet->AddReceiver(this);
    }
}

void Level::Init() {
    GameObject::Init();
    camera_x = 0;

    next_enemy_x = not_found_enemies.top()->position.x;
    while (next_enemy_x < WINDOW_WIDTH) {
        if (!not_found_enemies.empty()) {
            game_objects[RENDERING_LAYER_ENEMIES].insert(not_found_enemies.top());
            not_found_enemies.pop();
            next_enemy_x = not_found_enemies.top()->position.x;
        }
    }

    for (const auto &layer: game_objects) {
        for (auto game_object : layer)
            game_object->Init();
    }
}

void Level::CreatePlayer() {
    player = new Player();
    player->Create(engine, &game_objects[0], spritesheet, level_floor, &camera_x, default_bullets, &grid,
            PLAYER_COLLISION_LAYER);
    playerControl = player->GetComponent<PlayerControl *>();
    player->AddReceiver(this);
    game_objects[RENDERING_LAYER_PLAYER].insert(player);
}
