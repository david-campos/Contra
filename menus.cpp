//
// Created by david on 22/2/20.
//

#include "menus.h"

void MainMenu::Update(float dt) {
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
            game->SetPlayers(selected + 1);
            game->SetCurrentLevel(-1);
            game->Reset();
            Send(NEXT_LEVEL);
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

void MenuWithStats::Update(float dt) {
    BaseScene::Update(dt);
    char msg[100];
    sprintf(reinterpret_cast<char *>(&msg), "P1 % 10d", m_game->GetPlayerStats()[0].score);
    m_engine->drawText(10 * PIXELS_ZOOM, 10 * PIXELS_ZOOM,
            msg, {188, 188, 188});
    if (m_game->GetPlayers() == 2) {
        sprintf(reinterpret_cast<char *>(&msg), "P2 % 10d", m_game->GetPlayerStats()[1].score);
        m_engine->drawText(WINDOW_WIDTH - 10 * PIXELS_ZOOM, 10 * PIXELS_ZOOM,
                msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_RIGHT_TOP);
    }
}

void PreLevel::Update(float dt) {
    MenuWithStats::Update(dt);
    AvancezLib::KeyStatus keyStatus;
    m_engine->getKeyStatus(keyStatus);
    char msg[100];
    sprintf(reinterpret_cast<char *>(&msg), "STAGE %d: %s",
            m_level->GetLevelIndex(), &m_level->GetLevelName()[0]);
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2,
            msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);
    m_time += dt;

    if ((keyStatus.start && m_time > 0.5f) || m_time > 5.f) {
        m_level->Init();
        m_game->Start(m_level);
    }
}

void ContinueLevel::Update(float dt) {
    MenuWithStats::Update(dt);

    AvancezLib::KeyStatus keyStatus;
    m_engine->getKeyStatus(keyStatus);
    char msg[100];
    sprintf(reinterpret_cast<char *>(&msg), "GAME OVER");
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2,
            msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);

    sprintf(reinterpret_cast<char *>(&msg), "CONTINUE");
    m_engine->drawText(options[0].x + 200, options[0].y, msg,
            {188, 188, 188}, AvancezLib::TEXT_ALIGN_LEFT_MIDDLE);

    sprintf(reinterpret_cast<char *>(&msg), "END");
    m_engine->drawText(options[1].x + 200, options[1].y, msg,
            {188, 188, 188}, AvancezLib::TEXT_ALIGN_LEFT_MIDDLE);

    if (!previousKeys.start && keyStatus.start) {
        if (selected == 0) {
            m_level->Init();
            if (m_game->GetCurrentLevel() == 0) {
                m_game->Reset();
            } else {
                m_game->ResetPlayerStats(); // Restore previous score
            }
            m_game->Start(m_level);
        } else {
            // Free the level, we are not going to continue
            m_level->Destroy();
            delete m_level;

            // Go to main menu
            auto* menu = new MainMenu();
            menu->Create(m_engine, m_game);
            menu->Init();
            menu->AddReceiver(m_game);
            m_game->Start(menu);
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

    previousKeys = keyStatus;
}

void Credits::Update(float dt) {
    MenuWithStats::Update(dt);

    AvancezLib::KeyStatus keyStatus;
    m_engine->getKeyStatus(keyStatus);
    char msg[200];
    sprintf(reinterpret_cast<char *>(&msg), "CONGRATULATIONS, YOU HAVE WON!");
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2 - 140,
            msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);
    sprintf(reinterpret_cast<char *>(&msg),"VERSION AUTHOR:");
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2 - 70,
            msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);
    sprintf(reinterpret_cast<char *>(&msg)," DAVID CAMPOS RODRIGUEZ");
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2 - 35,
            msg, {255, 255, 255}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);
    sprintf(reinterpret_cast<char *>(&msg),"ORIGINAL GAME BY KONAMI");
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2 + 35,
            msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);
    sprintf(reinterpret_cast<char *>(&msg), "THANKS FOR PLAYING");
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2 + 105,
            msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);
    m_time += dt;

    if (keyStatus.start && m_time > 0.5f) {
        // Go to main menu
        auto* menu = new MainMenu();
        menu->Create(m_engine, m_game);
        menu->Init();
        menu->AddReceiver(m_game);
        m_game->Start(menu);
    }
}
