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

// vector3 data
#define CAM_OFFSET 0
#define CAM_ROTATION 1

// float data
#define CAM_DISTANCE 0

// axisdata
#define AXIS_MOVE 0
#define AXIS_CAM 1

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

  resource->a_data = gfc_allocate_array(sizeof(EntityResourceA), 2);
  resource->a_data[AXIS_MOVE].min = -1;
  resource->a_data[AXIS_MOVE].max = 1;

  resource->f_data = gfc_allocate_array(sizeof(EntityResourceF), 1);
  resource->f_data[CAM_DISTANCE].value = 20;
  resource->f_data[CAM_DISTANCE].min = 0;

  resource->v3_data = gfc_allocate_array(sizeof(GFC_Vector3D), 2);
  gfc_vector3d_copy(resource->v3_data[CAM_OFFSET], gfc_vector3d(0, -2, 0));

  // stores a transform to be used as its own position
  camera_set_target(transform);
  camera_set_distance(10.0f);
  camera_set_angles(0.0f, 0.35f);
  camera_set_shoulder(1.2f, 1.5f);

  kinematics->vel_max = 0.15f;
  kinematics->acc_max = 1;

  transform->scale = gfc_vector3d(1, 1, 1);

  render->model = model_load("models/dino/dino.model");
  gfc_matrix4_identity(render->mat);
  render->color = GFC_COLOR_WHITE;

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
  // controls

  ResourceComponent *resource = entity_get_own_resource(self);

  resource->a_data[AXIS_MOVE].axis.x = 0;
  resource->a_data[AXIS_MOVE].axis.y = 0;

  if (gfc_input_key_down("UP"))
    camera_rotate(0, -0.04f); // look up = camera lowers
  if (gfc_input_key_down("DOWN"))
    camera_rotate(0, 0.04f);
  if (gfc_input_key_down("LEFT"))
    camera_rotate(0.04f, 0);
  if (gfc_input_key_down("RIGHT"))
    camera_rotate(-0.04f, 0);

  if (gfc_input_command_down("item_up")) {
    move_forward(self);
  }
  if (gfc_input_command_down("item_down")) {
    move_backward(self);
  }
  if (gfc_input_command_down("item_left")) {
    move_left(self);
  }
  if (gfc_input_command_down("item_right")) {
    move_right(self);
  }

  return;
}

void player_update(Entity *self) {
  if (!self)
    return;

  TransformComponent *transform = entity_get_own_transform(self);
  KinematicsComponent *kinematics = entity_get_own_kinematics(self);
  ResourceComponent *resource = entity_get_own_resource(self);

  // rotate input (x = strafe right, y = forward) by camera yaw into world XY
  float yaw = camera_get_yaw();
  GFC_Vector2D in = resource->a_data[AXIS_MOVE].axis;
  GFC_Vector2D world = gfc_vector2d(in.x * cosf(yaw) - in.y * sinf(yaw),
                                    in.x * sinf(yaw) + in.y * cosf(yaw));

  kinematics_accelerate(kinematics, &world);
  kinematics_velocity(kinematics);
  gfc_vector3d_add(transform->pos, kinematics->vel, transform->pos);
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

  resource->a_data[AXIS_MOVE].axis.y += 1;
}

void move_backward(Entity *self) {
  if (!self)
    return;

  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  resource->a_data[AXIS_MOVE].axis.y -= 1;
}
