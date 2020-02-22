//
// Created by david on 22/2/20.
//

#ifndef CONTRA_MAIN_MENU_H
#define CONTRA_MAIN_MENU_H

#include "game_object.h"
#include "scene.h"
#include "SimpleRenderer.h"

class MainMenu : public BaseScene {
private:
    GameObject *selector;
    Vector2D options[2];
    AvancezLib::KeyStatus previousKeys;
    int selected = 0;
public:
    void Create(AvancezLib *engine) {
        BaseScene::Create(engine, "data/main_menu/background.png");
        selector = new GameObject();
        selector->Create();
        auto *render = new SimpleRenderer();
        render->Create(this, selector,
                std::shared_ptr<Sprite>(m_engine->createSprite("data/main_menu/menu_spritesheet.png")),
                0, 0, 16, 10, 0, 0);
        selector->AddComponent(render);
        game_objects[0]->insert(selector);

        options[0] = Vector2D(35, 151) * PIXELS_ZOOM;
        options[1] = Vector2D(35, 167) * PIXELS_ZOOM;
    }

    void Init() override {
        GameObject::Init();
        m_camera = Vector2D(-WINDOW_WIDTH, 0);
    }

    void Update(float dt) override {
        BaseScene::Update(dt);
        AvancezLib::KeyStatus keyStatus;
        m_engine->getKeyStatus(keyStatus);
        if (m_camera.x < 0) {
            if (keyStatus.start) {
                m_camera.x = 0;
            } else {
                m_camera.x += dt * PIXELS_ZOOM * 40;
            }
            if (m_camera.x >= 0) {
                m_camera.x = 0;
                selector->Init();
            }
        } else {
            if (!previousKeys.start && keyStatus.start) {
                if (selected == 0) {
                    Send(NEXT_SCENE);
                    return;
                }
            }
            if (!previousKeys.up && keyStatus.up) {
                selected -= 1;
            }
            if (!previousKeys.down && keyStatus.down) {
                selected += 1;
            }
            if (selected < 0) selected += 2;
            if (selected > 1) selected -= 2;
            selector->position = options[selected];
        }
        previousKeys = keyStatus;
    }

    void Destroy() override {
        BaseScene::Destroy();
    }
};

#endif //CONTRA_MAIN_MENU_H
