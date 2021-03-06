//
// Created by david on 5/2/20.
//

#include "Gravity.h"
#include "../level/level.h"

void Gravity::Update(float dt) {
    m_onFloor = false;
    m_onWater = false;
    m_canFall = false;

    auto floor = level->GetLevelFloor().lock();

    float y_increment = m_velocity * dt;
    if (m_velocity < 0) {
        go->position = go->position + Vector2D(0, y_increment);
        m_velocity += m_acceleration * dt;
        return;
    }

    int next_y = (int) std::floor((go->position.y + y_increment) / PIXELS_ZOOM);
    int x = (int) std::floor(go->position.x / PIXELS_ZOOM);
    int y;
    if (floor) {
        // Check all the intermediate pixels to ensure it never misses even in low frame rate
        for (y = (int) std::floor(go->position.y / PIXELS_ZOOM); y <= next_y && !m_onFloor &&
                                                                 (!m_onWater || m_fallThroughWater); y++) {
            m_onFloor = m_onFloor || floor->IsFloor(x, y);
            m_onWater = m_onWater || floor->IsWater(x, y);
        }
        y--; // Correct y as the loop will over-increase 1

        if (!m_onFloor) {
            m_lettingFall = false;
        }

        if (!(m_onFloor or m_onWater) or m_lettingFall or (!m_onFloor && m_fallThroughWater)) {
            go->position = go->position + Vector2D(0, y_increment); // Falls free
            m_velocity += m_acceleration * dt;
        } else if (!m_lettingFall) {
            m_canFall = floor->ShouldBeAbleToFall(x, y);

            if (!m_canFall || !m_fallThroughCanFall) {
                // Put on the floor pixel
                go->position = Vector2D(go->position.x, y * PIXELS_ZOOM);
                m_velocity = 0;
            } else {
                go->position = go->position + Vector2D(0, y_increment); // Falls free (yep, again)
                m_velocity += m_acceleration * dt;
            }
        }
    } else {
        m_onWater = false;
        m_canFall = false;
        m_onFloor = go->position.y + y_increment >= m_baseFloor - 0.001;
        if (m_onFloor) {
            go->position = Vector2D(go->position.x, m_baseFloor);
            m_velocity = 0;
        } else {
            go->position = go->position + Vector2D(0, y_increment); // Falls free
            m_velocity += m_acceleration * dt;
        }
    }
}

bool Gravity::IsOnFloor() const {
    return m_onFloor && m_velocity == 0;
}

float Gravity::GetAcceleration() const {
    return m_acceleration;
}

void Gravity::SetAcceleration(float mAcceleration) {
    m_acceleration = mAcceleration;
}

void Gravity::LetFall() {
    m_lettingFall = true;
}

bool Gravity::IsOnWater() const {
    return m_onWater && m_velocity == 0;
}

bool Gravity::IsOnAir() const {
    return not(m_onWater or m_onFloor) or m_velocity != 0;
}

bool Gravity::CanFall() const {
    return m_canFall;
}

void Gravity::SetBaseFloor(float mBaseFloor) {
    m_baseFloor = mBaseFloor;
}

