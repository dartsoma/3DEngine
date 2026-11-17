#include "camera_controller.h"
#include "gfc_matrix.h"
#include "gfc_vector.h"
#include "simple_logger.h"
#include <stddef.h>
#include <wchar.h>

CameraController camera_controller = {0};

void camera_controller_reset() {

  memset(&camera_controller, 0, sizeof(camera_controller));
}

void camera_view_scene() {

  GFC_Matrix4 *view = gf3d_vgraphics_get_view_matrix();
  GFC_Vector3D camera_pos, target_pos;

  if (!camera_controller.target || !camera_controller.rot) {
    return;
  }

  camera_pos = gfc_vector3d(0, 1, 0);

  target_pos = camera_controller.target->pos;

  gfc_vector3d_scale(camera_pos, camera_pos, camera_controller.camera_distance);
  gfc_vector3d_rotate_about_x(&camera_pos, camera_controller.rot->y);
  gfc_vector3d_rotate_about_z(&camera_pos, camera_controller.rot->x);

  gfc_vector3d_add(camera_pos, camera_pos, target_pos);

  gfc_matrix4_view(*view, camera_pos, target_pos, gfc_vector3d(0, 0, 1));

  //  slog("Camera System");
  //  slog("camera position: %f %f %f", camera_pos.x, camera_pos.y,
  //  camera_pos.z); slog("target position: %f %f %f ", target_pos.x,
  //  target_pos.y, target_pos.z); slog("camera rotation: %f %f ",
  //  camera_controller.rot->x, camera_controller.rot->y);
}

void camera_set_target(TransformComponent *transform) {

  if (!transform) {
    camera_controller.target = NULL;
  }

  camera_controller.target = transform;
}

void camera_set_rotation(GFC_Vector2D *rotation) {

  if (!rotation) {
    return;
  }

  camera_controller.rot = rotation;
}

void camera_set_distance(float distance) {

  camera_controller.camera_distance = distance;
}

void camera_clear() {
  memset(&camera_controller, 0, sizeof(camera_controller));
}
