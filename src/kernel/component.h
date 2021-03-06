#pragma once

#include <SDL_log.h>
#include "game_object.h"


class BaseScene;

class Component {
protected:
    BaseScene *scene; // the scene reference
    GameObject *go;        // the game object this component is part of
public:
    virtual ~Component() {}

    virtual void Create(BaseScene *scene, GameObject *go) {
        this->scene = scene;
        this->go = go;
    }

    virtual void Init() {
        if (!go) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                    "Component::Init: No GameObject assigned, probably a missing Component::Create call?");
        }
    }

    /** Delegates into GameObject::GetComponent of the associated game object */
    template<typename T>
    T GetComponent() {
        return go->GetComponent<T>();
    }

    /** Hookup called when the associated game object has just been enabled */
    virtual void OnGameObjectEnabled() {}

    /** Hookup called when the associated game object is disabled */
    virtual void OnGameObjectDisabled() {}

    [[nodiscard]] GameObject *GetGameObject() const { return go; }

    virtual void Update(float dt) = 0;

    virtual void Receive(int message) {}

    virtual void Destroy() {}
};


