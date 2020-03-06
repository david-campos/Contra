//
// Created by david on 3/3/20.
//

#include "perspective_level.h"
#include "../entities/Player.h"
#include "../entities/perspective/cores.h"
#include "../entities/perspective/pers_enemies.h"

void PerspectiveLevel::Create(const std::string &folder,
                              const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets,
                              YAML::Node scene_root, short num_players, PlayerStats *stats, AvancezLib *engine) {
    Level::Create(folder, spritesheets, scene_root, num_players, stats, engine);
    for (const auto &rc_node: scene_root["weak_cores"]) {
        auto *core = new WeakCore();
        core->Create(this, rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM);
        core->Init();
        core->AddReceiver(this);
        m_screens.insert({
                floor(core->position.x / WINDOW_WIDTH) - 4, // First 4 are the transition screens
                core
        });
    }
    for (const auto &rc_node: scene_root["strong_cores"]) {
        auto *core = new StrongCore();
        core->Create(this, rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM);
        core->Init();
        core->AddReceiver(this);
        m_screens.insert({
                floor(core->position.x / WINDOW_WIDTH) - 4, // First 4 are the transition screens
                core
        });
    }
    for (const auto &rc_node: scene_root["core_canons"]) {
        auto *canon = new CoreCannon();
        canon->Create(this, rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM);
        canon->Init();
        canon->AddReceiver(this);
        m_screens.insert({
                floor(canon->position.x / WINDOW_WIDTH) - 4, // First 4 are the transition screens
                canon
        });
    }
}

void PerspectiveLevel::Init() {
    Level::Init();
    m_laserOn = true;
}

Player *PerspectiveLevel::CreatePlayer(int index, PlayerStats *stats) {
    auto *player = new Player();
    player->Create(this, index);
    player->position = Vector2D(WINDOW_WIDTH / 2, PIXELS_ZOOM * PERSP_PLAYER_Y);

    auto *playerControl = new PlayerControlPerspective();
    // TODO: add weapon to the stats and change this
    playerControl->Create(this, player, index, stats->lives, new DefaultWeapon(this));

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
        ClearScreen();
        m_laserOn = false;
        m_currentScreen++;
        m_onTransition = 0;
        for (auto pos: {
                Vector2D(104, 71),
                Vector2D(151, 71),
                Vector2D(104, 102),
                Vector2D(151, 102)
        }) {
            auto *explosion = new Explosion();
            explosion->Create(this, pos * PIXELS_ZOOM);
            explosion->Init();
            AddGameObject(explosion, RENDERING_LAYER_ENEMIES);
        }
    } else {
        Level::Receive(m);
    }
}

void PerspectiveLevel::Update(float dt) {
    Level::Update(dt);

    float new_x = (m_onTransition < 0 ? m_currentScreen + 4 : m_onTransition) * WINDOW_WIDTH;
    if (abs(new_x - m_camera.x) > 0.001) {
        for (auto *player: players) {
            player->position.x = player->position.x - m_camera.x + new_x;
        }
        InitScreen();
        m_camera = Vector2D(new_x, 0); // The first 4 are the transitions
    }

    if (m_onTransition >= 0 && PlayersMinY() < PIXELS_ZOOM * (182 - 35) && AllPlayersOnFloor()) {
        m_onTransition++;
        for (int i = 0; i < players.size(); i++) {
            players[i]->position.y = PIXELS_ZOOM * 182;
            playerControls[i]->SetBaseFloor(PIXELS_ZOOM * 182);
        }
        if (m_onTransition == 4) {
            m_onTransition = -1;
            m_laserOn = true;
        }
    }
}

void PerspectiveLevel::InitScreen() {
    auto ret = m_screens.equal_range(m_currentScreen);
    for (auto it = ret.first; it != ret.second; ++it) {
        it->second->Init();
        AddGameObject(it->second, RENDERING_LAYER_BRIDGES);
    }

    auto *ledder = new PerspectiveLedder();
    ledder->Create(this, false, 1.f, PLAYER_SPEED * PIXELS_ZOOM);
    ledder->Init(Vector2D((m_currentScreen + 4) * WINDOW_WIDTH + PERSP_ENEMIES_MARGINS * PIXELS_ZOOM,
            PERSP_ENEMIES_Y * PIXELS_ZOOM));
    AddGameObject(ledder, RENDERING_LAYER_ENEMIES);
    ledder = new PerspectiveLedder();
    ledder->Create(this, false, 0.5f, PLAYER_SPEED * PIXELS_ZOOM);
    ledder->Init(Vector2D((m_currentScreen + 4) * WINDOW_WIDTH + (PERSP_ENEMIES_MARGINS - 20) * PIXELS_ZOOM,
            PERSP_ENEMIES_Y * PIXELS_ZOOM));
    AddGameObject(ledder, RENDERING_LAYER_ENEMIES);
    ledder = new PerspectiveLedder();
    ledder->Create(this, true, 0.f, PLAYER_SPEED * PIXELS_ZOOM);
    ledder->Init(Vector2D((m_currentScreen + 4) * WINDOW_WIDTH + (PERSP_ENEMIES_MARGINS - 40) * PIXELS_ZOOM,
            PERSP_ENEMIES_Y * PIXELS_ZOOM));
    AddGameObject(ledder, RENDERING_LAYER_ENEMIES);

    PickUp *pickUp = new PickUp();
    pickUp->Create(this, GetSpritesheet(SPRITESHEET_PICKUPS), &m_grid, level_floor, PICKUP_SPREAD,
            PERSP_PLAYER_Y * PIXELS_ZOOM);
    ledder = new PerspectiveLedder();
    ledder->Create(this, true, 0.f, 0.5 * PLAYER_SPEED * PIXELS_ZOOM, pickUp);
    ledder->Init(Vector2D((m_currentScreen + 4) * WINDOW_WIDTH + (PERSP_ENEMIES_MARGINS - 40) * PIXELS_ZOOM,
            PERSP_ENEMIES_Y * PIXELS_ZOOM));
    AddGameObject(ledder, RENDERING_LAYER_ENEMIES);

    pickUp = new PickUp();
    pickUp->Create(this, GetSpritesheet(SPRITESHEET_PICKUPS), &m_grid, level_floor, PICKUP_SPREAD,
            PERSP_PLAYER_Y * PIXELS_ZOOM);
    ledder = new PerspectiveLedder();
    ledder->Create(this, false, 0.f, 0.5 * PLAYER_SPEED * PIXELS_ZOOM, pickUp);
    ledder->Init(Vector2D((m_currentScreen + 4) * WINDOW_WIDTH + (PERSP_ENEMIES_MARGINS - 40) * PIXELS_ZOOM,
            PERSP_ENEMIES_Y * PIXELS_ZOOM));
    AddGameObject(ledder, RENDERING_LAYER_ENEMIES);
}

void PerspectiveLevel::KillScreen() {
    auto ret = m_screens.equal_range(m_currentScreen);
    for (auto it = ret.first; it != ret.second; ++it) {
        auto *killable = it->second->GetComponent<Killable *>();
        if (killable) {
            killable->Kill();
        }
    }
}

void PerspectiveLevel::ClearScreen() {
    auto ret = m_screens.equal_range(m_currentScreen);
    for (auto it = ret.first; it != ret.second; ++it) {
        it->second->MarkToRemove();
    }
}

bool PerspectiveLevel::AllPlayersOnFloor() {
    for (auto *player: playerControls) {
        if (!player->IsOnFloor()) return false;
    }
    return true;
}
