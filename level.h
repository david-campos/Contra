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

class Player;
class PlayerControl;
struct Box;
class Bullet;
class PickUpHolderBehaviour;

class Level : public GameObject {
    struct game_objects_comp_x {
        bool operator()(const GameObject *lhs, const GameObject *rhs) const {
            return lhs->position.x > rhs->position.x;
        }
    };

    AvancezLib *engine;
    std::unique_ptr<Sprite> background;
    std::shared_ptr<Sprite> spritesheet;
    std::shared_ptr<Sprite> enemies_spritesheet;
    std::shared_ptr<Sprite> pickups_spritesheet;
    std::shared_ptr<Floor> level_floor;
    Player *player;
    PlayerControl *playerControl;
    std::set<GameObject *> *game_objects[RENDERING_LAYERS];
    std::priority_queue<GameObject *, std::deque<GameObject *>, game_objects_comp_x> not_found_enemies;
    std::queue<std::pair<GameObject*, int>> game_objects_to_add;
    float next_enemy_x;
    ObjectPool<Bullet> *default_bullets, *fire_bullets,
            *machine_gun_bullets, *spread_bullets, *laser_bullets, *enemy_bullets;
    Grid grid;
    float camera_x;
    int level_width;

    std::random_device rd;
    std::mt19937 m_mt = std::mt19937(rd());
    std::uniform_real_distribution<float> m_random_dist = std::uniform_real_distribution<float>(0.f, 1.f);
public:
    Level() {
        for (int i = 0; i < RENDERING_LAYERS; i++) {
            game_objects[i] = new std::set<GameObject *>();
        }
    }

    ~Level() {
        for (int i = 0; i < RENDERING_LAYERS; i++) {
            delete game_objects[i];
        }
    }

    void Create(const std::string &folder, const std::shared_ptr<Sprite> &sprite_sheet,
                const std::shared_ptr<Sprite> &enemies_spritesheet, const std::shared_ptr<Sprite> &pickups_spritesheet,
                AvancezLib *engine);

    void Init() override;
    void Update(float dt) override;

    [[nodiscard]] AvancezLib *GetEngine() const {
        return engine;
    }

    void AddGameObject(GameObject* const game_object, const int layer) {
        game_objects_to_add.push(std::pair<GameObject*, int>(game_object, layer));
    }

    /**
     * Just a convenient method in case someone is confused about how to safely remove objects.
     * Marks the object to be removed, GameObject::MarkToRemove can safely be used instead
     * @param game_object
     */
    void RemoveGameObject(GameObject *game_object) { game_object->MarkToRemove(); }
    /**
     * Removes immediately the object from the layer.
     * @warning Prefer Level::RemoveGameObject instead, as immediately removing an
     * object while iterating over the layer can cause memory issues.
     * @param game_object
     * @param layer
     */
    void RemoveImmediately(GameObject *game_object, const int layer) {
        game_objects[layer]->erase(game_object);
        if (game_object->onRemoval == DESTROY) {
            game_object->Destroy();
        }
    }

    /**
     * @return The x position of the camera, this corresponds to the exact left side of the same.
     */
    float GetCameraX() const {
        return camera_x;
    }

    void Destroy() override;

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

    Player *GetPlayer() const {
        return player;
    }

    PlayerControl *GetPlayerControl() const {
        return playerControl;
    }

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

    Grid* GetGrid() {
        return &grid;
    }

private:
    void CreateBulletPools();
    ObjectPool<Bullet> *CreatePlayerBulletPool(int num_bullets, const AnimationRenderer::Animation &animation,
                                               const Box &box);
    void CreatePlayer();
    /**
     * @param behaviour Create will be called, no need to create it first
     */
    void CreateAndAddPickUpHolder(const PickUpType &type, const Vector2D &position, PickUpHolderBehaviour *behaviour,
                                  const Box& box, AnimationRenderer **renderer_out);
};

#endif //CONTRA_LEVEL_H
