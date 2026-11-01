#ifndef __LEVEL_H__
#define __LEVEL_H__
#include "entity_core.h"
#include "gfc_hashmap.h"
#include "gfc_vector.h"
#include "model.h"
#include <stdint.h>

typedef struct {

  uint8_t type; // 255 unique entity types; dont need more than that
  GFC_Vector3D pos;
  GFC_Vector3D rot;
  GFC_Vector3D scale;

} EntityRegistry;

typedef struct {

  Model *model;
  char modelpath[255];
  char name[255];
  GFC_Matrix4 mat;
  GFC_Color color;
  GFC_Vector3D pos;
  GFC_Vector3D rot;
  GFC_Vector3D scale;

  uint8_t collision_type; // 0 - simple (bounding box), 1 - complex (mesh), 2 -
  uint32_t collision_mask;

} StaticModel;

typedef struct {
  GFC_HashMap *static_hash;
  uint32_t static_count;
  StaticModel *static_list;
  uint32_t entity_count;
  EntityRegistry *entity_registry;
  char name[255];
  char sky[255];
  char music[255];
  GFC_Box bounds;
} Level;

uint8_t level_setup();

// spawn static objects
uint8_t level_build();

// ready/ spawn entities
uint8_t level_populate();

void level_clear();

uint32_t level_get_static_count();

const char *level_get_current();

uint32_t level_get_current_index(); // cast to a 32 bit integer PLEASE!!!!!! i
// had no other way to do error cases

uint32_t level_get_count();

StaticModel *level_get_static(uint32_t id);

GFC_Box level_get_bounds();

#endif
