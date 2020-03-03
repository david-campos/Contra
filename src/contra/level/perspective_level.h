//
// Created by david on 3/3/20.
//

#ifndef CONTRA_PERSPECTIVE_LEVEL_H
#define CONTRA_PERSPECTIVE_LEVEL_H

#include "level.h"

class PerspectiveLevel : public Level {
public:
    void Create(const std::string &folder, const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets,
                YAML::Node scene_root, short num_players, PlayerStats *stats, AvancezLib *engine) override;

    void Init() override;

protected:
    Player *CreatePlayer(int index, PlayerStats *stats) override;
};

#endif //CONTRA_PERSPECTIVE_LEVEL_H
