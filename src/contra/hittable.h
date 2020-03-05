//
// Created by david on 5/3/20.
//

#ifndef CONTRA_HITTABLE_H
#define CONTRA_HITTABLE_H

#include "level/level_component.h"

class Hittable {
public:
    virtual void Hit() = 0;
};

#endif //CONTRA_HITTABLE_H
