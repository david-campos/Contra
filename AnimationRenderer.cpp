//
// Created by david on 4/2/20.
//

#include "AnimationRenderer.h"
#include "consts.h"

#include <utility>

void AnimationRenderer::Update(float dt) {
    if (!go->enabled || !enabled || !sprite || !m_currentAnimation)
        return;

    if (playing) {
        m_currentTime += dt;
    }

    int frame = floor(m_currentTime / m_currentAnimation->speed);
    // Check to loop the animation
    while (frame >= m_currentAnimation->frames) {
        if (m_currentAnimation->stop == DONT_STOP) {
            m_currentTime -= (float) frame * m_currentAnimation->speed;
            frame = floor(m_currentTime / m_currentAnimation->speed);
        } else {
            Pause();
            if (m_currentAnimation->stop == STOP_AND_FIRST) {
                m_currentTime = 0;
                frame = 0;
            } else {
                frame = m_currentAnimation->frames - 1;
                m_currentTime = (float) frame * m_currentAnimation->speed;
            }
        }
    }
    // Flip the anchor shift in x if we are mirroring horizontally (so the shift is correct)
    int x_shift = (mirrorHorizontal
                   ? m_currentAnimation->frame_w - m_currentAnimation->anchor_x
                   : m_currentAnimation->anchor_x
                  ) * PIXELS_ZOOM;
    sprite->draw(
            int(round(go->position.x - *camera_x)) - x_shift,
            int(round(go->position.y)) - m_currentAnimation->anchor_y * PIXELS_ZOOM,
            m_currentAnimation->frame_w * PIXELS_ZOOM, m_currentAnimation->frame_h * PIXELS_ZOOM,
            m_currentAnimation->start_x + frame * m_currentAnimation->frame_w,
            m_currentAnimation->start_y,
            m_currentAnimation->frame_w, m_currentAnimation->frame_h,
            mirrorHorizontal
    );
}

int AnimationRenderer::AddAnimation(AnimationRenderer::Animation animation) {
    long saved_index = -1;
    if (m_currentAnimation) {
        saved_index = m_currentAnimation - &m_animations[0];
    }
    m_animations.push_back(animation);
    if (saved_index < 0) {
        m_currentAnimation = &m_animations[0];
    } else {
        m_currentAnimation = &m_animations[saved_index];
    }
    return (int) m_animations.size() - 1;
}

void AnimationRenderer::CurrentAndPause(int index) {
    if (index < 0 || index >= m_animations.size()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "AnimationRenderer::PlayAnimation: Invalid animation %d", index);
        return;
    }
    if (m_currentAnimation != &m_animations[index]) {
        m_currentAnimation = &m_animations[index];
        m_currentTime = 0.f;
        Pause();
    }
}

void AnimationRenderer::PlayAnimation(int index) {
    if (index < 0 || index >= m_animations.size()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "AnimationRenderer::PlayAnimation: Invalid animation %d", index);
        return;
    }
    if (m_currentAnimation != &m_animations[index]) {
        m_currentAnimation = &m_animations[index];
        m_currentTime = 0.f;
    }
    Play();
}

bool AnimationRenderer::IsCurrent(int animationIndex) {
    return m_currentAnimation == &m_animations[animationIndex];
}

int AnimationRenderer::FindAnimation(std::string name) {
    for (int i = 0; i < m_animations.size(); i++) {
        if (m_animations[i].name == name) {
            return i;
        }
    }
    return -1;
}

void AnimationRenderer::Stop() {
    Pause();
    m_currentTime = 0;
}

void AnimationRenderer::Play(int frame) {
    GoToFrame(frame);
    playing = true;
}

void AnimationRenderer::Pause() {
    playing = false;
}

AnimationRenderer::Animation AnimationRenderer::GetCurrentAnimation() const {
    return *m_currentAnimation;
}

void AnimationRenderer::GoToFrame(int frame) {
    if (frame > 0) {
        m_currentTime = (float) (frame % m_currentAnimation->frames) * m_currentAnimation->speed;
    }
}

