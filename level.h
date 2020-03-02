//
// Created by david on 14/2/20.
//

#ifndef CONTRA_LEVEL_H
#define CONTRA_LEVEL_H


#include <queue>
#include <random>
#include "game_object.h"
#include "floor.h"
#include "avancezlib.h"
#include "consts.h"
#include "grid.h"
#include "object_pool.h"
#include "AnimationRenderer.h"
#include "pickup_types.h"
#include "scene.h"

class Player;
class PlayerControl;
struct Box;
class Bullet;
class BulletBehaviour;
class PickUpHolderBehaviour;

class Level : public BaseScene {
    struct game_objects_comp_x {
        bool operator()(const std::pair<GameObject *, short>lhs, const std::pair<GameObject *, short> rhs) const {
            return lhs.first->position.x > rhs.first->position.x;
        }
    };

    std::shared_ptr<Sprite> spritesheet;
    std::shared_ptr<Sprite> enemies_spritesheet;
    std::shared_ptr<Sprite> pickups_spritesheet;
    std::shared_ptr<Sprite> bridge_sprite; // Loaded on demand
    std::unordered_map<int, SoundEffect*> shared_sounds;
    std::shared_ptr<Floor> level_floor;
    std::vector<Player *> players;
    std::vector<PlayerControl*> playerControls;
    std::priority_queue<std::pair<GameObject *, short>, std::deque<std::pair<GameObject *, short>>, game_objects_comp_x> not_found_enemies;
    float next_enemy_x;
    ObjectPool<Bullet> *default_bullets, *fire_bullets,
            *machine_gun_bullets, *spread_bullets, *laser_bullets, *enemy_bullets;
    bool complete;
    int level_width;
    std::string levelName;
    int levelIndex;

    std::random_device rd;
    std::mt19937 m_mt = std::mt19937(rd());
    std::uniform_real_distribution<float> m_random_dist = std::uniform_real_distribution<float>(0.f, 1.f);
public:
    void Create(const std::string &folder, const std::shared_ptr<Sprite> &sprite_sheet,
                const std::shared_ptr<Sprite> &enemies_spritesheet, const std::shared_ptr<Sprite> &pickups_spritesheet,
                short num_players, AvancezLib *engine);
    void PreloadSounds();
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

    void AddNotFoundEnemy(GameObject* const game_object, const short layer) {
        not_found_enemies.push(std::pair<GameObject*, short>(game_object, layer));
    }

    void Destroy() override;

    SoundEffect* GetSound(int id) const {
        return shared_sounds.at(id);
    }

    const std::shared_ptr<Sprite> &GetSpritesheet() const {
        return spritesheet;
    }

    const std::shared_ptr<Sprite> &GetEnemiesSpritesheet() const {
        return enemies_spritesheet;
    }

    const std::shared_ptr<Sprite> &GetPickupsSpritesheet() const {
        return pickups_spritesheet;
    }

    const std::weak_ptr<Floor> GetLevelFloor() const {
        return level_floor;
    }

    Player *GetClosestPlayer(const Vector2D& position) const;
    /**
     * Gets the player control of the closest player to the given point
     * @param position Point for which to look for the closest player
     * @param only_before If set to true, any position with lower x than the given
     * position has preference over a position with higher x value than the given one
     * (prefers to select a player "in front"/"before" the given point).
     * @return
     */
    PlayerControl *GetClosestPlayerControl(const Vector2D& position, bool only_before = false) const;
    short PlayersAlive() const;
    float PlayersTopX() const;
    float PlayersMinX() const;

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

    [[nodiscard]] int GetLevelWidth() const {
        return level_width;
    }

    void Receive(Message m) override;

    const std::string &GetLevelName() const;

    int GetLevelIndex() const;

private:
    void CreateBulletPools();

    template<typename T>
    ObjectPool<Bullet> *CreatePlayerBulletPool(int num_bullets, const AnimationRenderer::Animation &animation,
                                               const Box &box);
    void CreatePlayers(short num_players);
    /**
     * @param behaviour Create will be called, no need to create it first
     */
    void CreateAndAddPickUpHolder(const PickUpType &type, const Vector2D &position, PickUpHolderBehaviour *behaviour,
                                  const Box& box, AnimationRenderer **renderer_out);
    /**
     * Creates the defense wall of the end of stage 1
     */
    void CreateDefenseWall();
    ObjectPool<Bullet>* CreateBlasterBulletPool();
};

#endif //CONTRA_LEVEL_H
