#include "camera_controller.h"
#include "entity_component.h"
#include "entity_core.h"
#include "gfc_color.h"
#include "gfc_input.h"
#include "gfc_matrix.h"
#include "gfc_types.h"
#include "gfc_vector.h"
#include "model.h"
#include "simple_logger.h"

#include <stdlib.h>

#include "player.h"

// axis data
#define AXIS_MOVE 0
#define AXIS_PREVMOVE 1
#define AXIS_CAMERA 2

#define FLOAT_CAMERADISTANCE 0

void move_left(Entity *self);
void move_right(Entity *self);
void move_forward(Entity *self);
void move_backward(Entity *self);

Entity *player_create() {
  Entity *player;

  player = entity_create();

  player->free = player_free;
  player->update = player_update;
  player->think = player_think;
  player->draw = player_draw;

  player->tag |= EntityRender;
  player->tag |= EntityTransform;
  player->tag |= EntityCollision;
  player->tag |= EntityKinematics;
  player->tag |= EntityResource;

  RenderComponent *render = (RenderComponent *)entity_get_own_render(player);
  TransformComponent *transform =
      (TransformComponent *)entity_get_own_transform(player);
  KinematicsComponent *kinematics =
      (KinematicsComponent *)entity_get_own_kinematics(player);
  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(player);
  CollisionComponent *collision =
      (CollisionComponent *)entity_get_own_collision(player);

  resource->a_data = gfc_allocate_array(sizeof(EntityResourceA), 3);

  resource->a_data[AXIS_MOVE].min = -1;
  resource->a_data[AXIS_MOVE].max = 1;

  resource->a_data[AXIS_PREVMOVE].min = -1;
  resource->a_data[AXIS_PREVMOVE].max = 1;

  resource->a_data[AXIS_CAMERA].axis.x = 0;
  resource->a_data[AXIS_CAMERA].axis.y = (GFC_PI / 6);

  resource->f_data = gfc_allocate_array(sizeof(EntityResourceF), 1);

  resource->f_data[FLOAT_CAMERADISTANCE].value = 30;

  transform->scale = gfc_vector3d(1, 1, 1);
  transform->pos = gfc_vector3d(1, 1, 1);
  kinematics->vel_max = 6;
  kinematics->acc_max = 1;

  render->model = model_load("models/dino/dino.model");




  if (!render->model)

    slog("no model");

  gfc_matrix4_identity(render->mat);
  render->color = GFC_COLOR_WHITE;

  camera_set_target(transform);
  camera_set_rotation(&resource->a_data[AXIS_CAMERA].axis);
  camera_set_distance(resource->f_data[FLOAT_CAMERADISTANCE].value);

  collision->type = 1;
  collision->layer = 1;
  collision->mask = 0;

  collision->capsule = (CapsuleObject *)malloc(sizeof(CapsuleObject));
  if (!collision->capsule) {
    slog("Failed to build player casule");
    return NULL;
  }

  slog("Player Spawned!");
  return player;
}

void player_free(Entity *self) {

  if (!self)
    return;

  RenderComponent *render = (RenderComponent *)entity_get_own_render(self);
  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  free(resource->f_data);
  model_free(render->model);

  slog("Player Freed!");
  return;
}

void player_think(Entity *self) {
  if (!self)
    return;

  ResourceComponent *resource = entity_get_own_resource(self);
  KinematicsComponent *kinematics = entity_get_own_kinematics(self);
  TransformComponent *transform = entity_get_own_transform(self);

  if (!resource || !kinematics || !transform)
    return;

  resource->a_data[AXIS_MOVE].axis = gfc_vector2d(0, 0);

  if (gfc_input_key_down("UP"))
    resource->a_data[AXIS_CAMERA].axis.y -= 0.05f;

  if (gfc_input_key_down("DOWN"))
    resource->a_data[AXIS_CAMERA].axis.y += 0.05f;

  if (gfc_input_key_down("LEFT"))
    resource->a_data[AXIS_CAMERA].axis.x += 0.05f;

  if (gfc_input_key_down("RIGHT"))
    resource->a_data[AXIS_CAMERA].axis.x -= 0.05f;

  if (gfc_input_key_held("w"))
    resource->a_data[AXIS_MOVE].axis.x = 1;

  if (gfc_input_key_held("s"))
    resource->a_data[AXIS_MOVE].axis.x = -1;

  if (gfc_input_key_held("d"))
    resource->a_data[AXIS_MOVE].axis.y = 1;

  if (gfc_input_key_held("a"))
    resource->a_data[AXIS_MOVE].axis.y = -1;

  kinematics->direction = gfc_vector2d(0, 0);

  float yaw = resource->a_data[AXIS_CAMERA].axis.x;
  float forward = resource->a_data[AXIS_MOVE].axis.x;
  float strafe = resource->a_data[AXIS_MOVE].axis.y;

  kinematics->direction.x = forward * sinf(yaw) + strafe * cosf(yaw);
  kinematics->direction.y = -forward * cosf(yaw) + strafe * sinf(yaw);

}

void player_update(Entity *self) {
  if (!self)
    return;

  // phys
  TransformComponent *transform =
      (TransformComponent *)entity_get_own_transform(self);
  KinematicsComponent *kinematics =
      (KinematicsComponent *)entity_get_own_kinematics(self);
  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  kinematics_accelerate(kinematics);
  kinematics_velocity(kinematics);
  gfc_vector3d_add(transform->pos, kinematics->vel, transform->pos);

  // slog("Direction: %f, %f", kinematics->direction.x,
  // kinematics->direction.y);

  // slog("Velocity: %f, %f", kinematics->vel.x, kinematics->vel.y);

  slog("Position: %f, %f, %f", transform->pos.x, transform->pos.y,
       transform->pos.z);
  //
  // slog("camera->rot: %f, %f", resource->a_data[AXIS_CAMERA].axis.x,
  // resource->a_data[AXIS_CAMERA].axis.y);
  return;
}

void player_draw(Entity *self) {
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

void move_left(Entity *self) {
  if (!self)
    return;

  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  resource->a_data[AXIS_MOVE].axis.x -= 1;
}
void move_right(Entity *self) {
  if (!self)
    return;

  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  resource->a_data[AXIS_MOVE].axis.x += 1;
}
void move_forward(Entity *self) {
  if (!self)
    return;

  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  resource->a_data[AXIS_MOVE].axis.y -= 1;
}

void move_backward(Entity *self) {
  if (!self)
    return;

  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  resource->a_data[AXIS_MOVE].axis.y += 1;
}
