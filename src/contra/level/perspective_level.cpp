//
// Created by david on 3/3/20.
//

#include "perspective_level.h"
#include "../entities/Player.h"

void PerspectiveLevel::Create(const std::string &folder,
                              const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets,
                              YAML::Node scene_root, short num_players, PlayerStats *stats, AvancezLib *engine) {
    Level::Create(folder, spritesheets, scene_root, num_players, stats, engine);
}

void PerspectiveLevel::Init() {
    Level::Init();
    m_camera = Vector2D(1024 * PIXELS_ZOOM, 0);
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

