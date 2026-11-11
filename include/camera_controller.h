#ifndef __CAMERA_CONTROLLER_H__
#define __CAMERA_CONTROLLER_H__

#include "entity_component.h"
#include "gf3d_camera.h"
#include "gfc_vector.h"

typedef struct {

  TransformComponent *target;

  float camera_lag;
  float camera_distance;
  float zoom;
  GFC_Vector2D *rot;
  GFC_Vector3D offset;
  GFC_Vector3D abs_pos;
} CameraController;

// reset vamera values
void camera_controller_reset();

void camera_look_at_target();

// assumes entity checked for transform already before setting camera target
// DO ONCE don't call every frame
void camera_set_target(TransformComponent *transform);
void camera_set_rotation(GFC_Vector2D *rotation);

// stores a transform to be used as its own position
void camera_follow_target();

void camera_set_distance(float distance);

void camera_view_scene();

void camera_clear();
#endif
