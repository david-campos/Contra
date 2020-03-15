//
// Created by david on 5/2/20.
//

#ifndef CONTRA_GRAVITY_H
#define CONTRA_GRAVITY_H

#include "floor.h"
#include "../../consts.h"
#include "../level/level_component.h"

class Gravity : public LevelComponent {
private:
    float m_velocity;
    float m_acceleration = 350 * PIXELS_ZOOM;
    bool m_onFloor, m_onWater, m_canFall;
    bool m_lettingFall;
    bool m_fallThroughWater = false; // Used after death
    bool m_fallThroughCanFall = false;
    float m_baseFloor = 999999999;
public:
    void Update(float dt) override;

    /**
     * Adds vertical velocity, please keep in mind that our coordinate system
     * has inverted y (negative Y is up)
     */
    void AddVelocity(float velocity) {
        m_velocity += velocity;
    }

    /**
     * Replaces the vertical velocity, please keep in mind that our coordinate system
     * has inverted y (negative Y is up)
     */
    void SetVelocity(float velocity) {
        m_velocity = velocity;
    }

    [[nodiscard]] float GetVelocity() const {
        return m_velocity;
    }

    /**
     * Sets whether the gravity component should let the object fall through the water or stay on it
     */
    void SetFallThoughWater(const bool value) { m_fallThroughWater = value; }

    /**
     * Sets whether the gravity component should let the object automatically fall through floors the
     * player would be able to let herself fall through.
     * @param value
     */
    void SetFallThroughCanFall(const bool value) { m_fallThroughCanFall = value; }

    // Let's the game object fall until it leaves the current floor
    void LetFall();

    [[nodiscard]] bool CanFall() const;

    [[nodiscard]] float GetAcceleration() const;

    /**
     * Sets the acceleration applied by the component to the game object when it is falling.
     * @param acceleration
     */
    void SetAcceleration(float acceleration);

    /**
     * Sets a maximum Y value that is always going to be considered floor (so the object
     * will not fall through it)
     */
    void SetBaseFloor(float mBaseFloor);

    [[nodiscard]] float GetBaseFloor() const { return m_baseFloor; }

    [[nodiscard]] bool IsOnWater() const;

    [[nodiscard]] bool IsOnFloor() const;

    [[nodiscard]] bool IsOnAir() const;
};


#endif //CONTRA_GRAVITY_H
