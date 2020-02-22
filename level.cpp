//
// Created by david on 14/2/20.
//

#include "level.h"
#include "canons.h"
#include "enemies.h"
#include <SDL_log.h>
#include "yaml_converters.h"
#include "pickups.h"
#include "component.h"
#include "bullets.h"
#include "Player.h"
#include "exploding_bridge.h"
#include "defense_wall.h"

void Level::Update(float dt) {
    BaseScene::Update(dt);
    float players_top_x = PlayersTopX();
    if (complete && players_top_x >= level_width) {
        Send(NEXT_LEVEL);
        return;
    }

    if (PlayersAlive() <= 0) {
        Send(GAME_OVER);
        return;
    }

    // Print life sprites
    for (int i = 1; i <= playerControls[0]->getRemainingLives(); i++) {
        spritesheet->draw(
                ((3 - i) * (LIFE_SPRITE_WIDTH + LIFE_SPRITE_MARGIN) + LIFE_SPRITE_MARGIN) * PIXELS_ZOOM,
                9 * PIXELS_ZOOM,
                LIFE_SPRITE_WIDTH * PIXELS_ZOOM, LIFE_SPRITE_HEIGHT * PIXELS_ZOOM,
                LIFE_SPRITE_X, LIFE_SPRITE_Y, LIFE_SPRITE_WIDTH, LIFE_SPRITE_HEIGHT
        );
    }
    if (playerControls.size() > 1) {
        for (int i = 1; i <= playerControls[1]->getRemainingLives(); i++) {
            spritesheet->draw(
                    WINDOW_WIDTH -
                    ((3 - i) * (LIFE_SPRITE_WIDTH + LIFE_SPRITE_MARGIN) + LIFE_SPRITE_MARGIN) * PIXELS_ZOOM,
                    9 * PIXELS_ZOOM,
                    LIFE_SPRITE_WIDTH * PIXELS_ZOOM, LIFE_SPRITE_HEIGHT * PIXELS_ZOOM,
                    LIFE_SPRITE_X + SECOND_PLAYER_SHIFT, LIFE_SPRITE_Y, LIFE_SPRITE_WIDTH, LIFE_SPRITE_HEIGHT
            );
        }
    }

    // Adjust camera
    float players_min_x = PlayersMinX();
    if (players_min_x < level_width - WINDOW_WIDTH) {
        int top_x = WINDOW_WIDTH * (players.size() > 1 ? 0.7f : 0.5f);
        if (players_top_x > m_camera.x + top_x) {
            float center_on_top = players_top_x - top_x;
            m_camera.x = (float) center_on_top > players_min_x - SCREEN_PLAYER_LEFT_MARGIN * PIXELS_ZOOM ?
                    players_min_x - SCREEN_PLAYER_LEFT_MARGIN * PIXELS_ZOOM : center_on_top;
        }
    } else if (m_camera.x < level_width - WINDOW_WIDTH)
        m_camera.x += PLAYER_SPEED * PIXELS_ZOOM * 2 * dt;
    else
        m_camera.x = level_width - WINDOW_WIDTH;

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

void Level::Create(const std::string &folder, const std::shared_ptr<Sprite> &player_spritesheet,
                   const std::shared_ptr<Sprite> &enemy_spritesheet, const std::shared_ptr<Sprite> &pickup_spritesheet,
                   short num_players, AvancezLib *avancezLib) {
    SDL_Log("Level::Create");
    spritesheet = player_spritesheet;
    enemies_spritesheet = enemy_spritesheet;
    pickups_spritesheet = pickup_spritesheet;
    try {
        YAML::Node scene_root = YAML::LoadFile(folder + "/level.yaml");

        levelName = scene_root["name"].as<std::string>();
        levelIndex = scene_root["number"].as<int>();

        {
            std::string bg = folder + scene_root["background"].as<std::string>();
            BaseScene::Create(avancezLib, bg.data());
        }

        level_width = m_background->getWidth() * PIXELS_ZOOM;
        level_floor = std::make_shared<Floor>((folder + scene_root["floor_mask"].as<std::string>()).data());
        grid.Create(34 * PIXELS_ZOOM, level_width, WINDOW_HEIGHT);

        CreateBulletPools();
        CreatePlayers(num_players);

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
    } catch (YAML::BadFile &exception) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load level file: %s/level.yaml", &folder[0]);
    }
}

void Level::Destroy() {
    BaseScene::Destroy();
    SDL_Log("Level::Destroy");
    while (!not_found_enemies.empty()) {
        not_found_enemies.top().first->Destroy();
        not_found_enemies.pop();
    }
    default_bullets->Destroy();
    delete default_bullets;
    default_bullets = nullptr;
    enemy_bullets->Destroy();
    delete enemy_bullets;
    enemy_bullets = nullptr;
    for (auto *player: players) {
        delete player;
        player = nullptr;
    }
}

void Level::CreateBulletPools() {
    // Create bullet pools for the player
    default_bullets = CreatePlayerBulletPool<BulletStraightMovement>(MAX_DEFAULT_BULLETS, {
            82, 10, 0.2, 1,
            3, 3, 1, 1,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-1, -1, 2, 2});
    machine_gun_bullets = CreatePlayerBulletPool<BulletStraightMovement>(MAX_MACHINE_GUN_BULLETS, {
            89, 9, 0.2, 1,
            5, 5, 2, 2,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-2, -2, 2, 2});
    spread_bullets = CreatePlayerBulletPool<BulletStraightMovement>(MAX_SPREAD_BULLETS, {
            88, 8, 0.2, 3,
            8, 8, 4, 4,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-2, -2, 2, 2});
    fire_bullets = CreatePlayerBulletPool<BulletCirclesMovement>(MAX_FIRE_BULLETS, {
            112, 8, 0.2, 1,
            8, 8, 4, 4,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-2, -2, 2, 2});
    laser_bullets = CreatePlayerBulletPool<LaserBulletBehaviour>(MAX_LASER_BULLETS, {
            121, 0, 0.2, 1,
            6, 16, 3, 8,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-3, -3, 3, 3});
    for (auto *bullet: laser_bullets->pool) {
        auto *renderer = bullet->GetComponent<AnimationRenderer *>();
        renderer->AddAnimation({
                128, 3, 0.2, 1,
                8, 13, 4, 6,
                "BulletDiag", AnimationRenderer::STOP_AND_LAST
        });
        renderer->AddAnimation({
                136, 9, 0.2, 1,
                16, 6, 8, 3,
                "BulletHorizontal", AnimationRenderer::STOP_AND_LAST
        });
    }

    // Create bullet pool for the npcs
    enemy_bullets = new ObjectPool<Bullet>();
    enemy_bullets->Create(MAX_NPC_BULLETS);
    for (auto *bullet: enemy_bullets->pool) {
        bullet->Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(this, bullet, enemies_spritesheet);
        renderer->AddAnimation({
                199, 72, 0.2, 1,
                3, 3, 1, 1,
                "Bullet", AnimationRenderer::STOP_AND_LAST
        });
        auto *behaviour = new BulletStraightMovement();
        behaviour->Create(this, bullet);
        auto *box_collider = new BoxCollider();
        box_collider->Create(this, bullet,
                -1 * PIXELS_ZOOM, -1 * PIXELS_ZOOM,
                3 * PIXELS_ZOOM, 3 * PIXELS_ZOOM, PLAYER_COLLISION_LAYER, -1);
        bullet->AddComponent(behaviour);
        bullet->AddComponent(renderer);
        bullet->AddComponent(box_collider);
        bullet->AddReceiver(this);

        bullet->onRemoval = DO_NOT_DESTROY; // Do not destroy until the end of the game
    }
}

void Level::Init() {
    BaseScene::Init();
    m_camera = Vector2D(0, 0);
    complete = false;

    if (!not_found_enemies.empty()) {
        next_enemy_x = not_found_enemies.top().first->position.x;
        while (next_enemy_x < WINDOW_WIDTH && !not_found_enemies.empty()) {
            game_objects[not_found_enemies.top().second]->insert(not_found_enemies.top().first);
            not_found_enemies.pop();
            next_enemy_x = not_found_enemies.top().first->position.x;
        }
    }

    for (const auto &layer: game_objects) {
        for (auto game_object : *layer)
            game_object->Init();
    }
}

void Level::CreatePlayers(short num_players) {
    for (short i = 0; i < num_players; i++) {
        auto *player = new Player();
        player->Create(this, i);
        auto *playerControl = player->GetComponent<PlayerControl *>();
        player->AddReceiver(this);
        game_objects[RENDERING_LAYER_PLAYER]->insert(player);
        players.push_back(player);
        playerControls.push_back(playerControl);
    }
}

template<typename T>
ObjectPool<Bullet> *Level::CreatePlayerBulletPool(int num_bullets, const AnimationRenderer::Animation &animation,
                                                  const Box &box) {
    auto *pool = new ObjectPool<Bullet>();
    pool->Create(num_bullets);
    for (auto *bullet: pool->pool) {
        bullet->Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(this, bullet, spritesheet);
        renderer->AddAnimation(animation);
        renderer->AddAnimation({
                104, 0, 0.1, 1,
                7, 7, 3, 3,
                "Kill", AnimationRenderer::STOP_AND_FIRST
        });
        renderer->Play();
        auto *behaviour = new T();
        behaviour->Create(this, bullet);
        auto *box_collider = new BoxCollider();
        box_collider->Create(this, bullet, box * PIXELS_ZOOM, NPCS_COLLISION_LAYER, -1);
        bullet->AddComponent(behaviour);
        bullet->AddComponent(renderer);
        bullet->AddComponent(box_collider);
        bullet->AddReceiver(this);

        bullet->onRemoval = DO_NOT_DESTROY; // Do not destroy until the end of the game
    }
    return pool;
}

void Level::CreateAndAddPickUpHolder(const PickUpType &type, const Vector2D &position,
                                     PickUpHolderBehaviour *behaviour, const Box &box, AnimationRenderer **renderer) {
    auto *pickup = new PickUp();
    pickup->Create(this, pickups_spritesheet, &grid, level_floor, type);
    auto *pick_up_holder = new GameObject();
    pick_up_holder->position = position;
    behaviour->Create(this, pick_up_holder, pickup);
    *renderer = new AnimationRenderer();
    (*renderer)->Create(this, pick_up_holder, enemies_spritesheet);
    auto *collider = new BoxCollider();
    collider->Create(this, pick_up_holder, box * PIXELS_ZOOM, -1, NPCS_COLLISION_LAYER);
    collider->SetListener(behaviour);

    pick_up_holder->AddComponent(behaviour);
    pick_up_holder->AddComponent(*renderer);
    pick_up_holder->AddComponent(collider);
    pick_up_holder->AddReceiver(this);
    AddNotFoundEnemy(pick_up_holder, RENDERING_LAYER_ENEMIES);
}

void Level::CreateDefenseWall() {
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
}

ObjectPool<Bullet> *Level::CreateBlasterBulletPool() {
    auto *pool = new ObjectPool<Bullet>();
    pool->Create(MAX_BLASTER_CANON_BULLETS);
    for (auto *bullet: pool->pool) {
        bullet->Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(this, bullet, GetEnemiesSpritesheet());
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

void Level::Receive(Message m) {
    if (m == LEVEL_END) {
        complete = true;
    } else {
        BaseScene::Receive(m);
    }
}

const std::string &Level::GetLevelName() const {
    return levelName;
}

int Level::GetLevelIndex() const {
    return levelIndex;
}

Player *Level::GetClosestPlayer(const Vector2D &position) const {
    return dynamic_cast<Player *>(GetClosestPlayerControl(position)->GetGameObject());
}

PlayerControl *Level::GetClosestPlayerControl(const Vector2D &position, bool prefer_before) const {
    auto *closest = playerControls[0];
    float closestDist = (players[0]->position - position).magnitudeSqr();
    bool closestBefore = players[0]->position.x < position.x;
    for (int i = 1; i < playerControls.size(); i++) {
        float dist = (players[i]->position - position).magnitudeSqr();
        bool is_before = players[i]->position.x < position.x;
        if (dist < closestDist || (prefer_before && !closestBefore && is_before)) {
            closestDist = dist;
            closestBefore = is_before;
            closest = playerControls[i];
        }
    }
    return closest;
}

short Level::PlayersAlive() const {
    short alive = 0;
    for (auto *playerControl: playerControls) {
        if (playerControl->getRemainingLives() >= 0)
            alive++;
    }
    return alive;
}

float Level::PlayersMinX() const {
    float min_x = players[0]->position.x;
    for (auto* player = &players[1]; player < &players[0] + players.size(); player++) {
        if ((*player)->position.x < min_x)
            min_x = (*player)->position.x;
    }
    return min_x;
}

float Level::PlayersTopX() const {
    float max_x = players[0]->position.x;
    for (auto* player = &players[1]; player < &players[0] + players.size(); player++) {
        if ((*player)->position.x > max_x)
            max_x = (*player)->position.x;
    }
    return max_x;
}
