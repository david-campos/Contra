//
// Created by david on 3/3/20.
//

#include "perspective_level.h"
#include "../entities/Player.h"
#include "../entities/perspective/cores.h"
#include "../entities/perspective/pers_enemies.h"
#include "../entities/perspective/darr.h"
#include "../entities/perspective/garmakilma.h"

namespace YAML {
    template<>
    struct convert<PerspectiveLedderSpawn> {
        static bool decode(const Node &node, PerspectiveLedderSpawn &rhs) {
            rhs.jumps = node["jumps"] && node["jumps"].as<bool>();
            rhs.stopToShootChance = node["stop_to_shoot_prob"] ? node["stop_to_shoot_prob"].as<float>() : 0.f;
            rhs.changeDirectionChance = node["change_dir_prob"] ? node["change_dir_prob"].as<float>() : 0.f;
            rhs.speedFactor = node["speed_factor"] ? node["speed_factor"].as<float>() : (rhs.jumps ? 0.5 : 1.0);
            if (node["pickup"]) {
                rhs.pickupToDrop = node["pickup"].as<PickUpType>();
                rhs.doesDrop = true;
            } else {
                rhs.doesDrop = false;
            }
            rhs.shootsPills = node["shoots_pills"] && node["shoots_pills"].as<bool>();
            rhs.cooldownMin = node["cooldown_min"] ? node["cooldown_min"].as<float>() : 0.5f;
            rhs.cooldownMax = node["cooldown_max"] ? node["cooldown_max"].as<float>() : 1.f;
            rhs.entrance = node["entrance"].as<std::string>() == "right" ? PerspectiveLedderSpawn::RIGHT
                                                                         : PerspectiveLedderSpawn::LEFT;
            if (rhs.entrance == PerspectiveLedderSpawn::RIGHT) {
                rhs.speedFactor *= -1;
            }
            rhs.secsUntilNext = node["secs_next"].as<float>();
            rhs.timesUsed = 0;
            return true;
        }
    };

    template<>
    struct convert<DarrSpawn> {
        static bool decode(const Node &node, DarrSpawn &rhs) {
            rhs.start = node["start"].as<float>();
            rhs.interval = node["interval"].as<float>();
            return true;
        }
    };
}

void PerspectiveLevel::Create(const std::string &folder,
                              const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets,
                              YAML::Node scene_root, short num_players, PlayerStats *stats, AvancezLib *engine) {
    Level::Create(folder, spritesheets, scene_root, num_players, stats, engine);
    m_screenCount = scene_root["screens"].as<int>();
    for (const auto &rc_node: scene_root["weak_cores"]) {
        auto *core = new WeakCore();
        core->Create(this, rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM);
        core->AddReceiver(this);
        AddToScreens(core);
    }
    for (const auto &rc_node: scene_root["strong_cores"]) {
        auto *core = new StrongCore();
        core->Create(this, rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM);
        core->AddReceiver(this);
        AddToScreens(core);
    }
    for (const auto &rc_node: scene_root["core_canons"]) {
        auto *canon = new CoreCannon();
        canon->Create(this, rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM);
        canon->AddReceiver(this);
        AddToScreens(canon);
    }
    auto spawn_patterns = scene_root["spawn_patterns"];
    for (int i = 0; i < spawn_patterns.size(); i++) {
        std::vector<PerspectiveLedderSpawn> pattern;
        float pretime = spawn_patterns[i]["pretime"].as<float>();
        m_pretimes.insert({i, pretime});
        if (spawn_patterns[i]["darrs"]) {
            m_darrs.insert({i, spawn_patterns[i]["darrs"].as<DarrSpawn>()});
        } else {
            m_darrs.insert({i, {-1.f, -1.f}});
        }
        for (const auto &spawn_node: spawn_patterns[i]["pattern"]) {
            pattern.push_back(spawn_node.as<PerspectiveLedderSpawn>());
        }
        m_spawnPatterns.insert({i, pattern});
    }
}

void PerspectiveLevel::Init() {
    Level::Init();
    m_laserOn = true;
    m_currentScreen = 5;
}

Player *PerspectiveLevel::CreatePlayer(int index, PlayerStats *stats) {
    auto *player = new Player();
    player->Create(this, index);
    player->position = Vector2D(WINDOW_WIDTH / 2, PIXELS_ZOOM * PERSP_PLAYER_Y);

    auto *playerControl = new PlayerControlPerspective();
    playerControl->Create(this, player, index, *stats);

    auto *gravity = player->GetComponent<Gravity *>();
    gravity->SetBaseFloor(PIXELS_ZOOM * PERSP_PLAYER_Y);
    gravity->SetVelocity(-PLAYER_JUMP * PIXELS_ZOOM);

    player->AddComponent(playerControl);
    player->GetComponent<CollideComponent *>()->SetListener(playerControl);
    player->AddReceiver(this);
    return player;
}

void PerspectiveLevel::Receive(Message m) {
    if (m == SCREEN_CLEARED) {
        m_laserOn = false;
        ClearScreen();
        m_currentScreen++;
        m_onTransition = 0;
        for (auto pos: {
                Vector2D(104, 71),
                Vector2D(151, 71),
                Vector2D(104, 102),
                Vector2D(151, 102)
        }) {
            auto *explosion = new Explosion();
            explosion->Create(this, m_camera + pos * PIXELS_ZOOM);
            explosion->Init();
            AddGameObject(explosion, RENDERING_LAYER_ENEMIES);
        }
    } else {
        Level::Receive(m);
    }
}

void PerspectiveLevel::SubUpdate(float dt) {
    float new_x = (m_onTransition < 0 ? m_currentScreen + 4 : m_onTransition) *
                  WINDOW_WIDTH;  // The first 4 are the transitions
    if (abs(new_x - m_camera.x) > 0.001) {
        for (auto *layer = game_objects; layer != game_objects + RENDERING_LAYERS; layer++) {
            for (auto *game_object: **layer)
                game_object->position.x = game_object->position.x - m_camera.x + new_x;
        }
        if (m_onTransition < 0) {
            InitScreen();
        }
        m_camera = Vector2D(new_x, 0);
    }

    if (m_onTransition >= 0) {
        bool some_alive;
        float min_y = PlayersMinY(&some_alive);
        if (some_alive && min_y < PIXELS_ZOOM * (PERSP_PLAYER_Y - 35) && AllPlayersOnFloor()) {
            m_onTransition++;
            for (int i = 0; i < players.size(); i++) {
                players[i]->position.y = PIXELS_ZOOM * PERSP_PLAYER_Y;
                playerControls[i]->SetBaseFloor(PIXELS_ZOOM * PERSP_PLAYER_Y);
            }
            if (m_onTransition == 4) {
                m_onTransition = -1;
                m_laserOn = true;
            }
        }
    } else if (m_spawnPatterns.count(m_currentScreen) > 0) {
        m_nextSpawn -= dt;
        while (m_nextSpawn <= 0.f) {
            m_nextSpawn = SpawnLedders();
        }
        if (m_nextDarrs > 0) {
            m_nextDarrs -= dt;
            if (m_nextDarrs <= 0) {
                SpawnDarrs();
                m_nextDarrs = m_darrs[m_currentScreen].interval;
            }
        }
    }
}

void PerspectiveLevel::InitScreen() {
    if (IsInBossBattle()) {
        for (int i = 0; i < players.size(); i++) {
            players[i]->position.y = PIXELS_ZOOM * PERSP_BOSS_PLAYER_Y;
            playerControls[i]->SetBaseFloor(PIXELS_ZOOM * PERSP_BOSS_PLAYER_Y);
        }
        m_animationShiftTime = 0.5f;
    }

    m_currentSpawn = -1;
    if (m_pretimes.count(m_currentScreen) > 1) {
        m_nextSpawn = m_pretimes[m_currentScreen];
    } else {
        m_nextSpawn = 1.f;
    }
    m_nextDarrs = m_darrs[m_currentScreen].start;
    m_nextDarrsStart = 1;
    m_nextDarrsEnd = 5;

    for (auto *go: m_screens[m_currentScreen]) {
        go->Init();
        AddGameObject(go, RENDERING_LAYER_BRIDGES);
        m_onScreen.insert(go);
    }
    m_screens.erase(m_currentScreen);

    if (IsInBossBattle()) {
        auto *boss = new Garmakilma();
        boss->Create(this, Vector2D((m_currentScreen + 4) * WINDOW_WIDTH, 0) + Vector2D(64, 24) * PIXELS_ZOOM);
        boss->Init();
        AddGameObject(boss, RENDERING_LAYER_BRIDGES);
        m_onScreen.insert(boss);
    }
}

void PerspectiveLevel::KillScreen() {
    for (auto *go: m_onScreen) {
        if (!go->IsEnabled()) continue;
        auto *killable = go->GetComponent<Killable *>();
        if (killable) {
            killable->Kill();
        }
    }
}

void PerspectiveLevel::ClearScreen() {
    for (auto *go: m_onScreen) {
        go->MarkToRemove();
    }
    m_onScreen.clear();
}

bool PerspectiveLevel::AllPlayersOnFloor() {
    for (auto *player: playerControls) {
        if (player->IsAlive() && !player->IsOnFloor()) return false;
    }
    return true;
}

void PerspectiveLevel::Destroy() {
    // All the stuff left in m_screens
    // has not been added to the game objects
    // list so it needs to be destroyed here
    for (const auto &screen: m_screens) {
        for (auto *go: screen.second) {
            go->Destroy();
        }
    }
    m_screens.clear();
    Level::Destroy();
}

float PerspectiveLevel::SpawnLedders() {
    auto &screen_pattern = m_spawnPatterns[m_currentScreen];
    m_currentSpawn = (m_currentSpawn + 1) % screen_pattern.size();
    auto *spawn = &screen_pattern[m_currentSpawn];

    PickUp *pickUp = nullptr;
    if (spawn->doesDrop && spawn->timesUsed == 0) {
        pickUp = new PickUp();
        pickUp->Create(this, GetSpritesheet(SPRITESHEET_PICKUPS), &m_grid,
                level_floor, spawn->pickupToDrop,
                PERSP_PLAYER_Y * PIXELS_ZOOM);
    }
    auto *ledder = new PerspectiveLedder();
    ledder->Create(this, spawn->jumps, spawn->stopToShootChance,
            spawn->speedFactor * PLAYER_SPEED * PIXELS_ZOOM,
            pickUp, spawn->shootsPills, spawn->cooldownMin, spawn->cooldownMax,
            spawn->changeDirectionChance);
    float x_pos = spawn->entrance == PerspectiveLedderSpawn::LEFT ?
                  PERSP_ENEMIES_MARGINS * PIXELS_ZOOM : WINDOW_WIDTH - PERSP_ENEMIES_MARGINS * PIXELS_ZOOM;
    ledder->Init(Vector2D((m_currentScreen + 4) * WINDOW_WIDTH + x_pos,
            PERSP_ENEMIES_Y * PIXELS_ZOOM));
    AddGameObject(ledder, RENDERING_LAYER_ENEMIES);
    m_onScreen.insert(ledder);

    spawn->timesUsed++;
    return spawn->secsUntilNext;
}

void PerspectiveLevel::SpawnDarrs() {
    const int total = 6;
    const int extra_margin = 10;
    for (int i = m_nextDarrsStart; i <= m_nextDarrsEnd; i++) {
        auto *darr = new Darr();
        darr->Create(this);
        Vector2D pos = Vector2D(
                (PERSP_ENEMIES_MARGINS + extra_margin) * PIXELS_ZOOM
                + ((WINDOW_WIDTH - (PERSP_ENEMIES_MARGINS + extra_margin) * 2 * PIXELS_ZOOM) /
                   float(total - 1)) * i,
                PERSP_ENEMIES_Y * PIXELS_ZOOM) + m_camera;
        Vector2D target = ProjectFromBackToFront(pos);
        darr->Init(pos, (target - pos) * 0.4f);
        AddGameObject(darr, RENDERING_LAYER_BRIDGES);
        m_onScreen.insert(darr);
    }
    m_nextDarrsStart = 1 - m_nextDarrsStart;
    m_nextDarrsEnd = 2 * total - 3 - m_nextDarrsEnd;
}
