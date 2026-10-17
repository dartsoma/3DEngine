#include <stdio.h>
#include <stdlib.h>
#include "cf_queue.h"
#include "gfc_hashmap.h"
#include "gfc_types.h"
#include "simple_logger.h"
#include "entity_core.h"
#include "entity_component.h"

typedef struct {

  Entity *entity_list;

  GFC_HashMap *entity_hash; // Converts id to entity index in hashmap
  CF_Queue *open_indices; // Free slots for placing entities

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
  entity_delete_components(); // ensure all components are reset and deleted
  free(entity_manager.entity_list);

}

void entity_free(Entity *entity){

    if (!entity)return;

    if (entity->free)
        entity->free(entity);

    if (entity_manager.open_indices)
        cf_enqueue(entity_manager.open_indices, &entity);

    free(gfc_hashmap_get(entity_manager.entity_hash, entity->id));
    gfc_hashmap_delete_by_key(entity_manager.entity_hash, entity->id);

    memset(entity, 0, sizeof(Entity));
}

Entity *entity_create(){

    int i;
    uint32_t* index;
    uint32_t nextid;

    if (entity_manager.next_id < entity_manager.entity_max){

    for (i = 0; i < entity_manager.entity_max; i++){
        if (entity_manager.entity_list[i]._inuse) continue;
        entity_manager.entity_list[i]._inuse = 1;

        nextid = entity_manager.next_id;
        entity_manager.next_id++;

        index = (uint32_t*)malloc(sizeof(uint32_t));
        *index = i;
        sprintf(entity_manager.entity_list[i].id, "%ud", nextid);
        gfc_hashmap_insert(entity_manager.entity_hash, entity_manager.entity_list[i].id, index);
        return &entity_manager.entity_list[i];
    }
    }

    Entity *ent;

    ent = (Entity*) cf_dequeue(entity_manager.open_indices);
    if (ent != NULL){
        ent->_inuse = 1;

        nextid = entity_manager.next_id;
        entity_manager.next_id++;

        index = (uint32_t*) malloc(sizeof(uint32_t));
        *index = (uint32_t) (ent - entity_manager.entity_list);

        sprintf(ent->id, "%ud", nextid);
        gfc_hashmap_insert(entity_manager.entity_hash, ent->id, index);
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

    int i;

    for(i=0; i<entity_manager.entity_max; i++) {
        if (!entity_manager.entity_list[i]._inuse) continue;
        if (entity_manager.entity_list[i].draw){
            entity_manager.entity_list[i].draw(&entity_manager.entity_list[i]);
        }

    }
}


void entity_manager_think(){

    int i;

    for(i=0; i<entity_manager.entity_max; i++) {
        if (!entity_manager.entity_list[i]._inuse) continue;

        if (entity_manager.entity_list[i].think){
            entity_manager.entity_list[i].think(&entity_manager.entity_list[i]);
        }
    }
}

void entity_manager_update(){

    int i;

    for(i=0; i<entity_manager.entity_max; i++) {
        if (!entity_manager.entity_list[i]._inuse) continue;

        if (entity_manager.entity_list[i].update){
            entity_manager.entity_list[i].update(&entity_manager.entity_list[i]);
        }
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

    uint32_t index;
    index = entity_get_index(id);

    if(index == 0 && id != 0)
        return NULL;
    if (!entity_manager.entity_list[index]._inuse)
        return NULL;

    return &entity_manager.entity_list[index];

}

uint32_t entity_get_index(uint32_t id){
    char str[11];
    uint32_t *index;
    sprintf(str, "%ud", id);

    if (gfc_hashmap_get(entity_manager.entity_hash, str) == NULL)
        return 0;

    index = (uint32_t*) gfc_hashmap_get(entity_manager.entity_hash, str);

    return *index;
}

void entity_remove_all_components(uint32_t id) {
  int i;
  Entity *ent;

  ent = entity_get_reference(id);

  if (!ent)
    return;

  char *end_ptr;
  uint32_t new_id = id = strtoul(ent->id, &end_ptr, 10);

  for (i = 0; i < ALLCOMPONENTS; i++) {
    if (ent->tag == 0)
      continue;

    if ((ent->tag >> i) & 1) {
      entity_remove_component(new_id, (1 << i));
    }
  }
}


TransformComponent *entity_get_transform(uint32_t id){


    uint32_t index;
    Entity *ent;

    ent = entity_get_reference(id);

    if(!ent)
        return NULL;
    index = entity_get_index(id);

    if ((ent->tag & EntityTransform) != 0){
        return &entity_manager.transform_list[index];
    }

    slog("Entity %d has no Transform tag", id);
    return NULL;
}

KinematicsComponent *entity_get_kinematics(uint32_t id){
        uint32_t index;
        Entity *ent;

        ent = entity_get_reference(id);

        if(!ent)
            return NULL;
        index = entity_get_index(id);

        if ((ent->tag & EntityKinematics) != 0){
            return &entity_manager.kinematics_list[index];
        }

        slog("Entity %d has no Kinematics tag", id);
        return NULL;
}
ResourceComponent *entity_get_resource(uint32_t id){
    uint32_t index;
    Entity *ent;

    ent = entity_get_reference(id);

    if(!ent)
        return NULL;
    index = entity_get_index(id);

    if ((ent->tag & EntityResource) != 0){
        return &entity_manager.resource_list[index];
    }

    slog("Entity %d has no Resource tag", id);
    return NULL;
}

CollisionComponent *entity_get_collision(uint32_t id){
    uint32_t index;
    Entity *ent;

    ent = entity_get_reference(id);

    if(!ent)
        return NULL;
    index = entity_get_index(id);

    if ((ent->tag & EntityCollision) != 0){
        return &entity_manager.collision_list[index];
    }

    slog("Entity %d has no Collision tag", id);
    return NULL;
}

RenderComponent *entity_get_render(uint32_t id){
    uint32_t index;
    Entity *ent;

    ent = entity_get_reference(id);

    if(!ent)
        return NULL;
    index = entity_get_index(id);

    if ((ent->tag & EntityRender) != 0){
        return &entity_manager.render_list[index];
    }

    slog("Entity %d has no Render tag", id);
    return NULL;
}


TransformComponent *entity_get_own_transform(Entity *self){
    char *endptr;
    uint32_t id = strtoul(self->id, &endptr, 10);
    return entity_get_transform(id);
}
KinematicsComponent *entity_get_own_kinematics(Entity *self){
    char *endptr;
    uint32_t id = strtoul(self->id, &endptr, 10);
    return entity_get_kinematics(id);
}
ResourceComponent *entity_get_own_resource(Entity *self){
    char *endptr;
    uint32_t id = strtoul(self->id, &endptr, 10);
    return entity_get_resource(id);
}
CollisionComponent *entity_get_own_collisions(Entity *self){
    char *endptr;
    uint32_t id = strtoul(self->id, &endptr, 10);
    return entity_get_collision(id);
}
RenderComponent *entity_get_own_render(Entity *self){
    char *endptr;
    uint32_t id = strtoul(self->id, &endptr, 10);
    return entity_get_render(id);
}
