//
// Created by david on 22/2/20.
//

#include "game.h"
#include "menus.h"

void Game::Create(AvancezLib *avancezLib) {
    SDL_Log("Game::Create");
    this->engine = avancezLib;
    spritesheet.reset(engine->createSprite("data/spritesheet.png"));
    enemies_spritesheet.reset(engine->createSprite("data/enemies_spritesheet.png"));
    pickups_spritesheet.reset(engine->createSprite("data/pickups.png"));

    auto* menu = new MainMenu();
    menu->Create(engine, this);
    menu->AddReceiver(this);
    currentScene = menu;
}

void Game::Receive(Message m) {
    switch (m) {
        case GAME_OVER:
            if (can_continue) {
                can_continue = false;

                auto level = new Level();
                level->Create("data/level1/", spritesheet, enemies_spritesheet, pickups_spritesheet, engine);
                level->AddReceiver(this);
                auto continue_menu = new ContinueLevel();
                continue_menu->Create(engine, this);
                continue_menu->Init(level);

                Start(continue_menu);
            } else {
                auto* menu = new MainMenu();
                menu->Create(engine, this);
                menu->AddReceiver(this);
                menu->Init();

                Start(menu);
            }
            break;
        case NEXT_LEVEL:
            // We only have level 1 by now ^^'
            auto level = new Level();
            level->Create("data/level1/", spritesheet, enemies_spritesheet, pickups_spritesheet, engine);
            level->AddReceiver(this);

            auto introduction = new PreLevel();
            introduction->Create(engine, this);
            introduction->Init(level);

            Start(introduction);
            break;
    }
}

