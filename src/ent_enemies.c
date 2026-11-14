#include "ent_enemies.h"
#include "entity_component.h"
#include "entity_core.h"
#include "gfc_types.h"
#include "simple_logger.h"
#include <stdint.h>

// 0 - null, 1 - Gopher, 2 - School, 3 - Angler, 4 - Fatso, 5 - Stone

#define ENEMY_TAG 0

Entity *enemy_create(uint8_t tag) {
  Entity *enemy;

  enemy = entity_create();

  enemy->free = enemy_free;
  enemy->update = enemy_update;
  enemy->think = enemy_think;
  enemy->draw = enemy_draw;

  enemy->tag |= EntityRender;
  enemy->tag |= EntityTransform;
  enemy->tag |= EntityCollision;
  enemy->tag |= EntityKinematics;
  enemy->tag |= EntityResource;

  RenderComponent *render = (RenderComponent *)entity_get_own_render(enemy);
  TransformComponent *transform =
      (TransformComponent *)entity_get_own_transform(enemy);
  KinematicsComponent *kinematics =
      (KinematicsComponent *)entity_get_own_kinematics(enemy);
  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(enemy);
  CollisionComponent *collision =
      (CollisionComponent *)entity_get_own_collision(enemy);

  transform->scale = gfc_vector3d(1, 1, 1);
  transform->pos = gfc_vector3d(0, 0, 0);

  resource->i_data = gfc_allocate_array(sizeof(EntityResourceI), 1);

  switch (tag) {

  case 1:
    render->model = model_load("models/dino/dino.model");

    if (!render->model)
      slog("no model for Gopher enemy");
    break;
  case 2:
    render->model = model_load("models/dino/dino.model");

    if (!render->model)
      slog("no model for School enemy");
    break;
  case 3:
    render->model = model_load("models/dino/dino.model");

    if (!render->model)
      slog("no model for Angler enemy");
    break;
  case 4:
    render->model = model_load("models/dino/dino.model");

    if (!render->model)
      slog("no model for Fatso enemy");
    break;
  case 5:
    render->model = model_load("models/dino/dino.model");

    if (!render->model)
      slog("no model for Stone enemy");
    break;
  default:

    slog("null enemy created and scrapped");
    enemy_free(enemy);
    return NULL;
    break;
  }

  render->model = model_load("models/dino/dino.model");

  gfc_matrix4_identity(render->mat);
  render->color = GFC_COLOR_WHITE;

  collision->type = 0;
  collision->layer = 2;
  collision->mask = 0;

  return enemy;
}

void enemy_spawn_at(uint8_t tag, Transform *transform) {

  Entity *enemy = enemy_create(tag);

  TransformComponent *oldtransform = entity_get_own_transform(enemy);

  oldtransform->pos = *transform->pos;
}

void enemy_free(Entity *self) {

  if (!self)
    return;
  RenderComponent *render = (RenderComponent *)entity_get_own_render(self);
  TransformComponent *transform =
      (TransformComponent *)entity_get_own_transform(self);
  KinematicsComponent *kinematics =
      (KinematicsComponent *)entity_get_own_kinematics(self);
  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);
  CollisionComponent *collision =
      (CollisionComponent *)entity_get_own_collision(self);

  uint8_t tag = (uint8_t)resource->i_data[ENEMY_TAG].value;

  switch (tag) {
  default:
    break;
  }

  if (resource->f_data)
    free(resource->f_data);
  if (resource->i_data)
    free(resource->i_data);
  if (resource->a_data)
    free(resource->a_data);
  if (resource->v3_data)
    free(resource->v3_data);

  model_free(render->model);

  memset(render, 0, sizeof(*render));
  memset(transform, 0, sizeof(*transform));
  memset(kinematics, 0, sizeof(*kinematics));
  memset(collision, 0, sizeof(*collision));

  slog("Enemy Freed!");
}

void enemy_think(Entity *self) {
  if (!self)
    return;

  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  uint8_t tag = (uint8_t)resource->i_data[ENEMY_TAG].value;

  switch (tag) {
  default:
    break;
  }
}

void enemy_update(Entity *self) {
  if (!self)
    return;
  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  uint8_t tag = (uint8_t)resource->i_data[ENEMY_TAG].value;

  switch (tag) {
  default:
    break;
  }
}

void enemy_draw(Entity *self) {
  if (!self)
    return;

  RenderComponent *render = (RenderComponent *)entity_get_own_render(self);
  TransformComponent *transform =
      (TransformComponent *)entity_get_own_transform(self);

  gfc_matrix4_from_vectors(render->mat, transform->pos, transform->rot,
                           transform->scale);
  gf3d_model_queue_render(render->model, render->mat, render->color);
  return;
}
