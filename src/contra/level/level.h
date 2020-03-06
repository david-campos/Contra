//
// Created by david on 14/2/20.
//

#ifndef CONTRA_LEVEL_H
#define CONTRA_LEVEL_H


#include <queue>
#include <random>
#include <yaml-cpp/node/node.h>
#include "../../kernel/game_object.h"
#include "../components/floor.h"
#include "../../kernel/avancezlib.h"
#include "../../consts.h"
#include "../../components/collision/grid.h"
#include "../../kernel/object_pool.h"
#include "../../components/render/AnimationRenderer.h"
#include "../entities/pickup_types.h"
#include "../../components/scene.h"
#include "../player_stats.h"
#include "yaml_converters.h"

class Player;

class PlayerControl;

struct Box;

class Bullet;

class PickUpHolderBehaviour;

class Level : public BaseScene {
protected:
    std::unordered_map<int, SoundEffect *> shared_sounds;
    const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets;
    std::shared_ptr<Sprite> bridge_sprite;
    std::shared_ptr<Floor> level_floor;
    std::unique_ptr<Music> mus_stage_clear;
    std::vector<Player *> players;
    std::vector<PlayerControl *> playerControls;
    ObjectPool<Bullet> *default_bullets, *fire_bullets,
            *machine_gun_bullets, *spread_bullets, *laser_bullets, *enemy_bullets;
    bool complete;
    float completeTime;
    std::string levelName;
    int levelIndex;
    int levelWidth;

    std::random_device rd;
    std::mt19937 m_mt = std::mt19937(rd());
    std::uniform_real_distribution<float> m_random_dist = std::uniform_real_distribution<float>(0.f, 1.f);
public:
    virtual void Create(const std::string &folder, const std::unordered_map<int, std::shared_ptr<Sprite>> *spritesheets,
                        YAML::Node scene_root, short num_players, PlayerStats *stats, AvancezLib *engine);

    void Init() override;

    void Update(float dt) override;

    [[nodiscard]] bool IsComplete() const {
        return complete;
    }

    std::shared_ptr<Sprite> GetBridgeSprite() {
        if (!bridge_sprite) {
            bridge_sprite = std::shared_ptr<Sprite>(m_engine->createSprite("data/bridge.png"));
        }
        return bridge_sprite;
    }

    void Destroy() override;

    SoundEffect *GetSound(int id) const {
        return shared_sounds.at(id);
    }

    const std::shared_ptr<Sprite> &GetSpritesheet(int id) const {
        return spritesheets->at(id);
    }

    const std::weak_ptr<Floor> GetLevelFloor() const {
        return level_floor;
    }

    Player *GetClosestPlayer(const Vector2D &position) const;

    /**
     * Gets the player control of the closest player to the given point
     * @param position Point for which to look for the closest player
     * @param only_before If set to true, any position with lower x than the given
     * position has preference over a position with higher x value than the given one
     * (prefers to select a player "in front"/"before" the given point).
     * @return
     */
    PlayerControl *GetClosestPlayerControl(const Vector2D &position, bool only_before = false) const;

    short PlayersAlive() const;

    float PlayersTopX() const;

    float PlayersMinX() const;

    float PlayersTopY() const;

    float PlayersMinY() const;

    ObjectPool<Bullet> *GetDefaultBullets() const {
        return default_bullets;
    }

    ObjectPool<Bullet> *GetFireBullets() const {
        return fire_bullets;
    }

    ObjectPool<Bullet> *GetMachineGunBullets() const {
        return machine_gun_bullets;
    }

    ObjectPool<Bullet> *GetSpreadBullets() const {
        return spread_bullets;
    }

    ObjectPool<Bullet> *GetLaserBullets() const {
        return laser_bullets;
    }

    ObjectPool<Bullet> *GetEnemyBullets() const {
        return enemy_bullets;
    }

    void Receive(Message m) override;

    const std::string &GetLevelName() const;

    int GetLevelIndex() const;

    [[nodiscard]] int GetLevelWidth() const {
        return levelWidth;
    }

    float GetTimeSinceComplete();

    int GetPlayerColliderLayer() {
        return m_enemyBulletsCollisionCheckLayer;
    }

    int GetPlayerColliderCheckLayer() {
        return m_enemyBulletsCollisionLayer;
    }

    const std::vector<PlayerControl *> &GetPlayerControls() const {
        return playerControls;
    }

private:
    void CreateBulletPools();

    void PreloadSounds();

    template<typename T>
    ObjectPool<Bullet> *CreatePlayerBulletPool(int num_bullets, const AnimationRenderer::Animation &animation,
                                               const Box &box);

    void CreatePlayers(short num_players, PlayerStats *stats);

protected:
    virtual Player *CreatePlayer(int index, PlayerStats *stats) = 0;

    int m_playerBulletsCollisionLayer = NPCS_COLLISION_LAYER, m_playerBulletsCollisionCheckLayer = -1,
            m_enemyBulletsCollisionLayer = PLAYER_COLLISION_LAYER, m_enemyBulletsCollisionCheckLayer = -1;
};

#endif //CONTRA_LEVEL_H
