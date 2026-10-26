#ifndef __ENTITY_COMPONENT_H__
#define __ENTITY_COMPONENT_H__

#include "gfc_vector.h"
#include "entity_core.h"
#include <stdint.h>

#define ALLCOMPONENTS 5
#define DAMPENER 80.0f
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

typedef struct{

    float max;
    float min;
    GFC_Vector2D axis;

} EntityResourceA;

typedef struct {
    // Float integer axis
  EntityResourceF *f_data;
  EntityResourceI *i_data;
  EntityResourceA *a_data;

} ResourceComponent;

typedef struct {

} RaycasterObject;

typedef struct {
  uint8_t _inuse;
  uint32_t layer;      // I exist on X Layer
  uint32_t layer_mask; // X layer Doesn't Effect Me'

  GFC_Shape bounds;
  RaycasterObject *linetrace;

} CollisionComponent;

void entity_delete_component_lists();

void entity_remove_component(uint32_t index, EntityComponentType type);

void entity_remove_all_component(uint32_t id);

// awful remember to clear from codebase
void *entity_get_component(uint32_t id, EntityComponentType type);

void kinematics_accelerate(KinematicsComponent *kinematics,GFC_Vector2D *axis); // Lateral
void kinematics_velocity(KinematicsComponent *kinematics, GFC_Vector2D *axis);
void kinematics_smooth_velocity(KinematicsComponent *kinematics, GFC_Vector2D *axis, float coeff);

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


#endif
