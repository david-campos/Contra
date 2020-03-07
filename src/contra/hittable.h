//
// Created by david on 5/3/20.
//

#ifndef CONTRA_HITTABLE_H
#define CONTRA_HITTABLE_H

#include "level/level_component.h"

class Killable {
public:
    virtual void Kill() = 0;
};

class Hittable {
public:
    virtual void Hit() = 0;

    virtual bool CanBeHit() = 0;

    virtual bool HitLast() { return false; }
};

#endif //CONTRA_HITTABLE_H
