//
// Created by david on 5/2/20.
//

#ifndef CONTRA_GRAVITY_H
#define CONTRA_GRAVITY_H


#include "component.h"
#include "floor.h"
#include "consts.h"

class Gravity: public Component {
private:
    float m_velocity;
    float m_acceleration = 350 * PIXELS_ZOOM;
    bool m_onFloor, m_onWater, m_canFall;
    bool m_lettingFall;
    bool m_fallThroughWater; // Used after death
public:
    void Update(float dt) override;
    void AddVelocity(float velocity) {
        m_velocity += velocity;
    }
    void SetVelocity(float velocity) {
        m_velocity = velocity;
    }
    [[nodiscard]] float GetVelocity() const {
        return m_velocity;
    }
    void SetFallThoughWater(const bool value) { m_fallThroughWater = value; }
    // Let's the game object fall until it leaves the current floor
    void LetFall();
    [[nodiscard]] bool CanFall() const;

    [[nodiscard]] float GetAcceleration() const;
    void SetAcceleration(float acceleration);

    [[nodiscard]] bool IsOnWater() const;
    [[nodiscard]] bool IsOnFloor() const;
    [[nodiscard]] bool IsOnAir() const;
};


#endif //CONTRA_GRAVITY_H
