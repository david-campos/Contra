//
// Created by david on 14/2/20.
//

#include "level.h"
#include "canons.h"
#include "enemies.h"
#include <SDL_log.h>
#include "yaml_converters.h"
#include "pickups.h"

void Level::Create(const std::string &folder, const std::shared_ptr<Sprite> &player_spritesheet,
                   const std::shared_ptr<Sprite> &enemy_spritesheet, const std::shared_ptr<Sprite> &pickup_spritesheet,
                   AvancezLib *avancezLib) {
    SDL_Log("Level::Create");
    this->engine = avancezLib;
    spritesheet = player_spritesheet;
    enemies_spritesheet = enemy_spritesheet;
    pickups_spritesheet = pickup_spritesheet;
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
            tank->Create(engine, game_objects, enemies_spritesheet, &camera_x,
                    rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM, player, enemy_bullets, &grid,
                    rc_node["burst_length"].as<int>());
            tank->AddReceiver(this);
            not_found_enemies.push(tank);
        }
        for (const auto &rc_node: scene_root["gulcans"]) {
            auto *gulcan = new Gulcan();
            gulcan->Create(engine, game_objects, enemies_spritesheet, &camera_x,
                    rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM, player,
                    enemy_bullets, &grid);
            gulcan->AddReceiver(this);
            not_found_enemies.push(gulcan);
        }
        for (const auto &rc_node: scene_root["ledders"]) {
            auto *ledder = new Ledder();
            ledder->Create(engine, game_objects, enemy_bullets, player,
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
            if (rc_node["chance_skip"]) {
                if (m_random_dist(m_mt) < rc_node["chance_skip"].as<float>())
                    continue;
            }
            auto *spawner = new GameObject();
            spawner->position = rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM;
            auto *greeder_spawner = new GreederSpawner();
            greeder_spawner->Create(engine, spawner, game_objects,
                    enemies_spritesheet, &camera_x,
                    &grid, level_floor, this,
                    m_random_dist(m_mt)
            );
            spawner->AddComponent(greeder_spawner);
            spawner->AddReceiver(this);
            not_found_enemies.push(spawner);
        }
        for (const auto &rc_node: scene_root["covered_pickups"]) {
            auto *pickup = new PickUp();
            pickup->Create(engine, game_objects, pickups_spritesheet, &grid, &camera_x,
                    level_floor, rc_node["content"].as<PickUpType>());
            auto *pick_up_holder = new GameObject();
            pick_up_holder->position = rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM;
            auto *behaviour = new CoveredPickUpHolderBehaviour();
            behaviour->Create(engine, pick_up_holder, game_objects, pickup);
            auto *renderer = new AnimationRenderer();
            renderer->Create(engine, pick_up_holder, game_objects, enemies_spritesheet, &camera_x);
            renderer->AddAnimation({
                    1, 76, 0.15, 1,
                    34, 34, 17, 17,
                    "Closed", AnimationRenderer::STOP_AND_LAST
            });
            renderer->AddAnimation({
                    35, 110, 0.15, 3,
                    34, 34, 17, 17,
                    "Opening", AnimationRenderer::STOP_AND_LAST
            });
            renderer->AddAnimation({
                    137, 76, 0.15, 3,
                    34, 34, 17, 17,
                    "Open", AnimationRenderer::BOUNCE
            });
            renderer->AddAnimation({
                    92, 611, 0.15, 3,
                    30, 30, 15, 15,
                    "Dying", AnimationRenderer::BOUNCE_AND_STOP});
            auto *collider = new BoxCollider();
            collider->Create(engine, pick_up_holder, game_objects, &grid, &camera_x,
                    -12 * PIXELS_ZOOM, -15 * PIXELS_ZOOM,
                    22 * PIXELS_ZOOM, 30 * PIXELS_ZOOM, -1, NPCS_COLLISION_LAYER);
            collider->SetListener(behaviour);

            pick_up_holder->AddComponent(behaviour);
            pick_up_holder->AddComponent(renderer);
            pick_up_holder->AddComponent(collider);
            pick_up_holder->AddReceiver(this);
            not_found_enemies.push(pick_up_holder);
        }
    } catch (YAML::BadFile &exception) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load level file: %s/level.yaml", &folder[0]);
    }
}

void Level::Destroy() {
    GameObject::Destroy();
    for (const auto &layer: game_objects) {
        for (auto game_object : *layer)
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
    default_bullets = CreatePlayerBulletPool(MAX_DEFAULT_BULLETS, {
            82, 10, 0.2, 1,
            3, 3, 1, 1,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-1, -1, 2, 2});
    machine_gun_bullets = CreatePlayerBulletPool(MAX_MACHINE_GUN_BULLETS, {
            89, 9, 0.2, 1,
            5, 5, 2, 2,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-2, -2, 2, 2});
    spread_bullets = CreatePlayerBulletPool(MAX_SPREAD_BULLETS, {
            88, 8, 0.2, 3,
            8, 8, 4, 4,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-2, -2, 2, 2});

    // Create bullet pool for the npcs
    enemy_bullets = new ObjectPool<Bullet>();
    enemy_bullets->Create(MAX_NPC_BULLETS);
    for (auto *bullet: enemy_bullets->pool) {
        bullet->Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(engine, bullet, game_objects, enemies_spritesheet, &camera_x);
        renderer->AddAnimation({
                199, 72, 0.2, 1,
                3, 3, 1, 1,
                "Bullet", AnimationRenderer::STOP_AND_LAST
        });
        auto *behaviour = new BulletBehaviour();
        behaviour->Create(engine, bullet, game_objects, &camera_x);
        auto *box_collider = new BoxCollider();
        box_collider->Create(engine, bullet, game_objects, &grid, &camera_x,
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
            game_objects[RENDERING_LAYER_ENEMIES]->insert(not_found_enemies.top());
            not_found_enemies.pop();
            next_enemy_x = not_found_enemies.top()->position.x;
        }
    }

    for (const auto &layer: game_objects) {
        for (auto game_object : *layer)
            game_object->Init();
    }
}

void Level::CreatePlayer() {
    player = new Player();
    player->Create(engine, game_objects, spritesheet, level_floor, &camera_x,
            default_bullets, fire_bullets, machine_gun_bullets, spread_bullets, laser_bullets,
            &grid, PLAYER_COLLISION_LAYER);
    playerControl = player->GetComponent<PlayerControl *>();
    player->AddReceiver(this);
    game_objects[RENDERING_LAYER_PLAYER]->insert(player);
}

ObjectPool<Bullet> *Level::CreatePlayerBulletPool(int num_bullets, const AnimationRenderer::Animation &animation,
                                                  const Box &box) {
    auto *pool = new ObjectPool<Bullet>();
    pool->Create(num_bullets);
    for (auto *bullet: pool->pool) {
        bullet->Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(engine, bullet, game_objects, spritesheet, &camera_x);
        renderer->AddAnimation(animation);
        renderer->AddAnimation({
                104, 0, 0.2, 1,
                7, 7, 3, 3,
                "Kill", AnimationRenderer::STOP_AND_FIRST
        });
        renderer->Play();
        auto *behaviour = new BulletBehaviour();
        behaviour->Create(engine, bullet, game_objects, &camera_x);
        auto *box_collider = new BoxCollider();
        box_collider->Create(engine, bullet, game_objects, &grid, &camera_x, {
                box.top_left_x * PIXELS_ZOOM,
                box.top_left_y * PIXELS_ZOOM,
                box.bottom_right_x * PIXELS_ZOOM,
                box.bottom_right_y * PIXELS_ZOOM
        }, NPCS_COLLISION_LAYER, -1);
        bullet->AddComponent(behaviour);
        bullet->AddComponent(renderer);
        bullet->AddComponent(box_collider);
        bullet->AddReceiver(this);
    }
    return pool;
}
