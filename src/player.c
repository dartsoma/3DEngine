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

  resource->a_data = gfc_allocate_array(sizeof(EntityResourceA), 1);
  resource->a_data[AXIS_MOVE].min = -1;
  resource->a_data[AXIS_MOVE].max = 1;

  resource->f_data = gfc_allocate_array(sizeof(EntityResourceF), 1);
  resource->f_data[CAM_DISTANCE].value = 10;
  resource->f_data[CAM_DISTANCE].min = 0;

  resource->v3_data = gfc_allocate_array(sizeof(GFC_Vector3D), 2);
  gfc_vector3d_copy(resource->v3_data[CAM_OFFSET], gfc_vector3d(0, -2, 0));
  gfc_vector3d_copy(resource->v3_data[CAM_ROTATION],
                    gfc_vector3d(-2 * GFC_PI / 6, 0, 0));

  transform->scale = gfc_vector3d(1, 1, 1);

  // remember to divide by 60
  kinematics->vel_max = 10;
  kinematics->acc_max = 5;

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

  if (gfc_input_command_down("panco")) {
    resource->v3_data[CAM_ROTATION].x += 0.05;
  }
  if (gfc_input_command_down("pandown")) {
    resource->v3_data[CAM_ROTATION].x -= 0.05;
  }
  if (gfc_input_command_down("panright")) {
    resource->v3_data[CAM_ROTATION].y -= 0.05;
  }
  if (gfc_input_command_down("panleft")) {
    resource->v3_data[CAM_ROTATION].y += 0.05;
  }
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

void player_update_camera(TransformComponent *transform,
                          ResourceComponent *resource) {
  GFC_Matrix4 *view;
  GFC_Vector3D offset, target, eye;
  float dist, pitch, yaw, horiz;

  offset = resource->v3_data[CAM_OFFSET];
  pitch = resource->v3_data[CAM_ROTATION].x;
  yaw = resource->v3_data[CAM_ROTATION].z;

  dist = resource->f_data[CAM_DISTANCE].value;
  if (dist < resource->f_data[CAM_DISTANCE].min)
    dist = resource->f_data[CAM_DISTANCE].min;

  if (pitch > 1.5f)
    pitch = 1.5f;
  if (pitch < 0.05f)
    pitch = 0.05f;

  target =
      gfc_vector3d(transform->pos.x + offset.x, transform->pos.y + offset.y,
                   transform->pos.z + offset.z);

  horiz = dist * cosf(pitch);
  eye = gfc_vector3d(target.x + sinf(yaw) * horiz, target.y - cosf(yaw) * horiz,
                     target.z + sinf(pitch) * dist);

  view = gf3d_vgraphics_get_view_matrix();
  gfc_matrix4_view(*view, eye, target, gfc_vector3d(0, 0, 1));
}

void player_update(Entity *self) {
  if (!self)
    return;

  GFC_Matrix4 *view;
  GFC_Vector3D cam_offset, cam_rot, cam_pos;
  float cam_distance;

  // phys
  TransformComponent *transform =
      (TransformComponent *)entity_get_own_transform(self);
  KinematicsComponent *kinematics =
      (KinematicsComponent *)entity_get_own_kinematics(self);
  ResourceComponent *resource =
      (ResourceComponent *)entity_get_own_resource(self);

  kinematics_accelerate(kinematics, &resource->a_data[AXIS_MOVE].axis);
  // i just need a collision for grounded
  // kinematics_fall(kinematics);
  kinematics_velocity(kinematics);

  gfc_vector3d_add(transform->pos, kinematics->vel, transform->pos);

  cam_offset = resource->v3_data[CAM_OFFSET];
  cam_rot = resource->v3_data[CAM_ROTATION];

  cam_distance = resource->f_data[CAM_DISTANCE].value;

  if (cam_distance < resource->f_data[CAM_DISTANCE].min)
    cam_distance = resource->f_data[CAM_DISTANCE].min;

  view = gf3d_vgraphics_get_view_matrix();

  gfc_vector3d_scale(cam_offset, cam_offset, cam_distance);
  gfc_vector3d_rotate_about_y(&cam_offset, cam_rot.x);
  gfc_vector3d_rotate_about_z(&cam_offset, cam_rot.z);
  gfc_vector3d_add(cam_pos, cam_offset, transform->pos);

  view = gf3d_vgraphics_get_view_matrix();
  gfc_matrix4_view(*view, cam_pos, transform->pos, gfc_vector3d(0, 0, 1));

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
