//
// Created by david on 8/2/20.
//

#include "canons.h"
#include "../../components/collision/BoxCollider.h"
#include "bullets.h"

void CanonBehaviour::OnCollision(const CollideComponent &collider) {
    if (m_life > 0 && m_state == SHOWN) {
        auto *bullet = collider.GetGameObject()->GetComponent<BulletBehaviour *>();
        if (bullet && !bullet->IsKilled()) {
            m_life -= bullet->GetDamage();
            bullet->Kill();

            if (m_life <= 0) {
                go->Send(m_scoreGiven == 100 ? SCORE1_100 : SCORE1_500);
                level->GetSound(SOUND_ENEMY_DEATH)->Play(1);
            } else {
                level->GetSound(SOUND_ENEMY_HIT)->Play(1);
            }
        }
    }
}

void CanonBehaviour::Fire() {
    // Grab the bullet from the pool
    auto *bullet = level->GetEnemyBullets()->FirstAvailable();
    if (bullet != nullptr) {
        float rad = -0.5236f * (float) m_dir;
        Vector2D dir = Vector2D(cosf(rad), -sinf(rad));
        bullet->Init(go->position + dir * 13 * PIXELS_ZOOM,
                dir, ENEMY_BULLET_SPEED * PIXELS_ZOOM); // Notice our system has y inverted
        level->AddGameObject(bullet, RENDERING_LAYER_BULLETS);
    }
}

void CanonBehaviour::Update(float dt) {
    if (m_life == 0) {
        if (!m_animator->IsCurrent(animDie)) {
            m_animator->PlayAnimation(animDie);
        } else if (!m_animator->IsPlaying()) {
            go->MarkToRemove();
        }
        return;
    }
    auto *closestPlayer = GetClosestPlayer();
    Vector2D player_dir = GetPlayerDir(closestPlayer);
    switch (m_state) {
        case HIDDEN:
            UpdateHidden(closestPlayer, player_dir, dt);
            break;
        case SHOWING:
            UpdateShowing(closestPlayer, player_dir, dt);
            break;
        case SHOWN:
            UpdateShown(closestPlayer, player_dir, dt);
            break;
        case HIDING:
            UpdateHiding(closestPlayer, player_dir, dt);
            break;
    }
}

void CanonBehaviour::UpdateHidden(const PlayerControl *playerControl, const Vector2D &player_dir, float dt) {
    m_animator->PlayAnimation(animHidden);
    if (player_dir.x > -WINDOW_WIDTH / 2 + 16 * PIXELS_ZOOM && player_dir.x < 0) {
        m_animator->PlayAnimation(animShowing);
        m_state = SHOWING;
    }
}

void CanonBehaviour::UpdateShowing(const PlayerControl *playerControl, const Vector2D &player_dir, float dt) {
    if (!m_animator->IsPlaying()) {
        m_dir = std::max(std::min(DirToInt(player_dir), m_maxDir), m_minDir);
        m_animator->PlayAnimation(animDirsFirst + m_dir - m_minDir);
        m_state = SHOWN;
        m_currentDirTime = 0;
    }
}

void CanonBehaviour::UpdateShown(const PlayerControl *playerControl, const Vector2D &player_dir, float dt) {
    if (player_dir.x > WINDOW_WIDTH / 2 - 25 * PIXELS_ZOOM) {
        m_animator->PlayAnimation(animShowing, false);
        m_state = HIDING;
        return;
    }

    m_currentDirTime += dt;
    int target_dir = DirToInt(player_dir);
    bool is_default = target_dir > m_maxDir || target_dir < m_minDir;
    if (is_default) {
        target_dir = m_defaultDir;
    }
    if (m_currentDirTime > m_rotationInterval) {
        m_currentDirTime -= m_rotationInterval;
        int diff = target_dir - m_dir;
        // Turn
        if (diff != 0) {
            int clockwise_add;
            if (diff > 6 or (diff <= 0 && diff > -6)) clockwise_add = -1;
            else clockwise_add = 1;
            m_dir = m_dir + clockwise_add;
            if (m_dir < 0) m_dir += 12;
            else if (m_dir >= 12) m_dir -= 12;
            m_animator->PlayAnimation(animDirsFirst + m_dir - m_minDir);
        }
    }

    if (m_burstRemainingCooldown > 0) {
        m_burstRemainingCooldown -= dt;
    }
    if (m_fireRemainingCooldown > 0) {
        m_fireRemainingCooldown -= dt;
    } else if ((m_shotBulletsInBurst < m_burstLength || m_burstRemainingCooldown <= 0)
               && m_dir == target_dir && playerControl->IsAlive() && !is_default) {
        if (m_burstRemainingCooldown <= 0) {
            m_shotBulletsInBurst = 0; // New burst
            m_burstRemainingCooldown = m_burstCooldown;
        }
        Fire();
        m_fireRemainingCooldown = m_fireCooldown;
        m_shotBulletsInBurst++;
    }
}

void CanonBehaviour::UpdateHiding(const PlayerControl *playerControl, const Vector2D &player_dir, float dt) {
    if (!m_animator->IsPlaying()) m_state = HIDDEN;
}

void CanonBehaviour::Create(Level *level, GameObject *go, int min_dir, int max_dir, int default_dir,
                            float rotation_interval, int burst_length, float burst_cooldown, float shoot_cooldown) {
    LevelComponent::Create(level, go);
    m_rotationInterval = rotation_interval;
    m_minDir = min_dir;
    m_maxDir = max_dir;
    m_defaultDir = default_dir;
    m_fireCooldown = shoot_cooldown;
    m_burstCooldown = burst_cooldown;
    m_burstLength = burst_length;
}

void GulcanBehaviour::UpdateHidden(const PlayerControl *playerControl, const Vector2D &player_dir, float dt) {
    m_animator->enabled = false;
    if (player_dir.x > -WINDOW_WIDTH / 2 + 16 * PIXELS_ZOOM && player_dir.x < 0) {
        m_animator->enabled = true;
        m_animator->PlayAnimation(animShowing);
        m_state = SHOWING;
    }
}

void RotatingCanon::Create(Level *level, const Vector2D &pos, int burst_length) {
    GameObject::Create();
    position = pos;
    auto *renderer = new AnimationRenderer();
    renderer->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
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
            "Opening", AnimationRenderer::STOP_AND_LAST});
    renderer->AddAnimation({
            92, 611, anim_speed, anim_len,
            30, 30, 15, 15,
            "Dying", AnimationRenderer::BOUNCE_AND_STOP});
    renderer->Play();
    auto *behaviour = new CanonBehaviour();
    behaviour->Create(level, this, 0, 11, 6, 1.f,
            burst_length, 2, 0.25);
    auto *collider = new BoxCollider();
    collider->Create(level, this, -10 * PIXELS_ZOOM, -10 * PIXELS_ZOOM,
            20 * PIXELS_ZOOM, 20 * PIXELS_ZOOM, -1, NPCS_COLLISION_LAYER);
    collider->SetListener(behaviour);

    AddComponent(behaviour);
    AddComponent(renderer);
    AddComponent(collider);
}

void Gulcan::Create(Level *level, const Vector2D &pos) {
    GameObject::Create();
    position = pos;
    auto *renderer = new AnimationRenderer();
    renderer->Create(level, this, level->GetSpritesheet(SPRITESHEET_ENEMIES));
    // Add all the directions as consecutive animations
    const int frame_side = 34;
    const int anim_len = 3;
    const int anims_per_line = 3;
    const float anim_speed = 0.15;
    for (int i = 0; i < 12; i++) {
        std::string anim = "Dir" + std::to_string(i);
        renderer->AddAnimation({
                1 + (i % anims_per_line) * anim_len * frame_side,
                280 + (i / anims_per_line * frame_side),
                anim_speed, anim_len,
                frame_side, frame_side, frame_side / 2, frame_side / 2,
                anim, AnimationRenderer::BOUNCE});
    }
    renderer->AddAnimation({
            1, 280, anim_speed, anim_len,
            frame_side, frame_side, frame_side / 2, frame_side / 2,
            "Closed", AnimationRenderer::BOUNCE});
    renderer->AddAnimation({
            205, 314, anim_speed, anim_len,
            frame_side, frame_side, frame_side / 2, frame_side / 2,
            "Opening", AnimationRenderer::STOP_AND_LAST});
    renderer->AddAnimation({
            92, 611, 0.15, 3,
            30, 30, 15, 15,
            "Dying", AnimationRenderer::BOUNCE_AND_STOP});
    renderer->Play();
    auto *behaviour = new GulcanBehaviour();
    behaviour->Create(level, this, 6, 8, 6, 0.5f,
            3, 1.5, 0.25);
    auto *collider = new BoxCollider();
    collider->Create(level, this, -12 * PIXELS_ZOOM, -15 * PIXELS_ZOOM,
            22 * PIXELS_ZOOM, 30 * PIXELS_ZOOM, -1, NPCS_COLLISION_LAYER);
    collider->SetListener(behaviour);

    AddComponent(behaviour);
    AddComponent(renderer);
    AddComponent(collider);
}
