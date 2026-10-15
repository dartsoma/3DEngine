#include "entity_core.h"
#include "cf_queue.h"
#include "gfc_hashmap.h"
#include "gfc_types.h"
#include "simple_logger.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {

  Entity *entity_list;

  GFC_HashMap *entity_hash; // Converts id to entity index in hashmap
  CF_Queue *open_indices;   // Free slots for placing entities

  uint32_t next_id;
  uint32_t entity_max;

  TransformComponent *transform_list;

  KinematicsComponent *kinematics_list;

  ResourceComponent *resource_list;

  CollisionComponent *collision_list;

  RenderComponent *render_list;

} EntityManager;

EntityManager entity_manager = {0};

void entity_manager_init(uint32_t max) {

  entity_manager.entity_list = gfc_allocate_array(sizeof(Entity), max);

  entity_manager.entity_hash = gfc_hashmap_new();
  entity_manager.open_indices = cf_queue_init(); // Free slots for placing entities

  entity_manager.next_id = 0;
  entity_manager.entity_max = max;
  entity_manager.transform_list =
      gfc_allocate_array(sizeof(TransformComponent), max);
  entity_manager.kinematics_list =
      gfc_allocate_array(sizeof(KinematicsComponent), max);
  entity_manager.resource_list =
      gfc_allocate_array(sizeof(ResourceComponent), max);
  entity_manager.collision_list =
      gfc_allocate_array(sizeof(CollisionComponent), max);
  entity_manager.render_list =
      gfc_allocate_array(sizeof(RenderComponent), max);

  atexit(entity_manager_close);
}

void entity_manager_close() {
  int i;

  if (!entity_manager.entity_list)
    return;

  cf_queue_clear(entity_manager.open_indices);

  for (i = 0; i < entity_manager.entity_max; i++) {
    entity_free(&entity_manager.entity_list[i]);
  }

  gfc_hashmap_foreach(entity_manager.entity_hash, free);
  gfc_hashmap_free(entity_manager.entity_hash);
  entity_delete_components();
  free(entity_manager.entity_list);

}

void entity_free(Entity *entity){

    if (!entity)return;
    memset(entity, 0, sizeof(Entity));
    free(entity);

}

Entity *entity_create(){

    int i;

    for (i = 0; i < entity_manager.entity_max; i++){
        if (entity_manager.entity_list[i]._inuse) continue;
        entity_manager.entity_list[i]._inuse = 1;
        uint32_t* index = (uint32_t*)malloc(sizeof(uint32_t));
        *index = i;

        uint32_t nextid;
        nextid = entity_manager.next_id;

        sprintf(entity_manager.entity_list[i].id, "%ud", nextid);
        gfc_hashmap_insert(entity_manager.entity_hash, entity_manager.entity_list[i].id, index);
        return &entity_manager.entity_list[i];
    }

    slog("Not enough space for more entities");
    return NULL;
}



uint8_t entity_is_valid(uint32_t id){

if (entity_get_reference(id) != NULL){
    return 1;
}

return 0;

}

// Draw models
void entity_manager_draw(){





}


void entity_manager_think(){




}

void entity_manager_update(){


}

void entity_draw(uint32_t id){


}

void entity_add_component(uint32_t id, EntityComponentType type, void *data){

    switch (type) {
    case EntityTransform:
      memset(&entity_manager.transform_list[id], 0, sizeof(entity_manager.transform_list[id]));
      break;
    case EntityKinematics:
      memset(&entity_manager.kinematics_list[id], 0, sizeof(entity_manager.kinematics_list[id]));
      break;
    case EntityResource:
        free(entity_manager.resource_list[id].f_data);
        free(entity_manager.resource_list[id].i_data);
      memset(&entity_manager.resource_list[id], 0, sizeof(entity_manager.resource_list[id]));
      break;
    case EntityCollision:
      free(entity_manager.collision_list[id].linetrace);
      memset(&entity_manager.collision_list[id], 0, sizeof(entity_manager.collision_list[id]));
      break;
    case EntityRender:
      model_free(entity_manager.render_list[id].model);
      memset(&entity_manager.render_list[id], 0, sizeof(entity_manager.render_list[id]));
      break;
    }

}

void entity_delete_components(){

    if(entity_manager.transform_list){
    free(entity_manager.transform_list);
    }

    if(entity_manager.kinematics_list){
    free(entity_manager.kinematics_list);
    }

    if(entity_manager.resource_list){
    free(entity_manager.resource_list);
    }

    if(!entity_manager.collision_list){
    free(entity_manager.collision_list);
    }

    if(!entity_manager.render_list){
    free(entity_manager.render_list);
    }
}

void entity_remove_component(uint32_t id, EntityComponentType type) {

  switch (type) {
  case EntityTransform:
    memset(&entity_manager.transform_list[id], 0, sizeof(entity_manager.transform_list[id]));
    break;
  case EntityKinematics:
    memset(&entity_manager.kinematics_list[id], 0, sizeof(entity_manager.kinematics_list[id]));
    break;
  case EntityResource:
      free(entity_manager.resource_list[id].f_data);
      free(entity_manager.resource_list[id].i_data);
    memset(&entity_manager.resource_list[id], 0, sizeof(entity_manager.resource_list[id]));
    break;
  case EntityCollision:
    free(entity_manager.collision_list[id].linetrace);
    memset(&entity_manager.collision_list[id], 0, sizeof(entity_manager.collision_list[id]));
    break;
  case EntityRender:
    model_free(entity_manager.render_list[id].model);
    memset(&entity_manager.render_list[id], 0, sizeof(entity_manager.render_list[id]));
    break;
  }
}

Entity *entity_get_reference(uint32_t id){
    int i;
    char str[11];
    uint32_t *index;

    sprintf(str, "%ud", id);
    index = (uint32_t*) gfc_hashmap_get(entity_manager.entity_hash, str);

    if(index!=NULL){

        return &entity_manager.entity_list[*index];

    }

    return NULL;
}

void entity_remove_all_components(uint32_t id) {
  int i;
  Entity *ent;

  ent = entity_get_reference(id);

  if (!ent)
    return;

  uint32_t new_id = atoi(ent->id);

  for (i = 0; i < ALLCOMPONENTS; i++) {
    if (ent->tag == 0)
      continue;

    if ((ent->tag >> i) & 1) {
      entity_remove_component(new_id, (1 << i));
    }
  }
}

void *entity_get_component(uint32_t id, EntityComponentType type) {

    void *component;
    char str[11];
    sprintf(str, "%ud", id);

    component = gfc_hashmap_get(entity_manager.entity_hash, str);

    if (component == NULL) return NULL;

    return component;
}
