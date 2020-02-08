//
// Created by david on 8/2/20.
//

#include "Tank.h"
#include "AnimationRenderer.h"

void Tank::Create(AvancezLib *engine, std::set<GameObject *> *game_objects,
                  const std::shared_ptr<Sprite> &enemies_spritesheet,
                  float *camera_x, const Vector2D &pos, Player *player, ObjectPool<Bullet> *bullet_pool) {
    GameObject::Create();
    position = pos;
    auto *renderer = new AnimationRenderer();
    renderer->Create(engine, this, game_objects, enemies_spritesheet, camera_x);
    // Add all the directions as consecutive animations
    const int frame_side = 34;
    const int anim_len = 3;
    const int anims_per_line = 3;
    const float anim_speed = 0.15;
    for (int i = 0; i < 12; i++) {
        std::string anim = "Dir" + std::to_string(i);
        renderer->AddAnimation({
                1 + (i % anims_per_line) * anim_len * frame_side,
                144 + (i / anims_per_line * frame_side),
                anim_speed, anim_len,
                frame_side, frame_side, frame_side / 2, frame_side / 2,
                anim, AnimationRenderer::BOUNCE});
    }
    renderer->AddAnimation({
            1, 110, anim_speed, anim_len,
            frame_side, frame_side, frame_side / 2, frame_side / 2,
            "Closed", AnimationRenderer::BOUNCE});
    renderer->AddAnimation({
            1 + anim_len * frame_side, 110, anim_speed, anim_len,
            frame_side, frame_side, frame_side / 2, frame_side / 2,
            "Opening", AnimationRenderer::STOP_AND_FIRST});
    renderer->Play();
    auto *behaviour = new TankBehaviour();
    behaviour->Create(engine, this, game_objects, player, bullet_pool);

    AddComponent(behaviour);
    AddComponent(renderer);
}

void TankBehaviour::Update(float dt) {
    Vector2D player_dir = m_player->position - Vector2D(0, 12) - go->position; // Add 12 bc position is the feet
    switch (m_state) {
        case HIDDEN:
            m_animator->PlayAnimation(animHidden);
            if (player_dir.x > -WINDOW_WIDTH / 2 + 16 * PIXELS_ZOOM && player_dir.x < 0) {
                m_animator->PlayAnimation(animShowing);
                m_state = SHOWING;
            }
            break;
        case SHOWING:
            if (!m_animator->IsPlaying()) {
                m_dir = DirToInt(player_dir);
                m_animator->PlayAnimation(animDirsFirst + m_dir);
                m_state = SHOWN;
                m_currentDirTime = 0;
            }
            break;
        case SHOWN:
            if (player_dir.x > WINDOW_WIDTH / 2 - 17 * PIXELS_ZOOM) {
                m_animator->PlayAnimation(animShowing);
                m_state = HIDDEN;
                break;
            }
            m_currentDirTime += dt;
            if (m_currentDirTime > 1.0f) { // Update each second
                m_currentDirTime -= 1.0f;
                int target_dir = DirToInt(player_dir);
                int diff = target_dir - m_dir;
                // Turn
                if (diff != 0) {
                    int clockwise_add;
                    if (diff > 6 or (diff <= 0 && diff > -6)) clockwise_add = -1;
                    else clockwise_add = 1;
                    m_dir = m_dir + clockwise_add;
                    if (m_dir < 0) m_dir += 12;
                    else if (m_dir >= 12) m_dir -= 12;
                    m_animator->PlayAnimation(animDirsFirst + m_dir);
                }
                if (m_dir == target_dir) {
                    Fire();
                }
            }
            break;
    }
}

void TankBehaviour::Create(AvancezLib *engine, GameObject *go, std::set<GameObject *> *game_objects, Player *player,
        ObjectPool<Bullet>* bullet_pool) {
    Component::Create(engine, go, game_objects);
    m_player = player;
    m_bulletPool = bullet_pool;
}

int TankBehaviour::DirToInt(const Vector2D &dir) const {
    return (12 - // Counter-clockwise, preserving 0
           int(fmod( // Between 0 and 3.1416
                   atan2(-dir.y, dir.x) + 6.2832 + 0.2618, // ensure positive and add 15º
                   6.2832) / 0.5236)) % 12;  //0.5236rad = 30deg
}

void TankBehaviour::Fire() {
    // Grab the bullet from the pool
    auto *bullet = m_bulletPool->FirstAvailable();
    if (bullet != nullptr) {
        float rad = -0.5236f * (float) m_dir;
        bullet->Init(go->position, Vector2D(cosf(rad), -sinf(rad)), 160); // Notice our system has y inverted
        game_objects->insert(bullet);
    }
}
