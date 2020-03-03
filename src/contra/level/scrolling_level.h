//
// Created by david on 3/3/20.
//

#ifndef CONTRA_SCROLLING_LEVEL_H
#define CONTRA_SCROLLING_LEVEL_H

#include <yaml-cpp/yaml.h>
#include "level.h"

class ScrollingLevel : public Level {
private:
    struct game_objects_comp_x {
        bool operator()(const std::pair<GameObject *, short> lhs, const std::pair<GameObject *, short> rhs) const {
            return lhs.first->position.x > rhs.first->position.x;
        }
    };

    std::priority_queue<std::pair<GameObject *, short>, std::deque<std::pair<GameObject *, short>>, game_objects_comp_x> not_found_enemies;
    float next_enemy_x;
public:
    void Create(const std::string &folder, const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets,
                YAML::Node scene_root, short num_players, PlayerStats *stats, AvancezLib *engine) override;

    void Init() override;

    void Update(float dt) override;

    void Destroy() override;

    void AddNotFoundEnemy(GameObject *const game_object, const short layer) {
        not_found_enemies.push(std::pair<GameObject *, short>(game_object, layer));
    }

private:
    /**
     * @param behaviour Create will be called, no need to create it first
     */
    void CreateAndAddPickUpHolder(const PickUpType &type, const Vector2D &position, PickUpHolderBehaviour *behaviour,
                                  const Box &box, AnimationRenderer **renderer_out);

    /**
     * Creates the defense wall of the end of stage 1
     */
    void CreateDefenseWall();

    ObjectPool<Bullet> *CreateBlasterBulletPool();

protected:
    Player *CreatePlayer(int index, PlayerStats *stats) override;
};

#endif //CONTRA_SCROLLING_LEVEL_H
