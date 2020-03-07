//
// Created by david on 2/3/20.
//

#ifndef CONTRA_PLAYER_STATS_H
#define CONTRA_PLAYER_STATS_H

#include "entities/weapon_types.h"

struct PlayerStats {
    int score;
    int lives;
    WeaponType weapon;
    float hasRapid; // Ok for now, maybe it would be better to save the speed multiplier float tho
};

#endif //CONTRA_PLAYER_STATS_H
