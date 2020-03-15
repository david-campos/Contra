//
// Created by david on 5/3/20.
//

#ifndef CONTRA_HITTABLE_H
#define CONTRA_HITTABLE_H

#include "level/level_component.h"

/**
 * The Killable class provides an interface for all the objects that can be "killed"
 */
class Killable {
public:
    virtual void Kill() = 0;
};

/**
 * The Hittable class provides an interface for all the objects that can be hit by a bullet
 */
class Hittable {
public:
    virtual void Hit() = 0;

    virtual bool CanBeHit() = 0;

    /**
     * Return true from this method if the object is expected to be hit later than other
     * possible colliding objects.
     */
    virtual bool HitLast() { return false; }
};

#endif //CONTRA_HITTABLE_H
