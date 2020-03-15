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
    // All the player bullet pools are arrays, the enemy_bullets is just one object pool
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

    void Update(float dt) final;

    /** Indicates whether the player has complete the level */
    [[nodiscard]] bool IsComplete() const {
        return complete;
    }

    /**
     * Gets the sprite for the exploding bridges, as it is a separated file.
     * This sprite is lazy-loaded so it will not be created until requested for the first time.
     * @return
     */
    std::shared_ptr<Sprite> GetBridgeSprite() {
        if (!bridge_sprite) {
            bridge_sprite = std::shared_ptr<Sprite>(m_engine->createSprite("data/bridge.png"));
        }
        return bridge_sprite;
    }

    void Destroy() override;

    /**
     * Gets the specified sound from the preloaded sound effects. Use the SOUND_* macros
     * to get the different IDs.
     */
    SoundEffect *GetSound(int id) const {
        return shared_sounds.at(id);
    }

    /**
     * Gets the specified spritesheet from the preloaded ones. Use the SPRITESHEET_*
     * macros to get the different IDs.
     */
    const std::shared_ptr<Sprite> &GetSpritesheet(int id) const {
        return spritesheets->at(id);
    }

    const std::weak_ptr<Floor> GetLevelFloor() const {
        return level_floor;
    }

    /**
     * Gets the alive player that is closest to the given position.
     * @return nullptr if there are no alive players
     */
    Player *GetClosestPlayer(const Vector2D &position) const;

    /**
     * Gets the player control of the closest player to the given point
     * @param position Point for which to look for the closest player
     * @param only_before If set to true, any position with lower x than the given
     * position has preference over a position with higher x value than the given one
     * (prefers to select a player "in front"/"before" the given point).
     * @return nullptr if there are no alive players
     */
    PlayerControl *GetClosestPlayerControl(const Vector2D &position, bool only_before = false) const;

    /**
     * @return the number of players which are currently alive
     */
    short PlayersAlive() const;

    /**
     * @return the highest x value of the positions of the alive players
     */
    float PlayersTopX() const;

    /**
     * @return the lowest x value of the positions of the alive players
     */
    float PlayersMinX() const;

    /**
     * @param alive_players Out parameter, if a pointer is provided the value
     * of the pointed memory will be set to whether there are players alive or not
     * @return the highest y value of the positions of the alive players
     */
    float PlayersTopY(bool *alive_players = nullptr) const;

    /**
     * @param alive_players Out parameter, if a pointer is provided the value
     * of the pointed memory will be set to whether there are players alive or not
     * @return the lowest y value of the positions of the alive players
     */
    float PlayersMinY(bool *alive_players = nullptr) const;

    /** Gets the bullet pool for the rifle for the selected player */
    ObjectPool<Bullet> *GetDefaultBullets(int player_idx) const {
        if (player_idx < players.size())
            return default_bullets + player_idx;
        else
            return nullptr;
    }

    /** Gets the bullet pool for the fire gun for the selected player */
    ObjectPool<Bullet> *GetFireBullets(int player_idx) const {
        if (player_idx < players.size())
            return fire_bullets + player_idx;
        else
            return nullptr;
    }

    /** Gets the bullet pool for the machine gun for the selected player */
    ObjectPool<Bullet> *GetMachineGunBullets(int player_idx) const {
        if (player_idx < players.size())
            return machine_gun_bullets + player_idx;
        else
            return nullptr;
    }

    /** Gets the bullet pool for the spread for the selected player */
    ObjectPool<Bullet> *GetSpreadBullets(int player_idx) const {
        if (player_idx < players.size())
            return spread_bullets + player_idx;
        else
            return nullptr;
    }

    /** Gets the bullet pool for the laser for the selected player */
    ObjectPool<Bullet> *GetLaserBullets(int player_idx) const {
        if (player_idx < players.size())
            return laser_bullets + player_idx;
        else
            return nullptr;
    }

    /** Gets the bullet pool for the enemy bullets */
    ObjectPool<Bullet> *GetEnemyBullets() const {
        return enemy_bullets;
    }

    void Receive(Message m) override;

    const std::string &GetLevelName() const;

    int GetLevelIndex() const;

    /**
     * Gets the width of the level in art scale (without zoom)
     */
    [[nodiscard]] int GetLevelWidth() const {
        return levelWidth;
    }

    /**
     * Seconds since the player completed the level, useful for final animations
     */
    float GetTimeSinceComplete();

    /**
     * Gets the player collision layer for this level, it may vary depending on the type of the level
     */
    int GetPlayerColliderLayer() {
        return m_enemyBulletsCollisionCheckLayer;
    }

    const std::vector<PlayerControl *> &GetPlayerControls() const {
        return playerControls;
    }

private:
    /** Creates the bullet pools for the game */
    void CreateBulletPools(int num_players);
    /** Preloads the necessary sound effects */
    void PreloadSounds();

    /**
     * Creates bullet pools for all the players with the desired configuration
     * @tparam T the behaviour component to use, should be an extension of BulletBehaviour
     * @param num_bullets Number of bullets in the pool
     * @param animation Bullet main animation configuration
     * @param box Box for the bullet box collider
     * @param num_players Number of players
     * @return
     */
    template<typename T>
    ObjectPool<Bullet> *CreatePlayersBulletPools(int num_bullets, const AnimationRenderer::Animation &animation,
                                                 const Box &box, int num_players);

    /** Creates the players for the level */
    void CreatePlayers(short num_players, PlayerStats *stats);

protected:
    /** Should be implemented by child classes to customize the player creation */
    virtual Player *CreatePlayer(int index, PlayerStats *stats) = 0;

    /**
     * Called at the end of each frame update in case the level is still going on to perform level-type specefic
     * updates.
     */
    virtual void SubUpdate(float dt) = 0;
    int m_playerBulletsCollisionLayer = NPCS_COLLISION_LAYER, m_playerBulletsCollisionCheckLayer = -1,
            m_enemyBulletsCollisionLayer = PLAYER_COLLISION_LAYER, m_enemyBulletsCollisionCheckLayer = -1;
};

#endif //CONTRA_LEVEL_H
