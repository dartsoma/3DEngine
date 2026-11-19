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

// 0 - null, 1 - Void, 2 - Anchor, 3 - Boost, 4 - Pile, 5 - Trash

Entity* prop_create(uint8_t tag);

Entity* prop_spawn_at(uint8_t tag, GFC_Vector3D* vec);

void prop_free(Entity* self);

void prop_think(Entity* self);

void prop_update(Entity* self);

void prop_draw(Entity* self);

#endif
