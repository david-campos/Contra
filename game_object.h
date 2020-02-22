#pragma once

// GameObject represents objects which moves are drawn
#include <vector>
#include <set>
#include "vector2D.h"

enum Message {
    GAME_OVER, LEVEL_END, NEXT_LEVEL
};

class Component;

class GameObject {
private:
    bool enabled;
    bool marked_to_remove;
    bool destroyed = false;
protected:
    std::vector<GameObject *> receivers;
    std::vector<Component *> components;
    int id;
public:
    enum OnOutOfScreen {
        DESTROY,
        DO_NOT_DESTROY
    };

    static int s_nextId;
    Vector2D position;
    OnOutOfScreen onRemoval = DESTROY;

    GameObject() {
        id = s_nextId++;
        marked_to_remove = false;
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

    /**
     * Marks the object to be removed at the end of the frame
     */
    void MarkToRemove() { marked_to_remove = true; }
    /**
     * Used by the level to unmark removed objects
     */
    void UnmarkToRemove() { marked_to_remove = false; }

    [[nodiscard]] bool IsMarkedToRemove() const {
        return marked_to_remove;
    }

    void Send(Message m);

    void Disable() {
        if (enabled) {
            enabled = false;
            OnDisabled();
        }
    }

    void Enable() {
        if (!enabled) {
            enabled = true;
            OnEnabled();
        }
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

