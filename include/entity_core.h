#ifndef __ENTITY_CORE_H__
#define __ENTITY_CORE_H__

#include "cf_queue.h"
#include "gfc_matrix.h"
#include "gfc_shape.h"
#include "gfc_vector.h"
#include "model.h"
#include <stdint.h>

#define ALLCOMPONENTS 5

//  ported from gf2d

typedef struct Entity {
  uint8_t _inuse;
  char id[11];                         // Unique identifier (who is it) its a char because of hashmap
  uint32_t tag;                        // Component Mask
  void *data;
  void (*think)(struct Entity *self);  // Called every frame
  void (*update)(struct Entity *self); // Called on deltatime
  void (*free)(struct Entity *self);
  void (*draw)(struct Entity *self);
} Entity;


void entity_manager_init(uint32_t max);

void entity_manager_close();

void entity_free(Entity *entity);

Entity *entity_create();

uint8_t entity_is_valid(uint32_t id);

void entity_manager_draw();

void entity_manager_think();

void entity_manager_update();

Entity *entity_get_reference(uint32_t id);

uint32_t entity_get_index(uint32_t id);


#endif
