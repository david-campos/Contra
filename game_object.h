#pragma once

// GameObject represents objects which moves are drawn
#include <vector>
#include <set>
#include "vector2D.h"

enum Message {
    GAME_OVER, HIT, NO_MSG
};

class Component;

class GameObject {
private:
    bool enabled;
protected:
    std::vector<GameObject *> receivers;
    std::vector<Component *> components;
    int id;
public:
    enum OnOutOfScreen {
        DISABLE_AND_DESTROY,
        JUST_DISABLE
    };

    static int s_nextId;
    Vector2D position;
    OnOutOfScreen onOutOfScreen = DISABLE_AND_DESTROY;

    GameObject() {
        id = s_nextId++;
    }

    virtual ~GameObject();

    virtual void Create();

    virtual void AddComponent(Component *component);

    virtual void Init();

    virtual void Update(float dt);

    virtual void Destroy();

    virtual void AddReceiver(GameObject *go);

    virtual void Receive(Message m) {}

    void OnEnabled();
    void OnDisabled();

    void Send(Message m);

    void Disable() {
        enabled = false;
        OnDisabled();
    }

    void Enable() {
        enabled = true;
        OnEnabled();
    }

    [[nodiscard]] bool IsEnabled() const { return enabled; }

    template<typename T>
    T GetComponent() {
        for (Component *c : components) {
            T t = dynamic_cast<T>(c);  //ugly but it works...
            if (t != nullptr) {
                return t;
            }
        }

        return nullptr;
    }

    int getID() {
        return id;
    }
};

