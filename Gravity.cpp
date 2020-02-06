//
// Created by david on 5/2/20.
//

#include "Gravity.h"
#include "consts.h"

void Gravity::Update(float dt) {
    auto floor = m_floor.lock();
    if (!floor) {
        return;
    }

    m_onFloor = false;
    m_onWater = false;
    m_canFall = false;

    float y_increment = m_velocity * dt;
    if (m_velocity < 0) {
        go->position = go->position + Vector2D(0, y_increment);
        m_velocity += m_acceleration * dt;
        return;
    }

    // Check all the intermediate pixels to ensure it never misses even in low frame rate
    int next_y = (int) std::floor((go->position.y + y_increment) / PIXELS_ZOOM);
    int x = (int) std::floor(go->position.x / PIXELS_ZOOM);
    int y;
    for (y = (int) std::floor(go->position.y / PIXELS_ZOOM); y <= next_y && !m_onFloor; y++) {
        m_onFloor = m_onFloor || floor->IsFloor(x, y);
        m_onWater = m_onWater || floor->IsWater(x, y);
    }
    y -= 1; // Correct y as the loop will over-increase 1

    if (!m_onFloor) {
        m_lettingFall = false;
    }

    if (!(m_onFloor or m_onWater) || m_lettingFall) {
        go->position = go->position + Vector2D(0, y_increment); // Falls free
        m_velocity += m_acceleration * dt;
    } else if (!m_lettingFall) {
        // Put on the floor pixel
        go->position = Vector2D(go->position.x, y * PIXELS_ZOOM);
        m_velocity = 0;
        m_canFall = floor->ShouldBeAbleToFall(x, y);
    }
}

bool Gravity::IsOnFloor() const {
    return m_onFloor && m_velocity == 0;
}

float Gravity::getAcceleration() const {
    return m_acceleration;
}

void Gravity::setAcceleration(float mAcceleration) {
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

