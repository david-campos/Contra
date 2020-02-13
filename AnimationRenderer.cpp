//
// Created by david on 4/2/20.
//

#include "AnimationRenderer.h"
#include "consts.h"

#include <utility>

void AnimationRenderer::Update(float dt) {
    if (!go->IsEnabled() || !enabled || !sprite || !m_currentAnimation)
        return;

    if (playing) {
        m_currentTime += m_goingForward ? dt : -dt;
    }

    int frame = floor(m_currentTime / m_currentAnimation->speed);

    // Check if it is over the top or bellow zero to loop the animation
    float animation_duration = (float) m_currentAnimation->frames * m_currentAnimation->speed;
    while (frame >= m_currentAnimation->frames || frame < 0) {
        if (m_currentAnimation->stop == DONT_STOP) {
            float modulo = abs(fmod(m_currentTime, animation_duration));
            if (m_goingForward) {
                m_currentTime = modulo;
            } else {
                m_currentTime = animation_duration - modulo;
            }
        } else if (m_currentAnimation->stop == BOUNCE || m_currentAnimation->stop == BOUNCE_AND_STOP) {
            if (!m_goingForward && m_currentAnimation->stop == BOUNCE_AND_STOP) {
                Pause();
                m_currentTime = 0;
            } else {
                float modulo = fmod(m_currentTime, animation_duration);
                m_currentTime = m_currentTime < 0
                                ? -modulo + m_currentAnimation->speed // We add speed to avoid repeating frame 0
                                : animation_duration - modulo -
                                  m_currentAnimation->speed; // We subtract speed to avoid repeating last frame
                m_goingForward = !m_goingForward;
            }
        } else { // Stop and last or stop and first
            Pause();
            if (m_currentAnimation->stop == STOP_AND_FIRST) {
                m_currentTime = 0;
            } else {
                m_currentTime = (float) (m_currentAnimation->frames - 1) * m_currentAnimation->speed;
            }
        }
        frame = floor(m_currentTime / m_currentAnimation->speed);
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
    engine->fillSquare(round(go->position.x - 2 - *camera_x),
            round(go->position.y) - 2, 4, {255, 0, 0});
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

void AnimationRenderer::CurrentAndPause(int index, bool forward) {
    if (index < 0 || index >= m_animations.size()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "AnimationRenderer::PlayAnimation: Invalid animation %d", index);
        return;
    }
    if (m_currentAnimation != &m_animations[index]) {
        m_currentAnimation = &m_animations[index];
        m_currentTime = 0.f;
        m_goingForward = forward;
        Pause();
    }
}

void AnimationRenderer::PlayAnimation(int index, bool forward) {
    if (index < 0 || index >= m_animations.size()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "AnimationRenderer::PlayAnimation: Invalid animation %d", index);
        return;
    }
    if (m_currentAnimation != &m_animations[index]) {
        m_currentAnimation = &m_animations[index];
        m_currentTime = forward ? 0.f : (float) m_currentAnimation->frames * m_currentAnimation->speed;
        Play(-1, forward);
    }
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

void AnimationRenderer::Play(int frame, bool forward) {
    GoToFrame(frame);
    m_goingForward = forward;
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

