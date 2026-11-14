#include "ent_props.h"
#include "entity_component.h"
#include "entity_core.h"
#include "gfc_types.h"
#include "simple_logger.h"
#include <stdint.h>

// 0 - null, 1 - Gopher, 2 - School, 3 - Angler, 4 - Fatso, 5 - Stone

#define PROP_TAG 0

Entity *prop_create(uint8_t tag) {
  Entity *prop;

  prop = entity_create();

  prop->free = prop_free;
  prop->update = prop_update;
  prop->think = prop_think;
  prop->draw = prop_draw;

  prop->tag |= EntityRender;
  prop->tag |= EntityTransform;
  prop->tag |= EntityCollision;
  prop->tag |= EntityResource;

  RenderComponent *render = (RenderComponent *)entity_get_own_render(prop);
  TransformComponent *transform =
      (TransformComponent *)entity_get_own_transform(prop);
  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(prop);
  CollisionComponent *collision =
      (CollisionComponent *)entity_get_own_collision(prop);

  transform->scale = gfc_vector3d(1, 1, 1);
  transform->pos = gfc_vector3d(0, 0, 0);

  resource->i_data = gfc_allocate_array(sizeof(EntityResourceI), 1);

  switch (tag) {

  case 1:
    render->model = model_load("models/dino/dino.model");

    if (!render->model)

      break;
  case 2:
    render->model = model_load("models/dino/dino.model");

    if (!render->model)

      break;
  case 3:
    render->model = model_load("models/dino/dino.model");

    if (!render->model)

      break;
  case 4:
    render->model = model_load("models/dino/dino.model");

    if (!render->model)

      break;
  case 5:
    render->model = model_load("models/dino/dino.model");

    if (!render->model)

      break;
  default:

    slog("null prop created and scrapped");
    prop_free(prop);
    return NULL;
    break;
  }

  render->model = model_load("models/dino/dino.model");

  gfc_matrix4_identity(render->mat);
  render->color = GFC_COLOR_WHITE;

  collision->type = 0;
  collision->layer = 2;
  collision->mask = 0;

  return prop;
}

void prop_spawn_at(uint8_t tag, Transform *transform) {

  Entity *prop = prop_create(tag);

  TransformComponent *oldtransform = entity_get_own_transform(enemy);

  oldtransform->pos = *transform->pos;
}

void prop_free(Entity *self) {

  if (!self)
    return;
  RenderComponent *render = (RenderComponent *)entity_get_own_render(self);
  TransformComponent *transform =
      (TransformComponent *)entity_get_own_transform(self);
  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);
  CollisionComponent *collision =
      (CollisionComponent *)entity_get_own_collision(self);

  uint8_t tag = (uint8_t)resource->i_data[PROP_TAG].value;

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
  memset(collision, 0, sizeof(*collision));

  slog("prop Freed!");
}

void prop_think(Entity *self) {
  if (!self)
    return;

  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  uint8_t tag = (uint8_t)resource->i_data[PROP_TAG].value;

  switch (tag) {
  default:
    break;
  }
}

void prop_update(Entity *self) {
  if (!self)
    return;
  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  uint8_t tag = (uint8_t)resource->i_data[PROP_TAG].value;

  switch (tag) {
  default:
    break;
  }
}

void prop_draw(Entity *self) {
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
