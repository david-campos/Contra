//
// Created by david on 3/3/20.
//

#ifndef CONTRA_LEVEL_COMPONENT_H
#define CONTRA_LEVEL_COMPONENT_H

#include "../../kernel/component.h"
#include "level.h"

class LevelComponent : public Component {
protected:
    Level *level;
public:
    virtual void Create(Level *level, GameObject *go) {
        Component::Create(level, go);
        this->level = level;
    }
};

#endif //CONTRA_LEVEL_COMPONENT_H
