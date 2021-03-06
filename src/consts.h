//
// Created by david on 4/2/20.
//

#ifndef CONTRA_CONSTS_H
#define CONTRA_CONSTS_H

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 896
#define RENDERING_MARGINS 200
#define SCREEN_PLAYER_LEFT_MARGIN 10
#define PIXELS_ZOOM 4

#define MAX_DEFAULT_BULLETS 4
#define MAX_FIRE_BULLETS 4
#define MAX_MACHINE_GUN_BULLETS 6
#define MAX_SPREAD_BULLETS 10
#define MAX_LASER_BULLETS 4
#define MAX_NPC_BULLETS 40

#define MAX_BLASTER_CANON_BULLETS 10
#define MIN_BLAST_X_SPEED 20
#define MAX_BLAST_X_SPEED 80
#define MIN_BLAST_Y_SPEED 20
#define MAX_BLAST_Y_SPEED 130
#define MIN_BLAST_WAIT 0.2f
#define MAX_BLAST_WAIT 1.f

#define PLAYER_COLLISION_LAYER 0
#define NPCS_COLLISION_LAYER 1
#define PERSP_PLAYER_BULLETS_COLLISION_LAYER 2

#define SECOND_PLAYER_SHIFT 225

#define LIFE_SPRITE_WIDTH 8
#define LIFE_SPRITE_HEIGHT 16
#define LIFE_SPRITE_MARGIN 2
#define LIFE_SPRITE_X 193
#define LIFE_SPRITE_Y 0

#define RENDERING_LAYERS 4
#define RENDERING_LAYER_BRIDGES 0
#define RENDERING_LAYER_ENEMIES 1
#define RENDERING_LAYER_PLAYER 2
#define RENDERING_LAYER_BULLETS 3

#define PLAYER_SPEED 55
#define PLAYER_JUMP 200

#define PICKUP_SPEED 50

#define BULLET_SPEED 160
#define FIRE_BULLET_SPEED 90
#define FIRE_BULLET_MOVEMENT_RADIUS 15
#define ENEMY_BULLET_SPEED 90

// Preloaded sprites
#define SPRITESHEET_PLAYER 0
#define SPRITESHEET_ENEMIES 1
#define SPRITESHEET_PICKUPS 2

// Preloaded sounds in the level
#define SOUND_ENEMY_DEATH 0
#define SOUND_PLAYER_DEATH 1
#define SOUND_ENEMY_HIT 2
#define SOUND_EXPLOSION 3
#define SOUND_PICKUP 4

#endif //CONTRA_CONSTS_H
