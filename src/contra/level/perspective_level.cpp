//
// Created by david on 3/3/20.
//

#include "perspective_level.h"
#include "../entities/Player.h"
#include "../entities/perspective/cores.h"

void PerspectiveLevel::Create(const std::string &folder,
                              const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets,
                              YAML::Node scene_root, short num_players, PlayerStats *stats, AvancezLib *engine) {
    Level::Create(folder, spritesheets, scene_root, num_players, stats, engine);
    for (const auto &rc_node: scene_root["weak_cores"]) {
        auto *core = new WeakCore();
        core->Create(this, rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM);
        core->Init();
        core->AddReceiver(this);
        AddGameObject(core, RENDERING_LAYER_ENEMIES);
    }
    for (const auto &rc_node: scene_root["strong_cores"]) {
        auto *core = new StrongCore();
        core->Create(this, rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM);
        core->Init();
        core->AddReceiver(this);
        AddGameObject(core, RENDERING_LAYER_ENEMIES);
    }
    for (const auto &rc_node: scene_root["core_canons"]) {
        auto *canon = new CoreCannon();
        canon->Create(this, rc_node["pos"].as<Vector2D>() * PIXELS_ZOOM);
        canon->Init();
        canon->AddReceiver(this);
        AddGameObject(canon, RENDERING_LAYER_ENEMIES);
    }
}

void PerspectiveLevel::Init() {
    Level::Init();
    m_camera = Vector2D(1024 * PIXELS_ZOOM, 0);
    m_laserOn = true;
}

Player *PerspectiveLevel::CreatePlayer(int index, PlayerStats *stats) {
    auto *player = new Player();
    player->Create(this, index);
    player->position = Vector2D(1024 * PIXELS_ZOOM + WINDOW_WIDTH / 2, PIXELS_ZOOM * 182);

    auto *playerControl = new PlayerControlPerspective();
    // TODO: add weapon to the stats and change this
    playerControl->Create(this, player, index, stats->lives, new DefaultWeapon(this));

    auto *gravity = player->GetComponent<Gravity *>();
    gravity->SetBaseFloor(PIXELS_ZOOM * 182);
    gravity->SetVelocity(-PLAYER_JUMP * PIXELS_ZOOM);

    player->AddComponent(playerControl);
    player->GetComponent<CollideComponent *>()->SetListener(playerControl);
    player->AddReceiver(this);
    return player;
}

void PerspectiveLevel::Receive(Message m) {
    if (m == SCREEN_CLEARED) {
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

bool PerspectiveLevel::AllPlayersOnFloor() {
    for (auto *player: playerControls) {
        if (!player->IsOnFloor()) return false;
    }
    return true;
}

