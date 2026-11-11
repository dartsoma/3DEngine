#include "camera_controller.h"
#include "gf3d_vgraphics.h"
#include "gfc_matrix.h"
#include "gfc_vector.h"
#include <math.h>

#define CAM_PITCH_MIN 0.15f
#define CAM_PITCH_MAX 1.40f

static const CameraController camera_defaults = {
    .target = NULL,
    .yaw = 0.0f,
    .pitch = 0.35f,
    .distance = -10.0f,
    .shoulder = 1.2f,
    .height = 1.5f,
};

CameraController camera_controller = {
    .pitch = 0.35f, .distance = 10.0f, .shoulder = 1.2f, .height = 1.5f};

void camera_controller_reset(void) { camera_controller = camera_defaults; }
void camera_clear(void) { camera_controller_reset(); }

void camera_set_target(TransformComponent *transform) {
  camera_controller.target =
      transform; // NULL is fine, camera falls back to origin
}

void camera_set_distance(float distance) {
  camera_controller.distance = distance < 1.0f ? 1.0f : distance;
}

static float clampf(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

void camera_set_angles(float yaw, float pitch) {
  camera_controller.yaw = yaw;
  camera_controller.pitch = clampf(pitch, CAM_PITCH_MIN, CAM_PITCH_MAX);
}

void camera_set_shoulder(float side, float height) {
  camera_controller.shoulder = side;
  camera_controller.height = height;
}

void camera_rotate(float dyaw, float dpitch) {
  camera_controller.yaw =
      fmodf(camera_controller.yaw + dyaw, 2.0f * (float)M_PI);
  camera_controller.pitch =
      clampf(camera_controller.pitch + dpitch, CAM_PITCH_MIN, CAM_PITCH_MAX);
}

float camera_get_yaw(void) { return camera_controller.yaw; }

void camera_view_scene(void) {
  CameraController *c = &camera_controller;
  GFC_Matrix4 *view = gf3d_vgraphics_get_view_matrix();

  float sy = sinf(c->yaw), cy = cosf(c->yaw);
  float sp = sinf(c->pitch), cp = cosf(c->pitch);

  GFC_Vector3D focus = c->target ? c->target->pos : gfc_vector3d(0, 0, 0);

  focus.x += cy * c->shoulder;
  focus.y += sy * c->shoulder;
  focus.z += c->height;

  // forward (horizontal) = (-sin yaw, cos yaw, 0); eye sits behind and above
  GFC_Vector3D eye =
      gfc_vector3d(focus.x + c->distance * cp * sy,
                   focus.y - c->distance * cp * cy, focus.z + c->distance * sp);

  c->abs_pos = eye;
  gfc_matrix4_view(*view, eye, focus, gfc_vector3d(0, 0, 1));
}
