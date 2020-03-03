//
// Created by david on 14/2/20.
//

#include "level.h"
#include <SDL_log.h>
#include "yaml_converters.h"
#include "../entities/bullets.h"
#include "../entities/Player.h"
#include "../entities/defense_wall.h"

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
    if (scene_root["music"]) {
        music_str = folder + scene_root["music"].as<std::string>();
        music = music_str.data();
    }
    BaseScene::Create(avancezLib, bg.data(), music, animation_shift);
    levelWidth = m_background->getWidth() * PIXELS_ZOOM;
    m_grid.Create(34 * PIXELS_ZOOM, levelWidth, WINDOW_HEIGHT);

    CreateBulletPools();
    CreatePlayers(num_players, stats);
    PreloadSounds();
}

void Level::Destroy() {
    BaseScene::Destroy();
    SDL_Log("Level::Destroy");
    default_bullets->Destroy();
    delete default_bullets;
    default_bullets = nullptr;
    enemy_bullets->Destroy();
    delete enemy_bullets;
    enemy_bullets = nullptr;
    for (auto pair: shared_sounds) {
        delete pair.second;
    }
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
ObjectPool<Bullet> *Level::CreatePlayerBulletPool(int num_bullets, const AnimationRenderer::Animation &animation,
                                                  const Box &box) {
    auto *pool = new ObjectPool<Bullet>();
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
        box_collider->Create(this, bullet, box * PIXELS_ZOOM, NPCS_COLLISION_LAYER, -1);
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
