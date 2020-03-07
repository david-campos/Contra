//
// Created by david on 22/2/20.
//

#ifndef CONTRA_MENUS_H
#define CONTRA_MENUS_H

#include "../components/scene.h"
#include "../components/render/SimpleRenderer.h"
#include "game.h"

class MainMenu : public BaseScene {
private:
    GameObject *selector;
    Vector2D options[2];
    AvancezLib::KeyStatus previousKeys;
    int selected = 0;
    Game *game;
    float started = 0.f;
    bool musicPlayed = false;
public:
    void Create(AvancezLib *engine, Game* the_game) {
        BaseScene::Create(engine, "data/main_menu/background.png", "data/sound/title.wav");
        game = the_game;
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
        BaseScene::Init();
        m_camera = Vector2D(-WINDOW_WIDTH, 0);
    }

    void Update(float dt) override;

    void Destroy() override {
        BaseScene::Destroy();
        selector->Destroy();
        delete selector;
        selector = nullptr;
    }
};

class MenuWithStats: public BaseScene {
protected:
    Game* m_game;
public:
    virtual void Create(AvancezLib *engine, Game* game, const char* music_path = nullptr) {
        BaseScene::Create(engine, nullptr, music_path);
        m_engine = engine;
        m_game = game;
    }

    void Update(float dt) override;
};

class PreLevel: public MenuWithStats {
private:
    float m_time;
    Level* m_level;
public:
    void Init(Level* level) {
        MenuWithStats::Init();
        m_level = level;
        m_time = 0;
    }

    void Update(float dt) override;
};

class ContinueLevel: public MenuWithStats {
private:
    GameObject *selector;
    Vector2D options[2];
    AvancezLib::KeyStatus previousKeys;
    int selected = 0;
public:
    void Create(AvancezLib *engine, Game* game) {
        MenuWithStats::Create(engine, game, "data/sound/game_over.wav");
        selector = new GameObject();
        selector->Create();
        auto *render = new SimpleRenderer();
        render->Create(this, selector,
                std::shared_ptr<Sprite>(m_engine->createSprite("data/main_menu/menu_spritesheet.png")),
                0, 0, 16, 10, 8, 5);
        selector->AddComponent(render);
        selector->Init();
        game_objects[0]->insert(selector);

        options[0] = Vector2D(35, 151) * PIXELS_ZOOM;
        options[1] = Vector2D(35, 167) * PIXELS_ZOOM;
    }

    void Init() override {
        MenuWithStats::Init();
        m_music->Play(1);
    }

    void Update(float dt) override;
};

class Credits: public MenuWithStats {
private:
    float m_time;
public:
    void Init() override {
        MenuWithStats::Init();
        m_time = 0;
    }
    void Update(float dt) override;
};

#endif //CONTRA_MENUS_H
