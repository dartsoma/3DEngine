

#include "level.h"
#include "entity_core.h"
#include "gfc_color.h"
#include "gfc_hashmap.h"
#include "gfc_matrix.h"
#include "gfc_primitives.h"
#include "gfc_vector.h"
#include "model.h"
#include "simple_json.h"
#include "simple_json_value.h"
#include "simple_logger.h"

#include <stdint.h>
#include <stdlib.h>

typedef struct {
  GFC_HashMap *level_hash;
  uint32_t level_count;
  Level *level_list;
  char current_level[255];

} LevelManager;

LevelManager level_manager = {0};

// grab information from config to populate levels
uint8_t level_setup() {

  // for how many levels there are, populate the following levels

  uint32_t static_count, entity_count;
  SJson *json, *leveljson, *sublevel, *subvalue;
  StaticModel *static_model;
  EntityRegistry *entity_register;
  int i, j;

  json = sj_load("def/.levels");
  if (!json)
    return 1;

  json = sj_object_get_value(json, "levels");

  level_manager.current_level[0] = '\0';
  level_manager.level_count = sj_array_get_count(json);
  level_manager.level_hash = gfc_hashmap_new();
  level_manager.level_list =
      (Level *)malloc(sizeof(Level) * level_manager.level_count);

  for (i = 0; i < level_manager.level_count; i++) {

    leveljson = sj_array_get_nth(json, i);

    if (strlen(sj_object_get_string(leveljson, "name")) > 0) {
      strncpy(level_manager.level_list[i].name,
              sj_object_get_string(leveljson, "name"),
              sizeof(level_manager.level_list[i].name) - 1);
      level_manager.level_list[i]
          .name[sizeof(level_manager.level_list[i].name) - 1] = '\0';
    } else {
      slog("level with no name. skipped!");
      continue;
    }

    if (strlen(sj_object_get_string(leveljson, "sky")) > 0) {
      strncpy(level_manager.level_list[i].sky,
              sj_object_get_string(leveljson, "sky"),
              sizeof(level_manager.level_list[i].sky) - 1);
      level_manager.level_list[i]
          .sky[sizeof(level_manager.level_list[i].sky) - 1] = '\0';
    }

    if (strlen(sj_object_get_string(leveljson, "music")) > 0) {
      strncpy(level_manager.level_list[i].music,
              sj_object_get_string(leveljson, "music"),
              sizeof(level_manager.level_list[i].music) - 1);
      level_manager.level_list[i]
          .music[sizeof(level_manager.level_list[i].music) - 1] = '\0';
    }

    sublevel = sj_object_get_value(leveljson, "statics");

    static_count = sj_array_get_count(sublevel);
    level_manager.level_list[i].static_hash = gfc_hashmap_new();
    if (static_count > 0) {
      level_manager.level_list[i].static_count = static_count;
      level_manager.level_list[i].static_list = (StaticModel *)malloc(
          sizeof(StaticModel) * level_manager.level_list[i].static_count);

      for (j = 0; j < static_count; j++) {

        sublevel = sj_array_get_nth(sublevel, j);
        static_model = &level_manager.level_list[i].static_list[j];

        if (strlen(sj_object_get_string(sublevel, "name")) > 0) {

          strncpy(static_model->name, sj_object_get_string(sublevel, "name"),
                  sizeof(static_model->name) - 1);
          static_model->name[sizeof(static_model->name) - 1] = '\0';
          uint32_t *index = (uint32_t *)malloc(sizeof(uint32_t *));
          *index = j;
          gfc_hashmap_insert(level_manager.level_list[i].static_hash,
                             static_model->name, index);
        }

        if (strlen(sj_object_get_string(sublevel, "model")) > 0) {
          strncpy(static_model->modelpath,
                  sj_object_get_string(sublevel, "model"),
                  sizeof(static_model->modelpath) - 1);
          static_model->modelpath[sizeof(static_model->modelpath) - 1] = '\0';
          static_model->model = model_new();
          static_model->model = model_load(static_model->modelpath);
          static_model->color = GFC_COLOR_WHITE;
        }

        sj_object_get_uint8(sublevel, "collisiontype",
                            &static_model->collision_type);
        sj_object_get_uint32(sublevel, "collisionmask",
                             &static_model->collision_mask);

        subvalue = sj_object_get_value(sublevel, "position");

        sj_get_float_value(sj_array_get_nth(subvalue, 0), &static_model->pos.x);
        sj_get_float_value(sj_array_get_nth(subvalue, 1), &static_model->pos.y);
        sj_get_float_value(sj_array_get_nth(subvalue, 2), &static_model->pos.z);

        subvalue = sj_object_get_value(sublevel, "rotation");

        sj_get_float_value(sj_array_get_nth(subvalue, 0), &static_model->rot.x);
        sj_get_float_value(sj_array_get_nth(subvalue, 1), &static_model->rot.y);
        sj_get_float_value(sj_array_get_nth(subvalue, 2), &static_model->rot.z);

        subvalue = sj_object_get_value(sublevel, "scale");

        sj_get_float_value(sj_array_get_nth(subvalue, 0),
                           &static_model->scale.x);
        sj_get_float_value(sj_array_get_nth(subvalue, 1),
                           &static_model->scale.y);
        sj_get_float_value(sj_array_get_nth(subvalue, 2),
                           &static_model->scale.z);
      }
    }
    sublevel = sj_object_get_value(leveljson, "entities");

    entity_count = sj_array_get_count(sublevel);
    if (entity_count > 0) {
      level_manager.level_list[i].entity_count = entity_count;
      level_manager.level_list[i].entity_registry = (EntityRegistry *)malloc(
          sizeof(EntityRegistry) * level_manager.level_list[i].entity_count);
      for (j = 0; j < entity_count; j++) {

        sublevel = sj_array_get_nth(sublevel, j);
        entity_register = &level_manager.level_list[i].entity_registry[j];

        sj_object_get_uint8(sublevel, "type", &entity_register->type);

        subvalue = sj_object_get_value(sublevel, "position");

        sj_get_float_value(sj_array_get_nth(subvalue, 0),
                           &entity_register->pos.x);
        sj_get_float_value(sj_array_get_nth(subvalue, 1),
                           &entity_register->pos.y);
        sj_get_float_value(sj_array_get_nth(subvalue, 2),
                           &entity_register->pos.z);

        subvalue = sj_object_get_value(sublevel, "rotation");

        sj_get_float_value(sj_array_get_nth(subvalue, 0),
                           &entity_register->rot.x);
        sj_get_float_value(sj_array_get_nth(subvalue, 1),
                           &entity_register->rot.y);
        sj_get_float_value(sj_array_get_nth(subvalue, 2),
                           &entity_register->rot.z);

        subvalue = sj_object_get_value(sublevel, "scale");

        sj_get_float_value(sj_array_get_nth(subvalue, 0),
                           &entity_register->scale.x);
        sj_get_float_value(sj_array_get_nth(subvalue, 1),
                           &entity_register->scale.y);
        sj_get_float_value(sj_array_get_nth(subvalue, 2),
                           &entity_register->scale.z);

        sublevel = sj_object_get_value(leveljson, "bounds");

        subvalue = sj_object_get_value(sublevel, "dim");
        sj_get_float_value(sj_array_get_nth(subvalue, 0),
                           &level_manager.level_list[i].bounds.x);
        sj_get_float_value(sj_array_get_nth(subvalue, 0),
                           &level_manager.level_list[i].bounds.y);
        sj_get_float_value(sj_array_get_nth(subvalue, 0),
                           &level_manager.level_list[i].bounds.z);

        subvalue = sj_object_get_value(sublevel, "size");
        sj_get_float_value(sj_array_get_nth(subvalue, 0),
                           &level_manager.level_list[i].bounds.w);
        sj_get_float_value(sj_array_get_nth(subvalue, 0),
                           &level_manager.level_list[i].bounds.h);
        sj_get_float_value(sj_array_get_nth(subvalue, 0),
                           &level_manager.level_list[i].bounds.d);
      }
    }
    uint32_t *index = (uint32_t *)malloc(sizeof(uint32_t *));
    *index = i;
    gfc_hashmap_insert(level_manager.level_hash,
                       level_manager.level_list[i].name, index);
  }

  sj_free(json);
  slog("levels built");
  return 0;
}

// draw static objects
uint8_t level_build(const char *name) {

  int i;

  if (level_manager.current_level[0] == '\0') {
    if (name == NULL) {
      slog("dont seg fault mee");
      return 1;
    }
    strcpy(level_manager.current_level, name);
  }

  int *index = gfc_hashmap_get(level_manager.level_hash, name);

  if (index == NULL) {
    slog("level_build: level not found");
    return 1;
  }

  if (*index >= level_manager.level_count) {
    slog("level_build: invalid level index");
    return 1;
  }

  Level *level = &level_manager.level_list[*index];

  for (i = 0; i < level->static_count; i++) {

    // spawn mesh at these coordinates
    if (strlen(level->static_list[i].modelpath) > 0) {
      gfc_matrix4_from_vectors(
          level->static_list[i].mat, level->static_list[i].pos,
          level->static_list[i].rot, level->static_list[i].scale);

      gf3d_model_queue_render(level->static_list[i].model,
                              level->static_list[i].mat,
                              level->static_list[i].color);
    }
  }
  atexit(level_clear);

  return 0;
}

// ready/ spawn entities
uint8_t level_populate() { return 0; }

void level_clear() {

  int i, j;

  for (i = 0; i < level_manager.level_count; i++) {

    for (j = 0; j < level_manager.level_list[i].static_count; j++) {
      model_free(level_manager.level_list[i].static_list[j].model);
    }

    if (level_manager.level_list[i].static_hash) {
      gfc_hashmap_foreach(level_manager.level_list[i].static_hash, free);
      gfc_hashmap_free(level_manager.level_list[i].static_hash);
    }
    if (level_manager.level_list[i].static_list)
      free(level_manager.level_list[i].static_list);
    if (level_manager.level_list[i].entity_registry)
      free(level_manager.level_list[i].entity_registry);
  }

  if (level_manager.level_hash) {
    gfc_hashmap_foreach(level_manager.level_hash, free);
    gfc_hashmap_free(level_manager.level_hash);
  }

  if (level_manager.level_list)
    free(level_manager.level_list);

  memset(&level_manager, 0, sizeof(level_manager));
  slog("levels freed");
}

uint32_t level_get_static_count() {

  uint32_t *id = (uint32_t *)gfc_hashmap_get(level_manager.level_hash,
                                             level_manager.current_level);
  return level_manager.level_list[*id].static_count;
}

StaticModel *level_get_static(uint32_t index) {

  uint32_t *id = (uint32_t *)gfc_hashmap_get(level_manager.level_hash,
                                             level_manager.current_level);
  return &level_manager.level_list[*id].static_list[index];
}

char *level_get_name() { return level_manager.current_level; }

GFC_Box level_get_bounds() {

  uint32_t *id = (uint32_t *)gfc_hashmap_get(level_manager.level_hash,
                                             level_manager.current_level);
  return level_manager.level_list[*id].bounds;
}

const char *level_get_current() {

  if (strlen(level_manager.current_level) == 0)
    return NULL;

  return level_manager.current_level;
}

uint32_t level_get_count() { return level_manager.level_count; }

uint32_t level_get_current_index() {

  uint32_t *index;

  if (level_get_current() == NULL) {
    return level_manager.level_count;
  }

  index = (uint32_t *)gfc_hashmap_get(level_manager.level_hash,
                                      level_manager.current_level);

  return *index;
}
