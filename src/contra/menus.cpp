//
// Created by david on 22/2/20.
//

#include "menus.h"

void MainMenu::Update(float dt) {
    BaseScene::Update(dt);
    AvancezLib::KeyStatus keyStatus;
    m_engine->getKeyStatus(keyStatus);
    char msg[10];
    sprintf(msg, "1 Player");
    Uint8 color = m_camera.x >= 0 && selected == 0 ? 255 : 188;
    if (started == 0.f || color != 255 || fmod(m_time - started, 0.5) > 0.25) {
        m_engine->drawText(round(options[0].x - m_camera.x + 20 * PIXELS_ZOOM), round(options[0].y + PIXELS_ZOOM),
                msg, {color, color, color});
    }
    sprintf(msg, "2 Players");
    color =  m_camera.x >= 0 && selected == 1 ? 255 : 188;
    if (started == 0.f || color != 255 || fmod(m_time - started, 0.5) > 0.25) {
        m_engine->drawText(round(options[1].x - m_camera.x + 20 * PIXELS_ZOOM), round(options[1].y + PIXELS_ZOOM),
                msg, {color, color, color});
    }
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
        if (!musicPlayed) {
            m_music->Play(1);
            musicPlayed = true;
        }
        if (started != 0.f) {
            if (!m_engine->isMusicPlaying()) {
                game->SetPlayers(selected + 1);
                game->SetCurrentLevel(-1);
                game->Reset();
                Send(NEXT_LEVEL);
            }
        } else {
            if (!previousKeys.start && keyStatus.start) {
                started = m_time;
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
    }
    previousKeys = keyStatus;
}

void MenuWithStats::Update(float dt) {
    BaseScene::Update(dt);
    char msg[100];
    sprintf(reinterpret_cast<char *>(&msg), "P1 % 10d", m_game->GetPlayerStats()[0].score);
    m_engine->drawText(10 * PIXELS_ZOOM, 10 * PIXELS_ZOOM,
            msg, {188, 188, 188});
    sprintf(reinterpret_cast<char *>(&msg), "REST %d", m_game->GetPlayerStats()[0].lives + 1);
    m_engine->drawText(10 * PIXELS_ZOOM, 10 * PIXELS_ZOOM + 40,
            msg, {188, 188, 188});

    if (m_game->GetPlayers() == 2) {
        sprintf(reinterpret_cast<char *>(&msg), "P2 % 10d", m_game->GetPlayerStats()[1].score);
        m_engine->drawText(WINDOW_WIDTH - 10 * PIXELS_ZOOM, 10 * PIXELS_ZOOM,
                msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_RIGHT_TOP);
        sprintf(reinterpret_cast<char *>(&msg), "REST %d", m_game->GetPlayerStats()[1].lives + 1);
        m_engine->drawText(WINDOW_WIDTH - 10 * PIXELS_ZOOM, 10 * PIXELS_ZOOM + 40,
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
            Send(REPEAT_LEVEL);
        } else {
            Send(GO_TO_MAIN_MENU);
        }
        return;
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
    sprintf(reinterpret_cast<char *>(&msg), "VERSION AUTHOR:");
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2 - 70,
            msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);
    sprintf(reinterpret_cast<char *>(&msg), " DAVID CAMPOS RODRIGUEZ");
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2 - 35,
            msg, {255, 255, 255}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);
    sprintf(reinterpret_cast<char *>(&msg), "ORIGINAL GAME BY KONAMI");
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2 + 35,
            msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);
    sprintf(reinterpret_cast<char *>(&msg), "THANKS FOR PLAYING");
    m_engine->drawText(WINDOW_WIDTH / 2, WINDOW_WIDTH / 2 + 105,
            msg, {188, 188, 188}, AvancezLib::TEXT_ALIGN_CENTER_MIDDLE);
    m_time += dt;

    if (keyStatus.start && m_time > 0.5f) {
        Send(GO_TO_MAIN_MENU);
    }
}
