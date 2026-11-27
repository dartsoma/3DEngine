#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <stdint.h>
#define EFFECT_BOOST 1
#define EFFECT_FAKEBOOST 2
#define EFFECT_JUMPBOOST 3
#define EFFECT_ABILITY 4
#define EFFECT_LOWGRAV 5

#define BASE_VEL_MAX 6.0f
#define EFFECT_DURATION 5.0f // seconds
#define BOOST_VEL_MULT 1.6f
#define SLOW_VEL_MULT 0.4f
#define JUMPBOOST_MULT 1.7f
#define LOWGRAV_MULT 0.35f

#define AXIS_MOVE 0
#define AXIS_PREVMOVE 1
#define AXIS_CAMERA 2

#define FLOAT_CAMERADISTANCE 0
#define FLOAT_ABILITY_1_TIMER 1 // JUMP
#define FLOAT_ABILITY_2_TIMER 2 // DIVE
#define FLOAT_ABILITY_3_TIMER 3 // SLIDE
#define FLOAT_ABILITY_4_TIMER 4 // BREAK
#define FLOAT_ABILITY_5_TIMER 5
#define FLOAT_BOOST_TIMER 6
#define FLOAT_SLOW_TIMER 7
#define FLOAT_JUMPBOOST_TIMER 8
#define FLOAT_LOWGRAV_TIMER 9

#define INT_PLAYERFLAGS 0
#define FLAG_GROUNDED 1
#define FLAG_FASTFALL 2
#define FLAG_SLIDING 4
#define FLAG_JUMPING 8
#define FLAG_BRAKING 16

#define INT_UNLOCKFLAGS 1
#define FLAG_ABILITY1 1  // Jump
#define FLAG_ABILITY2 2  // Dive
#define FLAG_ABILITY3 4  // Slide
#define FLAG_ABILITY4 8  // Brake
#define FLAG_ABILITY5 16 //

#define JUMP_SPEED 4.0f
#define FALL_ACC -10.0f      // gravity (default)
#define FASTFALL_MULT 3.0f   // gravity multiplier
#define FASTFALL_SPEED 14.0f // instant speed

#define CAPSULE_MAIN 0

#include "entity_core.h"

Entity* player_create();

void player_free(Entity* self);

void player_think(Entity* self);

void player_update(Entity* self);

void player_draw(Entity* self);

Uint8 player_ability_check(Entity* self, int flag);

void toggle_ability_flag(Entity* self, int flag);

void toggle_player_flag(Entity* self, int flag);

void player_apply_effect(Entity* self, int effect);

float player_get_speed(Entity* self);

#endif
