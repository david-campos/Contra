//
// Created by david on 3/3/20.
//

#include "scrolling_level.h"
#include "../entities/canons.h"
#include "../entities/enemies.h"
#include "../entities/pickups.h"
#include "../entities/exploding_bridge.h"
#include "../entities/defense_wall.h"

void
ScrollingLevel::Create(const std::string &folder, const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets,
                       YAML::Node scene_root, short num_players, PlayerStats *stats, AvancezLib *engine) {
    Level::Create(folder, spritesheets, scene_root, num_players, stats, engine);
    level_floor = std::make_shared<Floor>((folder + scene_root["floor_mask"].as<std::string>()).data());

    for (const auto &rc_node: scene_root["rotating_canons"]) {
        auto *tank = new RotatingCanon();
        tank->Create(this,
                rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM,
                rc_node["burst_length"].as<int>());
        tank->AddReceiver(this);
        AddNotFoundEnemy(tank, RENDERING_LAYER_ENEMIES);
    }
    for (const auto &rc_node: scene_root["gulcans"]) {
        auto *gulcan = new Gulcan();
        gulcan->Create(this, rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM);
        gulcan->AddReceiver(this);
        AddNotFoundEnemy(gulcan, RENDERING_LAYER_ENEMIES);
    }
    for (const auto &rc_node: scene_root["ledders"]) {
        auto *ledder = new Ledder();
        ledder->Create(this,
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
        AddNotFoundEnemy(ledder, RENDERING_LAYER_ENEMIES);
    }
    for (const auto &rc_node: scene_root["greeders"]) {
        if (rc_node["chance_skip"]) {
            if (m_random_dist(m_mt) < rc_node["chance_skip"].as<float>())
                continue;
        }
        auto *spawner = new GameObject();
        spawner->position = rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM;
        auto *greeder_spawner = new GreederSpawner();
        greeder_spawner->Create(this, spawner, m_random_dist(m_mt));
        spawner->AddComponent(greeder_spawner);
        spawner->AddReceiver(this);
        AddNotFoundEnemy(spawner, RENDERING_LAYER_ENEMIES);
    }
    for (const auto &rc_node: scene_root["covered_pickups"]) {
        AnimationRenderer *renderer;
        CreateAndAddPickUpHolder(rc_node["content"].as<PickUpType>(),
                rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM,
                new CoveredPickUpHolderBehaviour(),
                {-12, -15, 10, 15},
                &renderer);
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
    }
    for (const auto &rc_node: scene_root["flying_pickups"]) {
        AnimationRenderer *renderer;
        CreateAndAddPickUpHolder(rc_node["content"].as<PickUpType>(),
                rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM,
                new FlyingPickupHolderBehaviour(),
                {-9, -6, 9, 6},
                &renderer);
        renderer->AddAnimation({
                243, 120, 0.15, 1,
                24, 14, 12, 7,
                "Flying", AnimationRenderer::STOP_AND_LAST
        });
        renderer->AddAnimation({
                92, 611, 0.15, 3,
                30, 30, 15, 15,
                "Dying", AnimationRenderer::BOUNCE_AND_STOP});
    }
    for (const auto &rc_node: scene_root["exploding_bridges"]) {
        auto *bridge = new GameObject();
        bridge->Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(this, bridge, GetBridgeSprite());
        for (int i = 0; i < 5; i++) {
            std::string anim = "BridgeState" + std::to_string(i);
            renderer->AddAnimation({0, i * 31, 0.15, 3,
                                    128, 31, 0, 0,
                                    anim, AnimationRenderer::BOUNCE});
        }
        auto *behaviour = new ExplodingBridgeBehaviour();
        behaviour->Create(this, bridge);
        bridge->AddComponent(renderer);
        bridge->AddComponent(behaviour);
        bridge->AddReceiver(this);
        bridge->position = rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM;

        bridge->Init();
        AddNotFoundEnemy(bridge, RENDERING_LAYER_BRIDGES);
    }

    CreateDefenseWall();
}

void ScrollingLevel::Init() {
    m_camera = Vector2D(0, 0);
    if (!not_found_enemies.empty()) {
        next_enemy_x = not_found_enemies.top().first->position.x;
        while (next_enemy_x < WINDOW_WIDTH && !not_found_enemies.empty()) {
            game_objects[not_found_enemies.top().second]->insert(not_found_enemies.top().first);
            not_found_enemies.pop();
            next_enemy_x = not_found_enemies.top().first->position.x;
        }
    }

    Level::Init();
}

void ScrollingLevel::SubUpdate(float dt) {
    float players_top_x = PlayersTopX();
    if (complete && players_top_x >= levelWidth && !m_engine->isMusicPlaying()) {
        Send(NEXT_LEVEL);
        return;
    }

    // Adjust camera
    float players_min_x = PlayersMinX();
    if (PlayersAlive() > 0) {
        if (players_min_x < levelWidth - WINDOW_WIDTH) {
            int top_x = WINDOW_WIDTH * (players.size() > 1 ? 0.7f : 0.5f);
            if (players_top_x > m_camera.x + top_x) {
                float center_on_top = players_top_x - top_x;
                m_camera.x = (float) center_on_top > players_min_x - SCREEN_PLAYER_LEFT_MARGIN * PIXELS_ZOOM ?
                             players_min_x - SCREEN_PLAYER_LEFT_MARGIN * PIXELS_ZOOM : center_on_top;
            }
        } else if (m_camera.x < levelWidth - WINDOW_WIDTH)
            m_camera.x += PLAYER_SPEED * PIXELS_ZOOM * 2 * dt;
        else
            m_camera.x = levelWidth - WINDOW_WIDTH;
    }

    // Progressively init the enemies in front of the camera
    while (m_camera.x + WINDOW_WIDTH + RENDERING_MARGINS > next_enemy_x && !not_found_enemies.empty()) {
        auto *enemy = not_found_enemies.top().first;
        game_objects[not_found_enemies.top().second]->insert(enemy);
        enemy->Init();
        not_found_enemies.pop();
        next_enemy_x = not_found_enemies.top().first->position.x;
    }

    // Eliminate the enemies behind the camera
    std::set<GameObject *>::iterator it = game_objects[RENDERING_LAYER_ENEMIES]->begin();
    while (it != game_objects[RENDERING_LAYER_ENEMIES]->end()) {
        auto *game_object = *it;
        if (game_object->position.x < m_camera.x - RENDERING_MARGINS) {
            it = game_objects[RENDERING_LAYER_ENEMIES]->erase(it);
            if (game_object->onRemoval == DESTROY) {
                game_object->Destroy();
            }
        } else {
            it++;
        }
    }
}

void ScrollingLevel::Destroy() {
    while (!not_found_enemies.empty()) {
        not_found_enemies.top().first->Destroy();
        not_found_enemies.pop();
    }
    Level::Destroy();
}

void ScrollingLevel::CreateAndAddPickUpHolder(const PickUpType &type, const Vector2D &position,
                                              PickUpHolderBehaviour *behaviour, const Box &box,
                                              AnimationRenderer **renderer) {
    auto *pickup = new PickUp();
    pickup->Create(this, GetSpritesheet(SPRITESHEET_PICKUPS), &m_grid, level_floor, type);
    auto *pick_up_holder = new GameObject();
    pick_up_holder->position = position;
    behaviour->Create(this, pick_up_holder, pickup);
    *renderer = new AnimationRenderer();
    (*renderer)->Create(this, pick_up_holder, GetSpritesheet(SPRITESHEET_ENEMIES));
    auto *collider = new BoxCollider();
    collider->Create(this, pick_up_holder, box * PIXELS_ZOOM, -1, NPCS_COLLISION_LAYER);
    collider->SetListener(behaviour);

    pick_up_holder->AddComponent(behaviour);
    pick_up_holder->AddComponent(*renderer);
    pick_up_holder->AddComponent(collider);
    pick_up_holder->AddReceiver(this);
    AddNotFoundEnemy(pick_up_holder, RENDERING_LAYER_ENEMIES);
}

void ScrollingLevel::CreateDefenseWall() {
    auto sprite = std::shared_ptr<Sprite>(m_engine->createSprite("data/level1/defense_wall.png"));

    // Wall front in player layer, between enemies and the bullets
    auto *wallFront = new GameObject();
    wallFront->Create();
    auto *renderer = new SimpleRenderer();
    renderer->Create(this, wallFront, sprite,
            111, 0, 89, 96, 0, 0);
    wallFront->AddComponent(renderer);

    wallFront->position = Vector2D(3238, 41) * PIXELS_ZOOM;
    wallFront->Init();
    AddNotFoundEnemy(wallFront, RENDERING_LAYER_PLAYER);

    // A second piece of front wall that covers the left blaster canon
    wallFront = new GameObject();
    wallFront->Create();
    renderer = new SimpleRenderer();
    renderer->Create(this, wallFront, sprite,
            80, 72, 13, 40, 0, 0);
    wallFront->AddComponent(renderer);

    wallFront->position = Vector2D(3230, 95) * PIXELS_ZOOM;
    wallFront->Init();
    AddNotFoundEnemy(wallFront, RENDERING_LAYER_ENEMIES);


    // Door in enemies layer
    auto *door = new GameObject();
    door->Create();
    renderer = new SimpleRenderer();
    renderer->Create(this, door, sprite,
            0, 0, 111, 65, 0, 0);
    door->AddComponent(renderer); // Important order! First the back, then the animated
    auto *animator = new AnimationRenderer();
    animator->Create(this, door, sprite);
    animator->AddAnimation({
            1, 82, 0.15, 3,
            24, 32, -6, -16,
            "Door", AnimationRenderer::BOUNCE
    });
    animator->Play();
    auto *door_behaviour = new DefenseDoorBehaviour();
    door_behaviour->Create(this, door);
    auto *collider = new BoxCollider();
    collider->Create(this, door,
            6 * PIXELS_ZOOM, 20 * PIXELS_ZOOM,
            24 * PIXELS_ZOOM, 24 * PIXELS_ZOOM, -1, NPCS_COLLISION_LAYER);
    collider->SetListener(door_behaviour);
    door->AddComponent(collider);
    door->AddComponent(door_behaviour);
    door->AddComponent(animator);
    door->position = Vector2D(3217, 136) * PIXELS_ZOOM;
    door->AddReceiver(this);
    AddNotFoundEnemy(door, RENDERING_LAYER_ENEMIES);

    // Blaster canons
    auto *pool = CreateBlasterBulletPool();
    auto *canon = new GameObject();
    canon->Create();
    animator = new AnimationRenderer();
    animator->Create(this, canon, sprite);
    animator->AddAnimation({
            1, 66, 0.2, 2,
            25, 15, 0, 0,
            "Shoot", AnimationRenderer::STOP_AND_FIRST
    });
    animator->Pause();
    auto *behaviour = new BlasterCanonBehaviour();
    behaviour->Create(this, canon, pool);
    collider = new BoxCollider();
    collider->Create(this, canon,
            PIXELS_ZOOM, PIXELS_ZOOM, 14 * PIXELS_ZOOM, 8 * PIXELS_ZOOM, -1, NPCS_COLLISION_LAYER);
    collider->SetListener(behaviour);
    canon->AddComponent(behaviour);
    canon->AddComponent(animator);
    canon->AddComponent(collider);
    canon->position = Vector2D(3217, 120) * PIXELS_ZOOM;
    AddNotFoundEnemy(canon, RENDERING_LAYER_BRIDGES);
    door_behaviour->AddCanon(behaviour);

    pool = CreateBlasterBulletPool();
    canon = new GameObject();
    canon->Create();
    animator = new AnimationRenderer();
    animator->Create(this, canon, sprite);
    animator->AddAnimation({
            1, 66, 0.2, 2,
            25, 15, 0, 0,
            "Shoot", AnimationRenderer::STOP_AND_FIRST
    });
    animator->Pause();
    behaviour = new BlasterCanonBehaviour();
    behaviour->Create(this, canon, pool);
    collider = new BoxCollider();
    collider->Create(this, canon,
            PIXELS_ZOOM, PIXELS_ZOOM, 14 * PIXELS_ZOOM, 8 * PIXELS_ZOOM, -1, NPCS_COLLISION_LAYER);
    collider->SetListener(behaviour);
    canon->AddComponent(behaviour);
    canon->AddComponent(animator);
    canon->AddComponent(collider);
    canon->position = Vector2D(3239, 120) * PIXELS_ZOOM;
    AddNotFoundEnemy(canon, RENDERING_LAYER_PLAYER);
    door_behaviour->AddCanon(behaviour);
}

ObjectPool<Bullet> *ScrollingLevel::CreateBlasterBulletPool() {
    auto *pool = new ObjectPool<Bullet>();
    pool->Create(MAX_BLASTER_CANON_BULLETS);
    for (auto *bullet: pool->pool) {
        bullet->Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(this, bullet, GetSpritesheet(SPRITESHEET_ENEMIES));
        renderer->AddAnimation({
                204, 67, 0.2, 1,
                8, 8, 4, 4,
                "Bullet", AnimationRenderer::STOP_AND_FIRST
        });
        renderer->AddAnimation({
                92, 611, 0.15, 3,
                30, 30, 15, 15,
                "Kill", AnimationRenderer::BOUNCE_AND_STOP});
        renderer->Play();
        auto *gravity = new Gravity();
        gravity->Create(this, bullet);
        gravity->SetFallThroughCanFall(true);
        auto *behaviour = new BlastBulletBehaviour();
        behaviour->Create(this, bullet);
        auto *box_collider = new BoxCollider();
        Box box{-4, -4, 4, 4};
        box_collider->Create(this, bullet, box * PIXELS_ZOOM, PLAYER_COLLISION_LAYER, -1);
        bullet->AddComponent(gravity);
        bullet->AddComponent(behaviour);
        bullet->AddComponent(renderer);
        bullet->AddComponent(box_collider);
        bullet->AddReceiver(this);

        bullet->onRemoval = DO_NOT_DESTROY; // Do not destroy until the end of the game
    }
    return pool;
}

Player *ScrollingLevel::CreatePlayer(int index, PlayerStats *stats) {
    auto *player = new Player();
    player->Create(this, index);

    auto *playerControl = new PlayerControlScrolling();
    playerControl->Create(this, player, index, *stats);

    player->AddComponent(playerControl);
    player->GetComponent<CollideComponent *>()->SetListener(playerControl);
    player->AddReceiver(this);

    return player;
}
