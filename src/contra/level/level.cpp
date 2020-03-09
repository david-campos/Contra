//
// Created by david on 14/2/20.
//

#include "level.h"
#include <SDL_log.h>
#include "yaml_converters.h"
#include "../entities/bullets.h"
#include "../entities/Player.h"

void Level::Update(float dt) {
    BaseScene::Update(dt);

    if (PlayersAlive() <= 0) {
        Send(GAME_OVER);
        return;
    }

    // Print life sprites
    for (int i = 1; i <= playerControls[0]->getRemainingLives(); i++) {
        GetSpritesheet(SPRITESHEET_PLAYER)->draw(
                ((3 - i) * (LIFE_SPRITE_WIDTH + LIFE_SPRITE_MARGIN) + LIFE_SPRITE_MARGIN) * PIXELS_ZOOM,
                9 * PIXELS_ZOOM,
                LIFE_SPRITE_WIDTH * PIXELS_ZOOM, LIFE_SPRITE_HEIGHT * PIXELS_ZOOM,
                LIFE_SPRITE_X, LIFE_SPRITE_Y, LIFE_SPRITE_WIDTH, LIFE_SPRITE_HEIGHT
        );
    }
    if (playerControls.size() > 1) {
        for (int i = 1; i <= playerControls[1]->getRemainingLives(); i++) {
            GetSpritesheet(SPRITESHEET_PLAYER)->draw(
                    WINDOW_WIDTH -
                    ((3 - i) * (LIFE_SPRITE_WIDTH + LIFE_SPRITE_MARGIN) + LIFE_SPRITE_MARGIN) * PIXELS_ZOOM,
                    9 * PIXELS_ZOOM,
                    LIFE_SPRITE_WIDTH * PIXELS_ZOOM, LIFE_SPRITE_HEIGHT * PIXELS_ZOOM,
                    LIFE_SPRITE_X + SECOND_PLAYER_SHIFT, LIFE_SPRITE_Y, LIFE_SPRITE_WIDTH, LIFE_SPRITE_HEIGHT
            );
        }
    }

    SubUpdate(dt);
}

void Level::Create(const std::string &folder, const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets_map,
                   YAML::Node scene_root, short num_players, PlayerStats *stats, AvancezLib *avancezLib) {
    levelName = scene_root["name"].as<std::string>();
    levelIndex = scene_root["number"].as<int>();
    spritesheets = spritesheets_map;

    std::string bg = folder + scene_root["background"].as<std::string>();
    std::string music_str;
    char *music = nullptr;
    Vector2D animation_shift(0, 0);
    if (scene_root["background_animation_shift"]) {
        animation_shift = scene_root["background_animation_shift"].as<Vector2D>();
    }
    float animation_shift_time = 0.2f;
    if (scene_root["background_animation_shift_time"]) {
        animation_shift_time = scene_root["background_animation_shift_time"].as<float>();
    }
    if (scene_root["music"]) {
        music_str = folder + scene_root["music"].as<std::string>();
        music = music_str.data();
    }
    BaseScene::Create(avancezLib, bg.data(), music, animation_shift, animation_shift_time);
    levelWidth = m_background->getWidth() * PIXELS_ZOOM;
    m_grid.Create(34 * PIXELS_ZOOM, levelWidth, WINDOW_HEIGHT);

    CreateBulletPools(num_players);
    CreatePlayers(num_players, stats);
    PreloadSounds();
}

void Level::Destroy() {
    SDL_Log("Level::Destroy");
    for (int i = 0; i < players.size(); i++) {
        default_bullets[i].Destroy();
        machine_gun_bullets[i].Destroy();
        spread_bullets[i].Destroy();
        fire_bullets[i].Destroy();
        laser_bullets[i].Destroy();
    }
    delete[] default_bullets;
    default_bullets = nullptr;
    delete[] machine_gun_bullets;
    machine_gun_bullets = nullptr;
    delete[] spread_bullets;
    spread_bullets = nullptr;
    delete[] fire_bullets;
    fire_bullets = nullptr;
    delete[] laser_bullets;
    laser_bullets = nullptr;

    enemy_bullets->Destroy();
    delete enemy_bullets;
    enemy_bullets = nullptr;

    BaseScene::Destroy();

    for (auto pair: shared_sounds) {
        delete pair.second;
    }
}

void Level::CreateBulletPools(int num_players) {
    // Create bullet pools for the players
    default_bullets = CreatePlayersBulletPools<BulletStraightMovement>(MAX_DEFAULT_BULLETS, {
            82, 10, 0.2, 1,
            3, 3, 1, 1,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-1, -1, 2, 2}, num_players);
    machine_gun_bullets = CreatePlayersBulletPools<BulletStraightMovement>(MAX_MACHINE_GUN_BULLETS, {
            89, 9, 0.2, 1,
            5, 5, 2, 2,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-2, -2, 2, 2}, num_players);
    spread_bullets = CreatePlayersBulletPools<BulletStraightMovement>(MAX_SPREAD_BULLETS, {
            88, 8, 0.2, 3,
            8, 8, 4, 4,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-2, -2, 2, 2}, num_players);
    fire_bullets = CreatePlayersBulletPools<BulletCirclesMovement>(MAX_FIRE_BULLETS, {
            112, 8, 0.2, 1,
            8, 8, 4, 4,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-2, -2, 2, 2}, num_players);
    laser_bullets = CreatePlayersBulletPools<LaserBulletBehaviour>(MAX_LASER_BULLETS, {
            121, 0, 0.2, 1,
            6, 16, 3, 8,
            "Bullet", AnimationRenderer::STOP_AND_LAST
    }, {-3, -3, 3, 3}, num_players);
    for (int i = 0; i < num_players; i++) {
        for (auto *bullet: laser_bullets[i].pool) {
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
    }

    // Create bullet pool for the npcs
    enemy_bullets = new ObjectPool<Bullet>();
    enemy_bullets->Create(MAX_NPC_BULLETS);
    for (auto *bullet: enemy_bullets->pool) {
        bullet->Create();
        auto *renderer = new AnimationRenderer();
        renderer->Create(this, bullet, GetSpritesheet(SPRITESHEET_ENEMIES));
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
                3 * PIXELS_ZOOM, 3 * PIXELS_ZOOM, m_enemyBulletsCollisionLayer, m_enemyBulletsCollisionCheckLayer);
        bullet->AddComponent(behaviour);
        bullet->AddComponent(renderer);
        bullet->AddComponent(box_collider);
        bullet->AddReceiver(this);

        bullet->onRemoval = DO_NOT_DESTROY; // Do not destroy until the end of the game
    }
}

void Level::Init() {
    BaseScene::Init();
    complete = false;

    for (const auto &layer: game_objects) {
        for (auto game_object : *layer)
            game_object->Init();
    }

    if (m_music) m_music->Play();
}

void Level::CreatePlayers(short num_players, PlayerStats *stats) {
    for (short i = 0; i < num_players; i++) {
        auto *player = CreatePlayer(i, &stats[i]);
        game_objects[RENDERING_LAYER_PLAYER]->insert(player);
        players.push_back(player);
        auto *playerControl = player->GetComponent<PlayerControl *>();
        playerControls.push_back(playerControl);
    }
}

template<typename T>
ObjectPool <Bullet> *Level::CreatePlayersBulletPools(int num_bullets, const AnimationRenderer::Animation &animation,
                                                     const Box &box, int num_players) {
    auto *pools = new ObjectPool<Bullet>[num_players]();
    for (auto *pool = pools; pool < pools + num_players; pool++) {
        pool->Create(num_bullets);
        for (auto *bullet: pool->pool) {
            bullet->Create();
            auto *renderer = new AnimationRenderer();
            renderer->Create(this, bullet, GetSpritesheet(SPRITESHEET_PLAYER));
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
            box_collider->Create(this, bullet, box * PIXELS_ZOOM,
                    m_playerBulletsCollisionLayer, m_playerBulletsCollisionCheckLayer);
            bullet->AddComponent(behaviour);
            bullet->AddComponent(renderer);
            bullet->AddComponent(box_collider);
            bullet->AddReceiver(this);

            bullet->onRemoval = DO_NOT_DESTROY; // Do not destroy until the end of the game
        }
    }
    return pools;
}


void Level::Receive(Message m) {
    if (m == LEVEL_END) {
        complete = true;
        completeTime = m_time;
        mus_stage_clear->Play(1);
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
    auto *control = GetClosestPlayerControl(position);
    if (!control) return nullptr;
    else return static_cast<Player *>(control->GetGameObject());
}

PlayerControl *Level::GetClosestPlayerControl(const Vector2D &position, bool prefer_before) const {
    PlayerControl *closest = nullptr;
    float closestDist = 0;
    bool closestBefore = false;
    for (int i = 0; i < playerControls.size(); i++) {
        float dist = (players[i]->position - position).magnitudeSqr();
        bool is_before = players[i]->position.x < position.x;
        if (!playerControls[i]->IsAlive())
            continue;
        if (!closest || (dist < closestDist || (prefer_before && !closestBefore && is_before))) {
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
    float min_x = 0;
    bool found_alive = false;
    for (auto *player = &playerControls[0]; player < &playerControls[0] + playerControls.size(); player++) {
        if ((*player)->IsAlive() && (
                !found_alive || (*player)->GetGameObject()->position.x < min_x
        )) {
            min_x = (*player)->GetGameObject()->position.x;
            found_alive = true;
        }
    }
    return min_x;
}

float Level::PlayersMinY(bool *alive_players) const {
    float min_y = 0;
    bool found_alive = false;
    for (auto *player = &playerControls[0]; player < &playerControls[0] + playerControls.size(); player++) {
        if ((*player)->IsAlive() && (
                !found_alive || (*player)->GetGameObject()->position.y < min_y
        )) {
            min_y = (*player)->GetGameObject()->position.y;
            found_alive = true;
        }
    }
    if (alive_players) *alive_players = found_alive;
    return min_y;
}

float Level::PlayersTopX() const {
    float top_x = 0;
    bool found_alive = false;
    for (auto *player = &playerControls[0]; player < &playerControls[0] + playerControls.size(); player++) {
        if ((*player)->IsAlive() && (
                !found_alive || (*player)->GetGameObject()->position.x > top_x
        )) {
            top_x = (*player)->GetGameObject()->position.x;
            found_alive = true;
        }
    }
    return top_x;
}

float Level::PlayersTopY(bool *alive_players) const {
    float top_y = 0;
    bool found_alive = false;
    for (auto *player = &playerControls[0]; player < &playerControls[0] + playerControls.size(); player++) {
        if ((*player)->IsAlive() && (
                !found_alive || (*player)->GetGameObject()->position.y > top_y
        )) {
            top_y = (*player)->GetGameObject()->position.y;
            found_alive = true;
        }
    }
    if (alive_players) *alive_players = found_alive;
    return top_y;
}

void Level::PreloadSounds() {
    shared_sounds.insert({SOUND_ENEMY_DEATH, m_engine->createSound("data/sound/enemy_death.wav")});
    shared_sounds.insert({SOUND_ENEMY_HIT, m_engine->createSound("data/sound/enemy_hit.wav")});
    shared_sounds.insert({SOUND_PLAYER_DEATH, m_engine->createSound("data/sound/death.wav")});
    shared_sounds.insert({SOUND_EXPLOSION, m_engine->createSound("data/sound/explosion.wav")});
    shared_sounds.insert({SOUND_PICKUP, m_engine->createSound("data/sound/pickup.wav")});
    mus_stage_clear.reset(m_engine->createMusic("data/sound/stage_clear.wav"));
}

float Level::GetTimeSinceComplete() {
    return complete ? m_time - completeTime : -1.f;
}

