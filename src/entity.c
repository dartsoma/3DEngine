#include "cf_queue.h"
#include "entity_component.h"
#include "entity_core.h"
#include "float.h"
#include "gfc_hashmap.h"
#include "gfc_primitives.h"
#include "gfc_types.h"
#include "gfc_vector.h"
#include "level.h"
#include "simple_logger.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {

  Entity *entity_list;

  GFC_HashMap *entity_hash; // Converts id to entity index in hashmap
  CF_Queue *open_indices;   // Free slots for placing entities

  uint32_t next_id;
  uint32_t entity_max;

  TransformComponent *transform_list;

  KinematicsComponent *kinematics_list;

  ResourceComponent *resource_list;

  CollisionComponent *collision_list;

  RenderComponent *render_list;

} EntityManager;

EntityManager entity_manager = {0};

void entity_manager_init(uint32_t max) {

  entity_manager.entity_list = gfc_allocate_array(sizeof(Entity), max);

  entity_manager.entity_hash = gfc_hashmap_new();
  entity_manager.open_indices =
      cf_queue_init(); // Free slots for placing entities

  entity_manager.next_id = 0;
  entity_manager.entity_max = max;
  entity_manager.transform_list =
      gfc_allocate_array(sizeof(TransformComponent), max);
  entity_manager.kinematics_list =
      gfc_allocate_array(sizeof(KinematicsComponent), max);
  entity_manager.resource_list =
      gfc_allocate_array(sizeof(ResourceComponent), max);
  entity_manager.collision_list =
      gfc_allocate_array(sizeof(CollisionComponent), max);
  entity_manager.render_list = gfc_allocate_array(sizeof(RenderComponent), max);

  atexit(entity_manager_close);
}

void entity_manager_close() {
  int i;

  if (!entity_manager.entity_list)
    return;

  cf_queue_clear(entity_manager.open_indices);

  for (i = 0; i < entity_manager.entity_max; i++) {
    entity_free(&entity_manager.entity_list[i]);
  }

  gfc_hashmap_foreach(entity_manager.entity_hash, free);
  gfc_hashmap_free(entity_manager.entity_hash);
  entity_delete_component_lists(); // ensure all components are reset and
                                   // deleted
  free(entity_manager.entity_list);
}

void entity_free(Entity *entity) {

  if (!entity)
    return;

  if (entity->free)
    entity->free(entity);

  if (entity_manager.open_indices)
    cf_enqueue(entity_manager.open_indices, &entity);

  free(gfc_hashmap_get(entity_manager.entity_hash, entity->id));
  gfc_hashmap_delete_by_key(entity_manager.entity_hash, entity->id);

  memset(entity, 0, sizeof(Entity));
}

Entity *entity_create() {

  int i;
  uint32_t *index;
  uint32_t nextid;

  if (entity_manager.next_id < entity_manager.entity_max) {

    for (i = 0; i < entity_manager.entity_max; i++) {
      if (entity_manager.entity_list[i]._inuse)
        continue;
      entity_manager.entity_list[i]._inuse = 1;

      nextid = entity_manager.next_id;
      entity_manager.next_id++;

      index = (uint32_t *)malloc(sizeof(uint32_t));
      *index = i;
      sprintf(entity_manager.entity_list[i].id, "%ud", nextid);
      gfc_hashmap_insert(entity_manager.entity_hash,
                         entity_manager.entity_list[i].id, index);
      return &entity_manager.entity_list[i];
    }
  }

  Entity *ent;

  ent = (Entity *)cf_dequeue(entity_manager.open_indices);
  if (ent != NULL) {
    ent->_inuse = 1;

    nextid = entity_manager.next_id;
    entity_manager.next_id++;

    index = (uint32_t *)malloc(sizeof(uint32_t));
    *index = (uint32_t)(ent - entity_manager.entity_list);

    sprintf(ent->id, "%ud", nextid);
    gfc_hashmap_insert(entity_manager.entity_hash, ent->id, index);
    return &entity_manager.entity_list[i];
  }

  slog("Not enough space for more entities");
  return NULL;
}

uint8_t entity_is_valid(uint32_t id) {

  if (entity_get_reference(id) != NULL) {
    return 1;
  }

  return 0;
}

// Draw models
void entity_manager_draw() {

  int i;

  for (i = 0; i < entity_manager.entity_max; i++) {
    if (!entity_manager.entity_list[i]._inuse)
      continue;
    if (entity_manager.entity_list[i].draw) {
      entity_manager.entity_list[i].draw(&entity_manager.entity_list[i]);
    }
  }
}

void entity_manager_think() {

  int i;

  for (i = 0; i < entity_manager.entity_max; i++) {
    if (!entity_manager.entity_list[i]._inuse)
      continue;

    if (entity_manager.entity_list[i].think) {
      entity_manager.entity_list[i].think(&entity_manager.entity_list[i]);
    }
  }
}

void entity_manager_update() {

  int i;

  for (i = 0; i < entity_manager.entity_max; i++) {
    if (!entity_manager.entity_list[i]._inuse)
      continue;

    if (entity_manager.entity_list[i].update) {
      entity_manager.entity_list[i].update(&entity_manager.entity_list[i]);
    }
  }
}

void entity_delete_component_lists() {

  if (entity_manager.transform_list) {
    free(entity_manager.transform_list);
  }

  if (entity_manager.kinematics_list) {
    free(entity_manager.kinematics_list);
  }

  if (entity_manager.resource_list) {
    free(entity_manager.resource_list);
  }

  if (!entity_manager.collision_list) {
    free(entity_manager.collision_list);
  }

  if (!entity_manager.render_list) {
    free(entity_manager.render_list);
  }
  return;
}

void entity_remove_component(uint32_t index, EntityComponentType type) {

  switch (type) {
  case EntityTransform:
    memset(&entity_manager.transform_list[index], 0,
           sizeof(entity_manager.transform_list[index]));
    break;
  case EntityKinematics:
    memset(&entity_manager.kinematics_list[index], 0,
           sizeof(entity_manager.kinematics_list[index]));
    break;
  case EntityResource:
    if (entity_manager.resource_list[index].f_data)
      free(entity_manager.resource_list[index].f_data);
    if (entity_manager.resource_list[index].i_data)
      free(entity_manager.resource_list[index].i_data);
    if (entity_manager.resource_list[index].a_data)
      free(entity_manager.resource_list[index].a_data);
    memset(&entity_manager.resource_list[index], 0,
           sizeof(entity_manager.resource_list[index]));
    break;
  case EntityCollision:
    free(entity_manager.collision_list[index].linetrace);
    memset(&entity_manager.collision_list[index], 0,
           sizeof(entity_manager.collision_list[index]));
    break;
  case EntityRender:
    model_free(entity_manager.render_list[index].model);
    memset(&entity_manager.render_list[index], 0,
           sizeof(entity_manager.render_list[index]));
    break;
  }
}

Entity *entity_get_reference(uint32_t id) {

  uint32_t index;
  index = entity_get_index(id);

  if (index == 0 && id != 0)
    return NULL;
  if (entity_manager.entity_list[index]._inuse == 0)
    return NULL;

  return &entity_manager.entity_list[index];
}

uint32_t entity_get_index(uint32_t id) {
  char str[11];
  uint32_t *index;
  sprintf(str, "%ud", id);

  if (gfc_hashmap_get(entity_manager.entity_hash, str) == NULL)
    return 0;

  index = (uint32_t *)gfc_hashmap_get(entity_manager.entity_hash, str);

  return *index;
}

void entity_remove_all_components(uint32_t id) {
  int i;
  Entity *ent;

  ent = entity_get_reference(id);

  if (!ent)
    return;

  char *end_ptr;
  uint32_t new_id = id = strtoul(ent->id, &end_ptr, 10);

  for (i = 0; i < ALLCOMPONENTS; i++) {
    if (ent->tag == 0)
      continue;

    if ((ent->tag >> i) & 1) {
      entity_remove_component(new_id, (1 << i));
    }
  }
}

TransformComponent *entity_get_transform(uint32_t id) {

  uint32_t index;
  Entity *ent;

  ent = entity_get_reference(id);

  if (!ent)
    return NULL;
  index = entity_get_index(id);

  if ((ent->tag & EntityTransform) != 0) {
    return &entity_manager.transform_list[index];
  }

  // slog("Entity %d has no Transform tag", id);
  return NULL;
}

KinematicsComponent *entity_get_kinematics(uint32_t id) {
  uint32_t index;
  Entity *ent;

  ent = entity_get_reference(id);

  if (!ent)
    return NULL;
  index = entity_get_index(id);

  if ((ent->tag & EntityKinematics) != 0) {
    return &entity_manager.kinematics_list[index];
  }

  // slog("Entity %d has no Kinematics tag", id);
  return NULL;
}

ResourceComponent *entity_get_resource(uint32_t id) {
  uint32_t index;
  Entity *ent;

  ent = entity_get_reference(id);

  if (!ent)
    return NULL;
  index = entity_get_index(id);

  if ((ent->tag & EntityResource) != 0) {
    return &entity_manager.resource_list[index];
  }

  // slog("Entity %d has no Resource tag", id);
  return NULL;
}

CollisionComponent *entity_get_collision(uint32_t id) {
  uint32_t index;
  Entity *ent;

  ent = entity_get_reference(id);

  if (!ent)
    return NULL;
  index = entity_get_index(id);

  if ((ent->tag & EntityCollision) != 0) {
    return &entity_manager.collision_list[index];
  }

  // slog("Entity %d has no Collision tag", id);
  return NULL;
}

RenderComponent *entity_get_render(uint32_t id) {
  uint32_t index;
  Entity *ent;

  ent = entity_get_reference(id);

  if (!ent)
    return NULL;
  index = entity_get_index(id);

  if ((ent->tag & EntityRender) != 0) {
    return &entity_manager.render_list[index];
  }

  // slog("Entity %d has no Render tag", id);
  return NULL;
}

TransformComponent *entity_get_own_transform(Entity *self) {
  if (!self)
    return NULL;
  char *endptr;
  uint32_t id = strtoul(self->id, &endptr, 10);
  return entity_get_transform(id);
}
KinematicsComponent *entity_get_own_kinematics(Entity *self) {
  if (!self)
    return NULL;
  char *endptr;
  uint32_t id = strtoul(self->id, &endptr, 10);
  return entity_get_kinematics(id);
}
ResourceComponent *entity_get_own_resource(Entity *self) {
  if (!self)
    return NULL;
  char *endptr;
  uint32_t id = strtoul(self->id, &endptr, 10);
  return entity_get_resource(id);
}
CollisionComponent *entity_get_own_collisions(Entity *self) {
  if (!self)
    return NULL;
  char *endptr;
  uint32_t id = strtoul(self->id, &endptr, 10);
  return entity_get_collision(id);
}
RenderComponent *entity_get_own_render(Entity *self) {
  if (!self)
    return NULL;
  char *endptr;
  uint32_t id = strtoul(self->id, &endptr, 10);
  return entity_get_render(id);
}

void kinematics_accelerate(KinematicsComponent *kinematics,
                           GFC_Vector2D *axis) {

  if (!kinematics)
    return;

  float max_accel = kinematics->acc_max;

  if (axis->x != 0 && kinematics->acc.x == 0) {
    kinematics->acc.x = axis->x;
  } else if (axis->x != 0) {
    kinematics->acc.x += axis->x * max_accel * 0.4 * 0.016;
  } else {
    kinematics->acc.x = kinematics->acc.x / DAMPENER;
    if (fabsf(kinematics->acc.x) <= GFC_EPSILON) {
      kinematics->acc.x = 0;
    }
  }

  if (axis->y != 0 && kinematics->acc.z == 0) {
    kinematics->acc.z = axis->y;
  } else if (axis->y != 0) {
    kinematics->acc.z += axis->y * max_accel * 0.4 * 0.016;
  } else {
    kinematics->acc.z = kinematics->acc.z / DAMPENER;
    if (fabsf(kinematics->acc.z) <= GFC_EPSILON) {
      kinematics->acc.z = 0;
    }
  }

  GFC_Vector2D new_accel = gfc_vector2d(kinematics->acc.x, kinematics->acc.z);

  float magnitude = gfc_vector2d_magnitude(new_accel);
  if (magnitude > max_accel) {
    float correction_scale = max_accel / magnitude;
    new_accel.x *= correction_scale;
    new_accel.y *= correction_scale;
  }
  kinematics->acc.x = new_accel.x;
  kinematics->acc.z = new_accel.y;
}

void kinematics_velocity(KinematicsComponent *kinematics, GFC_Vector2D *axis) {

  if (!kinematics)
    return;

  float max_vel = kinematics->vel_max;

  if (kinematics->acc.x != 0) {
    kinematics->vel.x += kinematics->acc.x * 0.016;
  } else {
    kinematics->vel.x = kinematics->vel.x / DAMPENER;
    if (fabsf(kinematics->vel.x) <= GFC_EPSILON) {
      kinematics->vel.x = 0;
    }
  }

  if (kinematics->acc.z != 0) {
    kinematics->vel.z += kinematics->acc.z * 0.016;
  } else {
    kinematics->vel.z = kinematics->vel.z / DAMPENER;
    if (fabsf(kinematics->vel.z) <= GFC_EPSILON) {
      kinematics->vel.z = 0;
    }
  }

  GFC_Vector2D new_vel = gfc_vector2d(kinematics->vel.x, kinematics->vel.z);

  float magnitude = gfc_vector2d_magnitude(new_vel);
  if (magnitude > max_vel) {
    float correction_scale = max_vel / magnitude;
    new_vel.x *= correction_scale;
    new_vel.y *= correction_scale;
  }

  kinematics->vel.x = new_vel.x;
  kinematics->vel.z = new_vel.y;
  // slog("x velocity: %f", kinematics->vel.x);
  // slog("z velocity: %f", kinematics->vel.z);
}

GFC_Box *render_get_bounds(Entity *self) {

  if (!self)
    return NULL;

  RenderComponent *render;

  render = entity_get_own_render(self);

  if (!render)
    return NULL;

  if (!render->model)
    return NULL;

  return &render->model->mesh->bounds;
}

GFC_Vector3D *transform_get_position(Entity *self) {

  if (!self)
    return NULL;

  TransformComponent *transform;

  transform = entity_get_own_transform(self);

  if (transform)
    return NULL;

  return &transform->pos;
}

GFC_Vector3D *transform_get_rotation(Entity *self) {

  if (!self)
    return NULL;

  TransformComponent *transform;

  transform = entity_get_own_transform(self);

  if (transform)
    return NULL;

  return &transform->rot;
}

GFC_Vector3D *transform_get_scale(Entity *self) {

  if (!self)
    return NULL;

  TransformComponent *transform;

  transform = entity_get_own_transform(self);

  if (transform)
    return NULL;

  return &transform->scale;
}

void *entity_transform_copy(Entity *self, Transform *transform) {

  if (!self)
    return NULL;

  TransformComponent *component;

  component = entity_get_own_transform(self);

  if (component)
    return NULL;

  transform->pos = &component->pos;
  transform->rot = &component->rot;
  transform->scale = &component->scale;

  return transform;
}

void entity_world_collision(Entity *self) {

  if (!self)
    return;

  int i;
  uint32_t c;
  StaticModel *static_model;
  GFC_Box *entity_box;
  CapsuleObject *entity_capsule;
  CollisionComponent *collision;
  Model *entity_model;
  CollisionInfo *collision_info;
  Transform *entity_transform, *static_transform;

  if (level_get_current_index() == level_get_count()) {
    slog("No current level");
    return;
  }

  c = level_get_static_count();
  collision = entity_get_own_collision(self);

  if (collision->_inuse == 0) {
    return;
  }
  entity_transform_copy(self, entity_transform);
  entity_model = render_get_model(self);
  entity_box = render_get_bounds(self);

  if (collision->capsule) {
    entity_capsule = collision->capsule;
  }

  for (i = 0; i < c; i++) {
    static_model = level_get_static(i);

    static_transform->pos = &static_model->pos;
    static_transform->rot = &static_model->rot;
    static_transform->scale = &static_model->scale;
    if (!static_model->model) {
      slog("no static model");
      continue;
    }

    if ((collision->layer & static_model->collision_mask) > 0)
      continue;

    // check first bounds first
    //

    if (!entity_box) {
      slog("entity_world_collision: No Entity Bounds Found!");
      return;
    }

    if (collision_box_box(*entity_box, *entity_transform,
                          static_model->model->mesh->bounds, *static_transform,
                          collision_info) == 1) {

      switch (collision->type) {

      case 0:

        switch (static_model->collision_mask) {
        case 0:
          collision_resolve(entity_transform, collision, collision_info);
          break;
        case 1:
          if (collision_box_complex(*entity_box, *entity_transform,
                                    static_model->model, *static_transform,
                                    collision_info) == 1) {
            collision_resolve(entity_transform, collision, collision_info);
          }
          break;
        }
        break;
      case 1:
        switch (static_model->collision_mask) {
        case 0:
          if (collision_box_complex(static_model->model->mesh->bounds,
                                    *static_transform, entity_model,
                                    *entity_transform, collision_info) == 1) {
            collision_resolve(entity_transform, collision, collision_info);
          }
          break;
        case 1:
          if (collision_complex_complex(entity_model, *entity_transform,
                                        static_model->model, *static_transform,
                                        collision_info) == 1) {
            collision_resolve(entity_transform, collision, collision_info);
          }
          break;
        }
        break;
      case 2:
        switch (static_model->collision_mask) {
        case 0:
          if (collision_box_capsule(static_model->model->mesh->bounds,
                                    *static_transform, entity_capsule,
                                    *entity_transform, collision_info) == 1) {
            collision_resolve(entity_transform, collision, collision_info);
          }

          break;
        case 1:
          if (collision_capsule_complex(entity_capsule, *entity_transform,
                                        static_model->model, *static_transform,
                                        collision_info) == 1) {
            collision_resolve(entity_transform, collision, collision_info);
          }
          break;
        }
        break;
      }
    }
  }
}

GFC_Vector3D collision_transform_vertex(GFC_Vector3D vertex,
                                        Transform *transform) {
  if (!transform)
    return vertex;

  float x = vertex.x * (*transform->scale.x);
  float y = vertex.y * (*transform->scale.y);
  float z = vertex.z * (*transform->scale.z);

  float rx = *transform->rotation.x * GFC_DEGTORAD;
  float ry = *transform->rotation.y * GFC_DEGTORAD;
  float rz = *transform->rotation.z * GFC_DEGTORAD;

  float cx = cosf(rx);
  float sx = sinf(rx);

  float cy = cosf(ry);
  float sy = sinf(ry);

  float cz = cosf(rz);
  float sz = sinf(rz);

  float y1 = y * cx - z * sx;
  float z1 = y * sx + z * cx;

  y = y1;
  z = z1;

  float x2 = x * cy + z * sy;
  float z2 = -x * sy + z * cy;

  x = x2;
  z = z2;

  float x3 = x * cz - y * sz;
  float y3 = x * sz + y * cz;

  x = x3;
  y = y3;

  GFC_Vector3D result = {x + transform->pos->x, y + transform->pos->y,
                         z + transform->pos->z};

  return result;
}

void collision_obb_construct(GFC_Box *box, Transform *transform, OBB *obb) {

  gfc_vector3d_copy(obb->center, *transform->pos);

  obb->half_lengths = (GFC_Vector3D){box->width * (*transform->scale.x) * 0.5,
                                     box->height * (*transform->scale.y) * 0.5,
                                     box->depth * (*transform->scale.z) * 0.5};

  GFC_Vector3D rotation = {(*transform->rotation.x) * DEG_TO_RAD,
                           (*transform->rotation.y) * DEG_TO_RAD,
                           (*transform->rotation.z) * DEG_TO_RAD};

  float x = rotation.x * GFC_DEGTORAD;
  float y = rotation.y * GFC_DEGTORAD;
  float z = rotation.z * GFC_DEGTORAD;

  float cx = cosf(x);
  float sx = sinf(x);
  float cy = cosf(y);
  float sy = sinf(y);
  float cz = cosf(z);
  float sz = sinf(z);

  obb->axis[0] = (GFC_Vector3D){cy * cz, cy * sz, -sy};

  obb->axis[1] =
      (GFC_Vector3D){sx * sy * cz - cx * sz, sx * sy * sz + cx * cz, sx * cy};

  obb->axis[2] =
      (GFC_Vector3D){cx * sy * cz + sx * sz, cx * sy * sz - sx * cz, cx * cy};
}

uint8_t collision_box_box(GFC_Box box1, Transform transform1, GFC_Box box2,
                          Transform transform2, CollisionInfo *info) {

  OBB *obb1, *obb2;
  collision_obb_construct(&box1, &transform1, obb1);
  collision_obb_construct(&box2, &transform2, obb2);

  GFC_Vector3D delta = {obb2->center.x - obb1->center.x,
                        obb2->center.y - obb1->center.y,
                        obb2->center.z - obb1->center.z};

  GFC_Vector3D axes[15];

  // Three axes from each OBB.
  for (int i = 0; i < 3; i++) {
    axes[i] = obb1->axis[i];
    axes[i + 3] = obb1->axis[i];
  }

  // Nine cross-product axes.
  int index = 6;

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      gfc_vector3d_cross_product(&axes[index++], obb1->axis[i], obb2->axis[j]);
    }
  }

  float min_depth = 1.0e30f;
  GFC_Vector3D collision_normal = {0, 0, 0};

  // test all seperating axis
  for (int i = 0; i < 15; i++) {
    GFC_Vector3D axis = axes[i];

    float length = gfc_vector3d_magnitude(axis);

    // Skip parallel axis
    if (length < 0.00001f)
      continue;

    axis.x /= length;
    axis.y /= length;
    axis.z /= length;

    //  Project both boxes onto the axis
    float r1 = obb1->half_lengths.x *
                   fabsf(gfc_vector3d_dot_product(obb1->axis[0], axis)) +
               obb1->half_lengths.y *
                   fabsf(gfc_vector3d_dot_product(obb1->axis[1], axis)) +
               obb1->half_lengths.z *
                   fabsf(gfc_vector3d_dot_product(obb1->axis[2], axis));

    float r2 = obb2->half_lengths.x *
                   fabsf(gfc_vector3d_dot_product(obb2->axis[0], axis)) +
               obb2->half_lengths.y *
                   fabsf(gfc_vector3d_dot_product(obb2->axis[1], axis)) +
               obb2->half_lengths.z *
                   fabsf(gfc_vector3d_dot_product(obb2->axis[2], axis));

    float distance = fabsf(gfc_vector3d_dot_product(delta, axis));

    float overlap = r1 + r2 - distance;

    // can never meet
    if (overlap < -0.0001f) {
      if (info)
        info->collided = 0;

      return 0;
    }

    // smallest pen
    // pushed based on that
    if (overlap < min_depth) {
      min_depth = overlap;

      // normal points from box1 toward box2.
      if (gfc_vector3d_dot_product(delta, axis) < 0) {
        axis.x = -axis.x;
        axis.y = -axis.y;
        axis.z = -axis.z;
      }

      collision_normal = axis;
    }
  }

  // Collision confirmed.
  if (info) {
    info->collided = 1;
    info->normal = collision_normal;
    info->depth = min_depth;
  }

  return 1;
}

uint8_t collision_box_complex(GFC_Box box1, Transform transform1, Model *model,
                              Transform transform2, CollisionInfo *info) {}
uint8_t collision_box_capsule(GFC_Box box, Transform transform1,
                              CapsuleObject *capsule, Transform transform2,
                              CollisionInfo *info) {}
uint8_t collision_capsule_capsule(CapsuleObject *capsule1, Transform transform1,
                                  CapsuleObject *capsule2, Transform transform2,
                                  CollisionInfo *info) {}
uint8_t collision_capsule_complex(CapsuleObject *capsule, Transform transform1,
                                  Model *model, Transform transform2,
                                  CollisionInfo *info) {}
uint8_t collision_complex_complex(Model *model1, Transform transform1,
                                  Model *model2, Transform transform2,
                                  CollisionInfo *info) {

  CollisionInfo temp;
  temp = *info;

  if (!model1 || !model2)
    return 0;

  Mesh *mesh1 = model1->mesh;
  Mesh *mesh2 = model2->mesh;

  if (!mesh1 || !mesh2)
    return 0;
  int p1, p2, i, j;

  int count1 = gfc_list_count(mesh1->primitives);
  int count2 = gfc_list_count(mesh2->primitives);

  for (p1 = 0; p1 < count1; p1++) {
    MeshPrimitive *prim1 = gfc_list_nth(mesh1->primitives, p1);

    if (!prim1 || !prim1->objData)
      continue;

    ObjData *data1 = prim1->objData;

    for (p2 = 0; p2 < count2; p2++) {
      MeshPrimitive *prim2 = gfc_list_nth(mesh2->primitives, p2);

      if (!prim2 || !prim2->objData)
        continue;

      ObjData *data2 = prim2->objData;

      uint32_t triangles1 = data1->face_vert_count / 3;
      uint32_t triangles2 = data2->face_vert_count / 3;

      for (i = 0; i < triangles1; i++) {
        Vertex *v1 = &data1->faceVertices[i * 3];

        GFC_Vector3D a1 = collision_transform_vertex(v1[0].vertex, &transform1);

        GFC_Vector3D a2 = collision_transform_vertex(v1[1].vertex, &transform1);

        GFC_Vector3D a3 = collision_transform_vertex(v1[2].vertex, &transform1);

        for (j = 0; j < triangles2; j++) {
          Vertex *v2 = &data2->faceVertices[j * 3];

          GFC_Vector3D b1 =
              collision_transform_vertex(v2[0].vertex, &transform2);

          GFC_Vector3D b2 =
              collision_transform_vertex(v2[1].vertex, &transform2);

          GFC_Vector3D b3 =
              collision_transform_vertex(v2[2].vertex, &transform2);

          if (collision_triangle_triangle(a1, a2, a3, b1, b2, b3, &temp)) {
            if (info)
              *info = temp;
            return 1;
          }
        }
      }
    }
  }

  if (info)
    info->collided = 0;

  return 0;
}
