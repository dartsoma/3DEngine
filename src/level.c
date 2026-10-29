
#include "entity_core.h"
#include "gfc_hashmap.h"
#include "gfc_primitives.h"
#include "gfc_vector.h"
#include "model.h"
#include "simple_json.h"
#include "simple_json_value.h"
#include "simple_logger.h"

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
  GFC_Vector3D pos;
  GFC_Vector3D rot;
  GFC_Vector3D scale;

  uint8_t collision_type; // 0 - simple (bounding box), 1 - complex (mesh), 2 -
                          // sphere (mesh)

} StaticModel;

typedef struct {
  GFC_HashMap static_hash;
  uint32_t static_count;
  StaticModel *static_list;
  uint32_t entity_count;
  EntityRegistry *entity_registry;
  char name[255];
  char sky[255];
  char music[255];
  GFC_Box bounds;
} Level;

typedef struct {

  uint32_t level_count;
  Level *level_list;

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

  level_manager.level_count = sj_array_get_count(json);
  level_manager.level_list =
      (Level *)malloc(sizeof(Level) * level_manager.level_count);

  for (i = 0; i < level_manager.level_count; i++) {

    leveljson = sj_array_get_nth(json, i);

    strncpy(level_manager.level_list[i].name,
            sj_object_get_string(leveljson, "name"),
            sizeof(level_manager.level_list[i].name) - 1);
    level_manager.level_list[i]
        .name[sizeof(level_manager.level_list[i].name) - 1] = '\0';

    strncpy(level_manager.level_list[i].sky,
            sj_object_get_string(leveljson, "sky"),
            sizeof(level_manager.level_list[i].sky) - 1);
    level_manager.level_list[i]
        .sky[sizeof(level_manager.level_list[i].sky) - 1] = '\0';

    strncpy(level_manager.level_list[i].music,
            sj_object_get_string(leveljson, "music"),
            sizeof(level_manager.level_list[i].music) - 1);
    level_manager.level_list[i]
        .music[sizeof(level_manager.level_list[i].music) - 1] = '\0';

    sublevel = sj_object_get_value(leveljson, "statics");

    static_count = sj_array_get_count(json);
    if (static_count > 0) {
      level_manager.level_list[i].static_count = static_count;
      level_manager.level_list[i].static_list = (StaticModel *)malloc(
          sizeof(StaticModel *) * level_manager.level_list[i].static_count);

      for (j = 0; j < static_count; j++) {

        sublevel = sj_array_get_nth(sublevel, j);
        static_model = &level_manager.level_list[i].static_list[j];

        strncpy(static_model->name, sj_object_get_string(sublevel, "name"),
                sizeof(static_model->name) - 1);
        static_model->name[sizeof(static_model->name) - 1] = '\0';

        strncpy(static_model->modelpath,
                sj_object_get_string(sublevel, "model"),
                sizeof(static_model->modelpath) - 1);
        static_model->modelpath[sizeof(static_model->modelpath) - 1] = '\0';

        sj_object_get_uint8(sublevel, "collisions",
                            &static_model->collision_type);

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

    entity_count = sj_array_get_count(json);
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

        sublevel = sj_object_get_value(json, "bounds");

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
  }

  sj_free(json);
  slog("we did it!");
  return 0;
}

// spawn static objects
uint8_t level_build() { return 0; }

// ready/ spawn entities
uint8_t level_populate() { return 0; }

void level_clear() {}

void level_delete() {}

uint32_t *level_get_static_count() { return 0; }

StaticModel *level_get_static(uint32_t id) { return 0; }

StaticModel *level_get_bounds(uint32_t id) { return 0; }
