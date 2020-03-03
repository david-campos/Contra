//
// Created by david on 3/3/20.
//

#ifndef CONTRA_RENDERCOMPONENT_H
#define CONTRA_RENDERCOMPONENT_H

#include <memory>
#include "../../kernel/component.h"
#include "../../kernel/avancezlib.h"

class RenderComponent : public Component {
protected:
    std::shared_ptr<Sprite> sprite;
public:
    virtual void Create(BaseScene *scene, GameObject *go, std::shared_ptr<Sprite> sprite) {
        Component::Create(scene, go);
        this->sprite = std::move(sprite);
    }

    void Destroy() override {
        sprite.reset();
    }

    std::shared_ptr<Sprite> GetSprite() { return sprite; }
};

#endif //CONTRA_RENDERCOMPONENT_H
