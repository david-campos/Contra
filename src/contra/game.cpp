//
// Created by david on 22/2/20.
//

#include "game.h"
#include "menus.h"
#include "level/level_factory.h"

void Game::Create(AvancezLib *avancezLib) {
    SDL_Log("Game::Create");
    this->engine = avancezLib;
    levelFactory = new LevelFactory(&spritesheets, players, &stats[0], engine);

    {
        std::shared_ptr<Sprite> ss_player(engine->createSprite("data/spritesheet.png"));
        std::shared_ptr<Sprite> ss_enemies(engine->createSprite("data/enemies_spritesheet.png"));
        std::shared_ptr<Sprite> ss_pickups(engine->createSprite("data/pickups.png"));
        spritesheets.insert({SPRITESHEET_PLAYER, ss_player});
        spritesheets.insert({SPRITESHEET_ENEMIES, ss_enemies});
        spritesheets.insert({SPRITESHEET_PICKUPS, ss_pickups});
    }

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

                auto *level = levelFactory->LoadLevel("data/level" + std::to_string(current_level + 1) + "/", players);
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

            if (current_level < 2) {
                auto *level = levelFactory->LoadLevel("data/level" + std::to_string(current_level + 1) + "/", players);
                level->AddReceiver(this);
                auto *introduction = new PreLevel();
                introduction->Create(engine, this);
                introduction->Init(level);
                introduction->AddReceiver(this);

                Start(introduction);
            } else {
                auto *credits = new Credits();
                credits->Create(engine, this);
                credits->Init();
                credits->AddReceiver(this);

                Start(credits);
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
        case LIFE_LOST_1:
            stats[0].lives -= 1;
            stats[0].weapon = RIFLE;
            stats[0].hasRapid = false;
            break;
        case LIFE_LOST_2:
            if (players > 1) {
                stats[1].lives -= 1;
                stats[1].weapon = RIFLE;
                stats[1].hasRapid = false;
            }
            break;
        case PLAYER_WEAPON_UPDATE: {
            auto *level = dynamic_cast<Level *>(currentScene);
            if (level) {
                auto controls = level->GetPlayerControls();
                stats[0].weapon = controls[0]->GetWeaponType();
                stats[0].hasRapid = abs(controls[0]->GetBulletSpeedMultiplier() - 1.f) > 0.01f;
                if (players > 1) {
                    stats[1].weapon = controls[1]->GetWeaponType();
                    stats[1].hasRapid = abs(controls[1]->GetBulletSpeedMultiplier() - 1.f) > 0.01f;
                }
                SDL_Log("Updated weapons %d %c", stats[0].weapon, stats[0].hasRapid ? 'R' : ' ');
            }
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

void Game::Destroy() {
    SDL_Log("Game::Destroy");
    if (currentScene) currentScene->Destroy();
    delete levelFactory;
}

