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

    auto *menu = new MainMenu();
    menu->Create(engine, this);
    menu->AddReceiver(this);
    currentScene = menu;
}

void Game::Receive(Message m) {
    switch (m) {
        case GAME_OVER: {
            SDL_Log("GAME_OVER");
            if (can_continue) {
                can_continue = false;

                auto level = new Level();
                level->Create("data/level1/", spritesheet, enemies_spritesheet, pickups_spritesheet, players, engine);
                level->AddReceiver(this);
                auto continue_menu = new ContinueLevel();
                continue_menu->Create(engine, this);
                continue_menu->Init(level);
                continue_menu->AddReceiver(this);

                Start(continue_menu);
            } else {
                auto *menu = new MainMenu();
                menu->Create(engine, this);
                menu->AddReceiver(this);
                menu->Init();

                Start(menu);
            }
            break;
        }
        case NEXT_LEVEL: {
            SDL_Log("NEXT_LEVEL");
            current_level++;

            memcpy(lastSavedStats, stats, sizeof(PlayerStats) * 2);

            // We only have level 1 by now ^^'
            switch (current_level) {
                case 0: {
                    auto level = new Level();
                    level->Create("data/level1/", spritesheet, enemies_spritesheet, pickups_spritesheet, players,
                            engine);

                    level->AddReceiver(this);

                    auto introduction = new PreLevel();
                    introduction->Create(engine, this);
                    introduction->Init(level);
                    introduction->AddReceiver(this);

                    Start(introduction);
                    break;
                }
                case 1: {
                    auto* credits = new Credits();
                    credits->Create(engine, this);
                    credits->Init();
                    credits->AddReceiver(this);

                    Start(credits);
                    break;
                }
            }
            break;
        }
        case SCORE1_100:
        case SCORE1_300:
        case SCORE1_500:
        case SCORE1_1000:
        case SCORE1_10000:
        case SCORE2_100:
        case SCORE2_300:
        case SCORE2_500:
        case SCORE2_1000:
        case SCORE2_10000: {
            int array[]{100, 300, 500, 1000, 10000};
            stats[(m - SCORE1_100) / 5].score += array[(m - SCORE1_100) % 5];
            break;
        }
    }
}

int Game::GetCurrentLevel() const {
    return current_level;
}

void Game::SetCurrentLevel(int currentLevel) {
    current_level = currentLevel;
}

