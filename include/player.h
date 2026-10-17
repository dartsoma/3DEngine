#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity_core.h"


Entity* player_create();

void player_free(Entity *self);

void player_think(Entity *self);

void player_update(Entity *self);

void player_draw(Entity *self);



#endif
