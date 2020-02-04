#include "consts.h"
#include "game.h"
#include "Player.h"

void Game::CreatePlayer() {
    auto *player = new Player();
    player->Create();
    player->position = Vector2D(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);
    auto *renderer = new AnimationRenderer();
    std::shared_ptr<Sprite> sprite_sheet(engine->createSprite("data/spritesheet.png"));
    renderer->Create(engine, player, &game_objects, sprite_sheet);
    AnimationRenderer::Animation animation_idle{
            0, 8, 0.1, 2,
            24, 34, 8, 34,
            "Idle", false
    };
    AnimationRenderer::Animation animation_up {
            49, 0, 0.1, 2,
            15, 42, 7, 42,
            "Up", false
    };
    AnimationRenderer::Animation animation_crawl {
            79, 25, 0.1, 2,
            33, 17, 16, 17,
            "Crawl", false
    };
    AnimationRenderer::Animation animation_run{
            0, 43, 0.1, 6,
            20, 35, 10, 35,
            "Run", true
    };
    AnimationRenderer::Animation animation_jump{
            122, 52, 0.16, 4,
            20, 20, 10, 26,
            "Jump", true
    };
    renderer->AddAnimation(animation_idle);
    renderer->AddAnimation(animation_up);
    renderer->AddAnimation(animation_crawl);
    renderer->AddAnimation(animation_run);
    renderer->AddAnimation(animation_jump);
    player->AddComponent(renderer);
    auto *control = new PlayerControl();
    control->Create(engine, player, &game_objects);
    player->AddComponent(control);
    player->AddReceiver(this);
    game_objects.insert(player);
}

