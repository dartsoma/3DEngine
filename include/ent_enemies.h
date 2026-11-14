#ifndef __ENT_ENEMIES_H__
#define __ENT_ENEMIES_H__

#include "entity_component.h"
#include "entity_core.h"
#include "gfc_color.h"
#include "gfc_input.h"
#include "gfc_matrix.h"
#include "gfc_types.h"
#include "gfc_vector.h"
#include "model.h"
#include "simple_logger.h"
#include "stdlib.h"
#include "string.h"

// 0 - null, 1 - Gopher, 2 - School, 3 - Angler, 4 - Fatso, 5 - Stone
//
//

Entity *enemy_create(uint8_t tag);

void entity_spawn_at(uint8_t tag, Transform *transform);

void enemy_free(Entity *self);

void enemy_think(Entity *self);

void enemy_update(Entity *self);

void enemy_draw(Entity *self);
#endif
