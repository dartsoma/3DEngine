#ifndef __ENT_PROPS_H__
#define __ENT_PROPS_H__

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

#define ANCHOR_PULL 0.8f
#define ANCHOR_DRAG 0.90f
#define BOOST_LAUNCH_UP 8.0f
#define BOOST_LAUNCH_FWD 6.0f
#define PILE_DRAG 0.85f

#define PROP_PICKUP_RADIUS 5.0f
#define PROP_HIT_RADIUS 6.0f
// 0 - null, 1 - Void, 2 - Anchor, 3 - Boost, 4 - Pile, 5 - Trash

// 6 - Boostpack, 7 - Fake Boost Pack, 8 - Jump Boost, 9 - Ability Pack, 10 - Gravity Pack

Entity* prop_create(uint8_t tag);

Entity* prop_spawn_at(uint8_t tag, GFC_Vector3D* vec);

void prop_free(Entity* self);

void prop_think(Entity* self);

void prop_update(Entity* self);

void prop_draw(Entity* self);

void prop_try_pickup(Entity* self, int effect);
#endif
