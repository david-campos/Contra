//
// Created by david on 6/3/20.
//

#ifndef CONTRA_PERSPECTIVE_CONST_H
#define CONTRA_PERSPECTIVE_CONST_H

#define PERSP

// Positions for enemies and player
#define PERSP_ENEMIES_Y 112
#define PERSP_PLAYER_Y 182
#define PERSP_BOSS_PLAYER_Y 208
#define PERSP_ENEMIES_MARGINS 95

// The following constants are used to project bullet targets
#define PERSP_FRONT_X_START 40
#define PERSP_FRONT_X_RANGE (WINDOW_WIDTH / PIXELS_ZOOM - 2 * PERSP_FRONT_X_START)
#define PERSP_BACK_X_START 95
#define PERSP_BACK_X_RANGE (WINDOW_WIDTH / PIXELS_ZOOM - 2 * PERSP_BACK_X_START)
#define PERSP_FRONT_Y_START 57
#define PERSP_FRONT_Y_RANGE 115
#define PERSP_BACK_Y_START 37
#define PERSP_BACK_Y_RANGE 85

#endif //CONTRA_PERSPECTIVE_CONST_H
