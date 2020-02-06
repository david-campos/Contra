#include "consts.h"
#include "game.h"
#include "Player.h"
#include "Gravity.h"

void Game::CreatePlayer() {
    player = new Player();
    player->Create();
    player->position = Vector2D(100, 0);
    auto *renderer = new AnimationRenderer();
    renderer->Create(engine, player, &game_objects, spritesheet, &camera_x);
    AnimationRenderer::Animation animation_idle{
            0, 8, 0.1, 2,
            24, 34, 8, 33,
            "Idle", AnimationRenderer::STOP_AND_FIRST
    };
    AnimationRenderer::Animation animation_up {
            49, 0, 0.1, 2,
            15, 42, 7, 42,
            "Up", AnimationRenderer::STOP_AND_FIRST
    };
    AnimationRenderer::Animation animation_crawl {
            79, 25, 0.1, 2,
            33, 17, 16, 17,
            "Crawl", AnimationRenderer::STOP_AND_FIRST
    };
    AnimationRenderer::Animation animation_run{
            0, 43, 0.1, 6,
            20, 35, 10, 35,
            "Run", AnimationRenderer::DONT_STOP
    };
    AnimationRenderer::Animation animation_fall{
            80, 43, 1, 1,
            20, 35, 10, 35,
            "Fall", AnimationRenderer::STOP_AND_FIRST
    };
    AnimationRenderer::Animation animation_run_shoot {
            0, 79, 0.1, 3,
            25, 34, 12, 34,
            "RunShoot", AnimationRenderer::STOP_AND_FIRST
    };
    AnimationRenderer::Animation animation_run_up{
            0, 149, 0.1, 3,
            20, 35, 10, 35,
            "RunUp", AnimationRenderer::DONT_STOP
    };
    AnimationRenderer::Animation animation_run_down {
            0, 221, 0.1, 3,
            22, 35, 11, 35,
            "RunDown", AnimationRenderer::DONT_STOP
    };
    AnimationRenderer::Animation animation_jump{
            122, 52, 0.1, 4,
            20, 20, 10, 26,
            "Jump", AnimationRenderer::DONT_STOP
    };
    AnimationRenderer::Animation animation_splash {
            0, 307, 0.2, 1,
            18, 16, 9, 13,
            "Splash", AnimationRenderer::STOP_AND_FIRST
    };
    AnimationRenderer::Animation animation_dive {
            18, 303, 0.2, 2,
            17, 16, 8, 12,
            "Dive", AnimationRenderer::DONT_STOP
    };
    AnimationRenderer::Animation animation_swim {
            52, 303, 0.2, 2,
            17, 16, 8, 12,
            "Swim", AnimationRenderer::DONT_STOP
    };
    AnimationRenderer::Animation animation_swim_shoot {
            60, 328, 0.2, 2,
            26, 18, 13, 14,
            "SwimShoot", AnimationRenderer::STOP_AND_FIRST
    };
    AnimationRenderer::Animation animation_swim_shoot_diagonal {
            90, 299, 0.2, 2,
            20, 20, 10, 16,
            "SwimShootDiagonal", AnimationRenderer::STOP_AND_FIRST
    };
    AnimationRenderer::Animation animation_swim_shoot_up {
            130, 290, 0.2, 2,
            18, 29, 9, 25,
            "SwimShootUp", AnimationRenderer::STOP_AND_FIRST
    };
    AnimationRenderer::Animation animation_die {
            61, 161, 0.1, 5,
            32, 23, 16, 23,
            "Die", AnimationRenderer::STOP_AND_LAST
    };
    renderer->AddAnimation(animation_idle);
    renderer->AddAnimation(animation_up);
    renderer->AddAnimation(animation_crawl);
    renderer->AddAnimation(animation_run);
    renderer->AddAnimation(animation_fall);
    renderer->AddAnimation(animation_run_shoot);
    renderer->AddAnimation(animation_run_up);
    renderer->AddAnimation(animation_run_down);
    renderer->AddAnimation(animation_jump);
    renderer->AddAnimation(animation_splash);
    renderer->AddAnimation(animation_dive);
    renderer->AddAnimation(animation_swim);
    renderer->AddAnimation(animation_swim_shoot);
    renderer->AddAnimation(animation_swim_shoot_diagonal);
    renderer->AddAnimation(animation_swim_shoot_up);
    renderer->AddAnimation(animation_die);
    player->AddComponent(renderer);
    auto *gravity = new Gravity();
    gravity->Create(engine, player, &game_objects, level_floor);
    player->AddComponent(gravity);
    playerControl = new PlayerControl();
    playerControl->Create(engine, player, &game_objects, level_floor, &camera_x, bullets);
    player->AddComponent(playerControl);
    player->AddReceiver(this);
    game_objects.insert(player);
}

