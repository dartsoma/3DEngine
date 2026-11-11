#ifndef __CAMERA_CONTROLLER_H__
#define __CAMERA_CONTROLLER_H__

#include "entity_component.h"
#include "gfc_vector.h"

typedef struct {
  TransformComponent *target;
  float yaw;      // radians, 0 = looking down +Y, + turns left (CCW from above)
  float pitch;    // radians, 0 = level, + = camera above looking down
  float distance; // units behind the aim point
  float shoulder; // + shifts camera to the right (character appears left)
  float height;   // aim point height above target origin
  GFC_Vector3D abs_pos; // last computed eye position
} CameraController;

void camera_controller_reset(void);
void camera_set_target(TransformComponent *transform);
void camera_set_distance(float distance);
void camera_set_angles(float yaw, float pitch);
void camera_set_shoulder(float side, float height);
void camera_rotate(float dyaw, float dpitch);
float camera_get_yaw(void);
void camera_view_scene(void);
void camera_clear(void);

#endif
