#ifndef __LEVEL_H__
#define __LEVEL_H__
#include "gfc_hashmap.h"
#include "gfc_vector.h"
#include "entity_core.h"
#include "model.h"
#include <stdint.h>

typedef struct {

    uint8_t type; // 255 unique entity types; dont need more than that
    GFC_Vector3D pos;
    GFC_Vector3D rot;
    GFC_Vector3D scale;

} EntityRegistry;

typedef struct{

    Model *model;

    GFC_Vector3D pos;
    GFC_Vector3D rot;
    GFC_Vector3D scale;

    uint8_t collision_type; // 0 - simple (bounding box), 1 - complex (mesh), 2 - sphere (mesh)

} StaticModel;

typedef struct {

    uint32_t model_max;
    StaticModel *model_list;
    EntityRegistry *registry;
    char name[255];
    char config[255]; // filepath

}Level;

uint8_t level_setup();

// spawn static objects
uint8_t level_build();

// ready/ spawn entities
uint8_t level_populate();

void level_clear();

void level_delete();


uint32_t *level_get_static_count();


StaticModel *level_get_static(uint32_t id);


StaticModel *level_get_bounds(uint32_t id);




#endif
