#include <iostream>

#include <thread>
#include <chrono>

#include "src/consts.h"

#include "src/contra/game.h"
#include "src/kernel/avancezlib.h"

float game_speed = 1.f;

int main (int argc, char *argv[]) {
    AvancezLib engine{};

    engine.init(WINDOW_WIDTH, WINDOW_HEIGHT);

    Game game;
    game.Create(&engine);
    game.Init();

    float lastTime = engine.getElapsedTime();
#ifndef NDEBUG
    char fps[100];
#endif
    float smoothedDt = 0.004;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        float newTime = engine.getElapsedTime();
        float dt = newTime - lastTime;

        // Cap to 240 FPS, because my laptop goes very noisy otherwise and it embarrasses me
        int dtMillis = (int) (dt * 1000.f);
        if (dtMillis < 4) {
            std::this_thread::sleep_for(std::chrono::milliseconds(4 - dtMillis));
            newTime = engine.getElapsedTime();
            dt = newTime - lastTime;
            smoothedDt = smoothedDt * 0.99f + dt * 0.01f;
        }

        dt = dt * game_speed;
        lastTime = newTime;

        engine.processInput();
        game.Update(dt);
#ifndef NDEBUG
        sprintf(fps, "FPS: %d", (int) round(1 / smoothedDt));
        engine.drawText(0, 0, fps);
#endif
        game.Draw();
    }
#pragma clang diagnostic pop
}
