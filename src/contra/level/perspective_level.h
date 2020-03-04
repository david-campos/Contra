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

    void Update(float dt) override;

    void Receive(Message m) override;

    [[nodiscard]] bool IsLaserOn() const {
        return m_laserOn;
    }

protected:
    Player *CreatePlayer(int index, PlayerStats *stats) override;

    bool m_laserOn;
    short m_currentScreen = 0;
    short m_onTransition = -1;

    bool AllPlayersOnFloor();
};

#endif //CONTRA_PERSPECTIVE_LEVEL_H
