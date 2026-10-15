#ifndef __ENTITY_CORE_H__
#define __ENTITY_CORE_H__

#include "cf_queue.h"
#include "gfc_matrix.h"
#include "gfc_shape.h"
#include "gfc_vector.h"
#include "model.h"

#define ALLCOMPONENTS 5

//  ported from gf2d

typedef struct Entity {
  uint8_t _inuse;
  char id[11];                         // Unique identifier (who is it) its a char because of hashmap
  uint32_t tag;                        // Component Mask
  void (*think)(struct Entity *self);  // Called every frame
  void (*update)(struct Entity *self); // Called on deltatime
  void (*free)(struct Entity *self);
} Entity;

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
  GFC_Vector3D direction;
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
  uint8_t _inuse;
  EntityResourceF *f_data;
  EntityResourceI *i_data;

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

void entity_manager_init(uint32_t max);

void entity_manager_close();

void entity_free(Entity *entity);

Entity *entity_create();

uint8_t entity_is_valid(uint32_t id);

void entity_manager_draw();

void entity_manager_think();

void entity_manager_update();

void entity_draw(uint32_t id);

Entity *entity_get_reference(uint32_t id);

void entity_add_component(uint32_t id, EntityComponentType type, void *data);

void entity_delete_components();

void entity_remove_component(uint32_t id, EntityComponentType type);

void entity_remove_all_components(uint32_t id);

void *entity_get_component(uint32_t id, EntityComponentType type);

#endif
