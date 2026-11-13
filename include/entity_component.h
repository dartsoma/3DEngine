#ifndef __ENTITY_COMPONENT_H__
#define __ENTITY_COMPONENT_H__

#include "entity_core.h"
#include "gfc_color.h"
#include "gfc_matrix.h"
#include "gfc_primitives.h"
#include "gfc_vector.h"
#include <stdint.h>

#define ALLCOMPONENTS 5
#define DAMPENER 80.0f
#define DELTATIME 0.016f
// Added list of methods for specific components
typedef enum {
  EntityTransform = 1,  //
  EntityKinematics = 2, // Manages physics like direction and acceleration
  EntityResource = 4,   // Manages health, ammo etc etc
  EntityCollision = 8,  // Raycasting hitboxes
  EntityRender = 16,    //
} EntityComponentType;

typedef struct {
  uint8_t _inuse;
  GFC_Vector3D pos;
  GFC_Vector3D rot;
  GFC_Vector3D scale;

} TransformComponent;

typedef struct {
  uint8_t _inuse;
  GFC_Vector3D vel;
  GFC_Vector3D acc;
  float vel_max;
  float acc_max;
  GFC_Vector2D direction;
  GFC_Vector3D gravity;

} KinematicsComponent;

typedef struct {
  // function varies
  uint8_t _inuse;
  uint8_t _visible;

  Model *model;

  GFC_Matrix4 mat;
  GFC_Color color; // color mod

} RenderComponent;

typedef struct {

  float max;
  float min;
  float value;

} EntityResourceF;

typedef struct {

  float max;
  float min;
  float value;

} EntityResourceI;

typedef struct {

  float max;
  float min;
  GFC_Vector2D axis;

} EntityResourceA;

typedef struct {
  // Float / integer / axis
  EntityResourceF *f_data;
  EntityResourceI *i_data;
  EntityResourceA *a_data;
  GFC_Vector3D *v3_data;
  GFC_Vector3D *v2_data;

} ResourceComponent;

typedef struct {

  GFC_Vector3D start;
  GFC_Vector3D dir; // normalized direction

} RaycasterObject;

typedef struct {

  GFC_Vector3D offset;
  float radius;
  float height;

} CapsuleObject;

typedef struct {

  GFC_Vector3D *pos;
  GFC_Vector3D *rot;
  GFC_Vector3D *scale;

} Transform;

typedef struct {
  GFC_Vector3D normal;
  GFC_Vector3D contact_point;

  float penetration_depth;

  uint8_t collided;
} CollisionInfo;

// object oriented bounding box
typedef struct {

  GFC_Vector3D axis[3];
  GFC_Vector3D center;
  GFC_Vector3D half_lengths;

} OBB;

typedef struct {
  uint8_t _inuse;
  uint8_t type;   // 0- Box, 1- Complex , 2- Capsule
  uint32_t layer; // I exist on X Layer

  // Player should be on 1 at all times

  Transform transform; // hold refernces to entity transform if any
  uint32_t mask;       // X layer Doesn't Effect Me

  //
  RaycasterObject *linetrace;
  CapsuleObject *capsule;

} CollisionComponent;

void entity_delete_component_lists();

void entity_remove_component(uint32_t index, EntityComponentType type);

void entity_remove_all_component(uint32_t id);

// awful remember to clear from codebase
void *entity_get_component(uint32_t id, EntityComponentType type);

void kinematics_accelerate(KinematicsComponent *kinematics); // Lateral
void kinematics_fall(KinematicsComponent *kinematics);
void kinematics_velocity(KinematicsComponent *kinematics);

TransformComponent *entity_get_transform(uint32_t id);
KinematicsComponent *entity_get_kinematics(uint32_t id);
ResourceComponent *entity_get_resource(uint32_t id);
CollisionComponent *entity_get_collisions(uint32_t id);
RenderComponent *entity_get_render(uint32_t id);

// Assumes you already have a reference to component
// its for my own ease of access
TransformComponent *entity_get_own_transform(Entity *self);
KinematicsComponent *entity_get_own_kinematics(Entity *self);
ResourceComponent *entity_get_own_resource(Entity *self);
CollisionComponent *entity_get_own_collision(Entity *self);
RenderComponent *entity_get_own_render(Entity *self);

Transform *entity_tranform_get(Entity *self, Transform *transform);

GFC_Vector3D transform_get_position(Entity *self);
GFC_Vector3D transform_get_rotation(Entity *self);
GFC_Vector3D transform_get_scale(Entity *self);

GFC_Box *render_get_bounds(Entity *self);
Model *render_get_model(Entity *self);

void entity_world_collision(Entity *self);

void entity_collision(Entity *self);

// assume center of bounding box is the model origin
//
void collision_obb_construct(GFC_Box *box, Transform *transform, OBB *obb);

GFC_Vector3D collision_transform_vertex(GFC_Vector3D vertex,
                                        Transform *transform);

uint8_t collision_box_box(GFC_Box box1, Transform transform1, GFC_Box box2,
                          Transform transform2, CollisionInfo *info);
uint8_t collision_box_complex(GFC_Box box1, Transform transform1, Model *model,
                              Transform transform2, CollisionInfo *info);
uint8_t collision_box_capsule(GFC_Box box, Transform transform1,
                              CapsuleObject *capsule, Transform transform2,
                              CollisionInfo *info);
uint8_t collision_capsule_capsule(CapsuleObject *capsule1, Transform transform1,
                                  CapsuleObject *capsule2, Transform transform2,
                                  CollisionInfo *info);
uint8_t collision_capsule_complex(CapsuleObject *capsule, Transform transform1,
                                  Model *model, Transform transform2,
                                  CollisionInfo *info);
uint8_t collision_complex_complex(Model *model1, Transform transform1,
                                  Model *model2, Transform transform2,
                                  CollisionInfo *info);

void collision_resolve(Transform *transform, CollisionComponent *collision,
                       const CollisionInfo *info);

void collision_resolve_pair(Transform *transform, CollisionComponent *collision,
                            const CollisionInfo *info);

#endif
