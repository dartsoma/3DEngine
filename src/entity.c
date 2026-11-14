#include "cf_queue.h"
#include "entity_component.h"
#include "entity_core.h"
#include "float.h"
#include "gf3d_mesh.h"
#include "gf3d_obj_load.h"
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
    if (entity_manager.resource_list[index].v3_data)
      free(entity_manager.resource_list[index].v3_data);
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
CollisionComponent *entity_get_own_collision(Entity *self) {
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

void kinematics_accelerate(KinematicsComponent *kinematics) {

  if (!kinematics)
    return;

  // kinematics_fall(kinematics);
  kinematics->acc.x = kinematics->direction.x * kinematics->acc_max;
  kinematics->acc.y = kinematics->direction.y * kinematics->acc_max;
}

void kinematics_fall(KinematicsComponent *kinematics) {

  if (!kinematics)
    return;

  kinematics->acc.z = -9.8;
}

void kinematics_velocity(KinematicsComponent *kinematics) {

  if (!kinematics)
    return;

  if (gfc_vector3d_magnitude(kinematics->acc) <= GFC_EPSILON) {
    if (gfc_vector3d_magnitude(kinematics->vel) <= GFC_EPSILON) {
      kinematics->vel = gfc_vector3d(0, 0, 0);
      if (gfc_vector3d_magnitude(kinematics->vel) == 0)
        return;
    } else if (gfc_vector3d_magnitude(kinematics->vel) != 0) {

      gfc_vector3d_scale(kinematics->vel, kinematics->vel, 0.75);
    }
  }

  GFC_Vector3D acceleration;

  acceleration = kinematics->acc;

  gfc_vector3d_scale(acceleration, kinematics->acc, 0.016);
  gfc_vector3d_add(kinematics->vel, acceleration, kinematics->vel);

  float magnitude;
  magnitude = gfc_vector3d_magnitude(kinematics->vel);
  if (magnitude > kinematics->vel_max) {
    kinematics->vel.x = kinematics->vel.x * (kinematics->vel_max / magnitude);
    kinematics->vel.y = kinematics->vel.y * (kinematics->vel_max / magnitude);
  }
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

Model *render_get_model(Entity *self) {

  if (!self)
    return NULL;

  RenderComponent *render;

  render = entity_get_own_render(self);

  if (!render)
    return NULL;

  if (!render->model)
    return NULL;

  return render->model;
}

GFC_Vector3D transform_get_position(Entity *self) {

  if (!self)
    return gfc_vector3d(0, 0, 0);

  TransformComponent *transform;

  transform = entity_get_own_transform(self);

  return transform->pos;
}

GFC_Vector3D transform_get_rotation(Entity *self) {

  if (!self)
    return gfc_vector3d(0, 0, 0);

  TransformComponent *transform;

  transform = entity_get_own_transform(self);

  return transform->rot;
}

GFC_Vector3D transform_get_scale(Entity *self) {

  if (!self)
    return gfc_vector3d(0, 0, 0);

  TransformComponent *transform;

  transform = entity_get_own_transform(self);

  return transform->scale;
}

void *entity_transform_get(Entity *self, Transform *transform) {

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
  CollisionInfo *collision_info = {0};
  Transform *entity_transform = {0}, *static_transform = {0};

  if (level_get_current_index() == level_get_count()) {
    slog("No current level");
    return;
  }

  c = level_get_static_count();
  collision = entity_get_own_collision(self);

  if (collision->_inuse == 0) {
    return;
  }
  entity_transform_get(self, entity_transform);
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

  float x = vertex.x * transform->scale->x;
  float y = vertex.y * transform->scale->y;
  float z = vertex.z * transform->scale->z;

  float rx = transform->rot->x * GFC_DEGTORAD;
  float ry = transform->rot->y * GFC_DEGTORAD;
  float rz = transform->rot->z * GFC_DEGTORAD;

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

  gfc_vector3d_copy(obb->center, (*transform->pos));

  obb->half_lengths = (GFC_Vector3D){box->w * transform->scale->x * 0.5,
                                     box->h * transform->scale->y * 0.5,
                                     box->d * transform->scale->z * 0.5};

  GFC_Vector3D rotation = {transform->rot->x * GFC_DEGTORAD,
                           transform->rot->y * GFC_DEGTORAD,
                           transform->rot->z * GFC_DEGTORAD};

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

  OBB *obb1 = {0}, *obb2 = {0};
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
    info->penetration_depth = min_depth;
  }

  return 1;
}

uint8_t collision_box_complex(GFC_Box box1, Transform transform1, Model *model,
                              Transform transform2, CollisionInfo *info) {
  return 0;
}

// most important
uint8_t collision_box_capsule(GFC_Box box, Transform transform1,
                              CapsuleObject *capsule, Transform transform2,
                              CollisionInfo *info) {
  return 0;
}
uint8_t collision_capsule_capsule(CapsuleObject *capsule1, Transform transform1,
                                  CapsuleObject *capsule2, Transform transform2,
                                  CollisionInfo *info) {
  return 0;
}

uint8_t collision_capsule_complex(CapsuleObject *capsule, Transform transform1,
                                  Model *model, Transform transform2,
                                  CollisionInfo *info) {
  return 0;
}

void collision_project_triangle(GFC_Vector3D a, GFC_Vector3D b, GFC_Vector3D c,
                                GFC_Vector3D axis, float *min, float *max) {
  float p1 = gfc_vector3d_dot_product(a, axis);
  float p2 = gfc_vector3d_dot_product(b, axis);
  float p3 = gfc_vector3d_dot_product(c, axis);

  *min = fminf(p1, fminf(p2, p3));
  *max = fmaxf(p1, fmaxf(p2, p3));
}

uint8_t collision_triangle_triangle(GFC_Vector3D a1, GFC_Vector3D a2,
                                    GFC_Vector3D a3,

                                    GFC_Vector3D b1, GFC_Vector3D b2,
                                    GFC_Vector3D b3,

                                    CollisionInfo *info) {
  if (info)
    *info = (CollisionInfo){0};

  GFC_Triangle3D triA = gfc_triangle(a1, a2, a3);
  GFC_Triangle3D triB = gfc_triangle(b1, b2, b3);

  GFC_Vector3D A[3] = {a1, a2, a3};
  GFC_Vector3D B[3] = {b1, b2, b3};

  GFC_Vector3D edgesA[3], edgesB[3];
  gfc_vector3d_sub(edgesA[0], a2, a1);
  gfc_vector3d_sub(edgesA[1], a3, a2);
  gfc_vector3d_sub(edgesA[2], a1, a3);

  gfc_vector3d_sub(edgesB[0], b2, b1);
  gfc_vector3d_sub(edgesB[1], b3, b2);
  gfc_vector3d_sub(edgesB[2], b1, b3);

  // face normals
  GFC_Vector3D normalA = gfc_triangle_get_normal(triA);
  GFC_Vector3D normalB = gfc_triangle_get_normal(triB);

  // scrap bad triangles
  if (gfc_vector3d_magnitude(normalA) <= GFC_EPSILON ||
      gfc_vector3d_magnitude(normalB) <= GFC_EPSILON)
    return 0;

  // unit edges so cross product length = sine of the angle (parallel test)
  for (int i = 0; i < 3; i++) {
    gfc_vector3d_normalize(&edgesA[i]);
    gfc_vector3d_normalize(&edgesB[i]);
  }

  GFC_Vector3D axes[17];
  int axisCount = 0;

  // face normals first so they win ties
  axes[axisCount++] = normalA;
  axes[axisCount++] = normalB;

  // in-plane edge normals (needed for coplanar triangles)
  for (int i = 0; i < 3; i++) {
    gfc_vector3d_cross_product(&axes[axisCount++], normalA, edgesA[i]);
    gfc_vector3d_cross_product(&axes[axisCount++], normalB, edgesB[i]);
  }

  // edge x edge
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      gfc_vector3d_cross_product(&axes[axisCount++], edgesA[i], edgesB[j]);

  float bestDepth = FLT_MAX;
  GFC_Vector3D bestNormal = {0, 0, 0};

  for (int i = 0; i < axisCount; i++) {
    GFC_Vector3D axis = axes[i];
    float axisLength = gfc_vector3d_magnitude(axis);

    // ignore parallel
    // id have used epsilon but thats too precise
    if (axisLength <= 1e-3f)
      continue;

    gfc_vector3d_scale(axis, axis, 1.0f / axisLength);

    float minA, maxA, minB, maxB;
    collision_project_triangle(a1, a2, a3, axis, &minA, &maxA);
    collision_project_triangle(b1, b2, b3, axis, &minB, &maxB);

    // separating axis found
    if (maxA < minB - GFC_EPSILON || maxB < minA - GFC_EPSILON)
      return 0;

    float dPos = maxA - minB;
    float dNeg = maxB - minA;

    float depth;
    GFC_Vector3D n;
    if (dPos < dNeg) {
      depth = dPos;
      n = axis;
    } else {
      depth = dNeg;
      gfc_vector3d_scale(n, axis, -1.0f);
    }
    if (depth < 0.0f)
      depth = 0.0f;

    // edge threshold
    float threshold = (i < 8) ? bestDepth : bestDepth * 0.95f;
    if (depth < threshold) {
      bestDepth = depth;
      bestNormal = n;
    }
  }

  if (bestDepth == FLT_MAX)
    return 0;

  if (info) {
    // contact point: average of where each triangle's edges pierce the other
    GFC_Vector3D sum = {0, 0, 0};
    GFC_Vector3D hit;
    int hits = 0;

    for (int i = 0; i < 3; i++) {
      if (gfc_triangle_edge_test(gfc_edge3d_from_vectors(A[i], A[(i + 1) % 3]),
                                 triB, &hit)) {
        gfc_vector3d_add(sum, sum, hit);
        hits++;
      }
      if (gfc_triangle_edge_test(gfc_edge3d_from_vectors(B[i], B[(i + 1) % 3]),
                                 triA, &hit)) {
        gfc_vector3d_add(sum, sum, hit);
        hits++;
      }
    }

    // coplanar overlap: edges never cross the plane, use contained vertices
    if (hits == 0) {
      for (int i = 0; i < 3; i++) {
        if (gfc_point_in_triangle(A[i], triB)) {
          gfc_vector3d_add(sum, sum, A[i]);
          hits++;
        }
        if (gfc_point_in_triangle(B[i], triA)) {
          gfc_vector3d_add(sum, sum, B[i]);
          hits++;
        }
      }
    }

    // last resort: midpoint of the two triangle centers
    if (hits == 0) {
      for (int i = 0; i < 3; i++) {
        gfc_vector3d_add(sum, sum, A[i]);
        gfc_vector3d_add(sum, sum, B[i]);
      }
      hits = 6;
    }

    info->collided = 1;
    info->normal = bestNormal;
    info->penetration_depth = bestDepth;
    gfc_vector3d_scale(info->contact_point, sum, 1.0f / (float)hits);
  }

  return 1;
}
// MESHES COLLIDING WITH MESHES (DO NOT USE OFTEN) if ever
// this is just an edge case
uint8_t collision_complex_complex(Model *model1, Transform transform1,
                                  Model *model2, Transform transform2,
                                  CollisionInfo *info) {

  if (!model1 || !model2 || !info)
    return 0;

  Mesh *mesh1 = model1->mesh;
  Mesh *mesh2 = model2->mesh;

  if (!mesh1 || !mesh2)
    return 0;

  CollisionInfo *hits = NULL;
  int hitCount = 0, hitCap = 0;

  int count1 = gfc_list_count(mesh1->primitives);
  int count2 = gfc_list_count(mesh2->primitives);

  for (int p2 = 0; p2 < count2; p2++) {

    MeshPrimitive *prim2 = gfc_list_nth(mesh2->primitives, p2);
    if (!prim2 || !prim2->objData)
      continue;

    ObjData *data2 = prim2->objData;
    uint32_t triangles2 = data2->face_vert_count / 3;
    if (triangles2 == 0)
      continue;

    // transform B once per primitive instead of once per A triangle
    GFC_Vector3D *wB = malloc(sizeof(GFC_Vector3D) * triangles2 * 3);
    if (!wB)
      continue;

    for (uint32_t j = 0; j < triangles2 * 3; j++)
      wB[j] = collision_transform_vertex(data2->faceVertices[j].vertex,
                                         &transform2);

    for (int p1 = 0; p1 < count1; p1++) {

      MeshPrimitive *prim1 = gfc_list_nth(mesh1->primitives, p1);
      if (!prim1 || !prim1->objData)
        continue;

      ObjData *data1 = prim1->objData;
      uint32_t triangles1 = data1->face_vert_count / 3;

      for (uint32_t i = 0; i < triangles1; i++) {

        Vertex *v1 = &data1->faceVertices[i * 3];
        GFC_Vector3D a1 = collision_transform_vertex(v1[0].vertex, &transform1);
        GFC_Vector3D a2 = collision_transform_vertex(v1[1].vertex, &transform1);
        GFC_Vector3D a3 = collision_transform_vertex(v1[2].vertex, &transform1);

        GFC_Box boxA = gfc_triangle_get_bounding_box(gfc_triangle(a1, a2, a3));

        for (uint32_t j = 0; j < triangles2; j++) {

          GFC_Vector3D b1 = wB[j * 3];
          GFC_Vector3D b2 = wB[j * 3 + 1];
          GFC_Vector3D b3 = wB[j * 3 + 2];

          // cheap broad reject before the SAT
          if (!gfc_box_overlap(boxA, gfc_triangle_get_bounding_box(
                                         gfc_triangle(b1, b2, b3))))
            continue;

          CollisionInfo candidate = {0};

          if (collision_triangle_triangle(a1, a2, a3, b1, b2, b3, &candidate)) {
            if (hitCount == hitCap) {
              int newCap = hitCap ? hitCap * 2 : 32;
              CollisionInfo *grown =
                  realloc(hits, sizeof(CollisionInfo) * newCap);
              if (!grown)
                continue;
              hits = grown;
              hitCap = newCap;
            }
            hits[hitCount++] = candidate;
          }
        }
      }
    }

    free(wB);
  }

  if (hitCount == 0) {
    free(hits);
    *info = (CollisionInfo){0};
    return 0;
  }

  int deepest = 0;
  for (int i = 1; i < hitCount; i++)
    if (hits[i].penetration_depth > hits[deepest].penetration_depth)
      deepest = i;

  GFC_Vector3D refNormal = hits[deepest].normal;

  GFC_Vector3D sumNormal = {0, 0, 0};
  GFC_Vector3D sumContact = {0, 0, 0};
  GFC_Vector3D t;
  float sumWeight = 0.0f;

  for (int i = 0; i < hitCount; i++) {
    if (gfc_vector3d_dot_product(hits[i].normal, refNormal) <
        0.2f) // normals align with deepest
      continue;

    float w = hits[i].penetration_depth + 1e-4f; // zero-depth still counts

    gfc_vector3d_scale(t, hits[i].normal, w);
    gfc_vector3d_add(sumNormal, sumNormal, t);

    gfc_vector3d_scale(t, hits[i].contact_point, w);
    gfc_vector3d_add(sumContact, sumContact, t);

    sumWeight += w;
  }

  CollisionInfo best = {0};
  best.collided = 1;
  best.penetration_depth = hits[deepest].penetration_depth;

  if (gfc_vector3d_magnitude(sumNormal) > GFC_EPSILON) {
    best.normal = sumNormal;
    gfc_vector3d_normalize(&best.normal);
  } else {
    best.normal = refNormal;
  }

  gfc_vector3d_scale(best.contact_point, sumContact, 1.0f / sumWeight);

  free(hits);
  *info = best;
  return 1;
}

void collision_resolve(Transform *transform, CollisionComponent *collision,
                       const CollisionInfo *info) {

  if (!info)
    return;

  float push = fmaxf(info->penetration_depth - 0.005f, 0.0f) / 0.8f;

  if (push <= 0.0f)
    return;

  GFC_Vector3D move;

  GFC_Vector3D p = *transform->pos;
  gfc_vector3d_scale(move, info->normal, -push);
  gfc_vector3d_add(p, p, move);
  *transform->pos = p;
}
