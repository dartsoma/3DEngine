#ifndef __LEVEL_H__
#define __LEVEL_H__
#include "gfc_hashmap.h"
#include "gfc_primitives.h"
#include "gfc_vector.h"
#include "entity_core.h"
#include "model.h"
#include "simple_json.h"
#include "simple_json_value.h"
#include <stdint.h>

typedef struct {

    uint8_t type; // 255 unique entity types; dont need more than that
    GFC_Vector3D pos;
    GFC_Vector3D rot;
    GFC_Vector3D scale;

} EntityRegistry;

typedef struct{

    Model *model;
    char name[255];
    GFC_Vector3D pos;
    GFC_Vector3D rot;
    GFC_Vector3D scale;

    uint8_t collision_type; // 0 - simple (bounding box), 1 - complex (mesh), 2 - sphere (mesh)

} StaticModel;

typedef struct {


    GFC_HashMap static_hash;
    uint32_t static_count;
    StaticModel *static_list;
    EntityRegistry *registry;
    char name[255];
    char sky[255];
    GFC_Box bounds;
}Level;

typedef struct{

    uint32_t level_count;
    Level *level_list;

}LevelManager;

LevelManager level_manager = {0};

// grab information from config to populate levels
uint8_t level_setup(){

    // for how many levels there are, populate the following levels

    SJson *json, *leveljson, *sublevel;
    int i;

    json = sj_load("levels/base_levels.level");
    json = sj_object_get_value(json, "levels");


    level_manager.level_count =  sj_array_get_count(json);
    level_manager.level_list = (Level*) malloc(sizeof(Level)*level_manager.level_count);

    for(i = 0; i < level_manager.level_count; i++){
        int j;
        leveljson = sj_array_get_nth(json,i);

        strncpy(level_manager.level_list[i].name, sj_object_get_string(leveljson, "name"), sizeof(level_manager.level_list[i].name)-1);
        level_manager.level_list[i].name[sizeof(level_manager.level_list[i].name) - 1] = '\0';

        strncpy(level_manager.level_list[i].sky, sj_object_get_string(leveljson, "sky"), sizeof(level_manager.level_list[i].sky)-1);
        level_manager.level_list[i].sky[sizeof(level_manager.level_list[i].sky) - 1] = '\0';

        sublevel = sj_object_get_value(leveljson, "static");

        level_manager.level_list[i].static_count =  sj_array_get_count(json);
        level_manager.level_list[i].static_list = (StaticModel*) malloc(sizeof(StaticModel)*level_manager.level_list[i].static_count);

        for (){

        }

        sj_object_get_int(propjson, "framewidth", &defs[i].framewidth );
        sj_object_get_int(propjson, "frameheight", &defs[i].frameheight);
        sj_object_get_int(propjson, "framesperline", &defs[i].framesperline);

    }

    sj_free(json);

    return 0;
}

// spawn static objects
uint8_t level_build(){

    return 0;
}

// ready/ spawn entities
uint8_t level_populate(){


    return 0;
}


void level_clear(){


}

void level_delete(){

}

uint32_t *level_get_static_count(){

}


StaticModel *level_get_static(uint32_t id){




}


StaticModel *level_get_bounds(uint32_t id){




}

#endif
