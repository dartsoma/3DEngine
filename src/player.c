#include "camera_controller.h"
#include "entity_component.h"
#include "entity_core.h"
#include "gfc_color.h"
#include "gfc_input.h"
#include "gfc_matrix.h"
#include "gfc_types.h"
#include "gfc_vector.h"
#include "model.h"
#include "simple_logger.h"
#include "level.h"
#include <stdint.h>
#include <stdlib.h>

#include "player.h"

// axis data
#define AXIS_MOVE 0
#define AXIS_PREVMOVE 1
#define AXIS_CAMERA 2

#define FLOAT_CAMERADISTANCE 0
#define FLOAT_ABILITY_1_TIMER 1
#define FLOAT_ABILITY_2_TIMER 2
#define FLOAT_ABILITY_3_TIMER 3
#define FLOAT_ABILITY_4_TIMER 4
#define FLOAT_ABILITY_5_TIMER 5

#define INT_PLAYERFLAGS 0
#define FLAG_GROUNDED 1
#define FLAG_FASTFALL 2
#define FLAG_SLIDING 4
#define FLAG_JUMP_DEBOUNCE 8
#define FLAG_DOUBLEJUMP_DEBOUNCE 16
#define FLAG_BRAKING 32

#define INT_UNLOCKFLAGS 1
#define FLAG_ABILITY1 1
#define FLAG_ABILITY2 2
#define FLAG_ABILITY3 4
#define FLAG_ABILITY4 8
#define FLAG_ABILITY5 16

#define CAPSULE_MAIN 0

void move_left(Entity* self);
void move_right(Entity* self);
void move_forward(Entity* self);
void move_backward(Entity* self);
void setup_resource(ResourceComponent* resource);
void setup_render(RenderComponent* render);
uint8_t setup_collision(CollisionComponent* collision, RenderComponent* render);
void setup_kinematics(KinematicsComponent* kinematics);
void setup_transform(TransformComponent* transform);

Entity* player_create() {
    Entity* player;

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

    RenderComponent* render = (RenderComponent*)entity_get_own_render(player);
    TransformComponent* transform = (TransformComponent*)entity_get_own_transform(player);
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(player);
    ResourceComponent* resource = (ResourceComponent*)entity_get_own_resource(player);
    CollisionComponent* collision = (CollisionComponent*)entity_get_own_collision(player);

    if (!render || !transform || !kinematics || !resource || !collision) {
        slog("player components failed to initialize");
        return NULL;
    }

    setup_resource(resource);
    setup_resource(resource);
    setup_render(render);

    if (setup_collision(collision, render) == 0) {
        return NULL;
    }

    setup_kinematics(kinematics);
    setup_transform(transform);

    camera_set_target(transform);
    camera_set_rotation(&resource->a_data[AXIS_CAMERA].axis);
    camera_set_distance(resource->f_data[FLOAT_CAMERADISTANCE].value);

    slog("Player Spawned!");
    entity_set_player_reference(player);
    return player;
}

void setup_render(RenderComponent* render) {
    render->model = model_load("models/dino/dino.model");

    if (!render->model)

        slog("no model");

    gfc_matrix4_identity(render->mat);
    render->color = GFC_COLOR_WHITE;
}

uint8_t setup_collision(CollisionComponent* collision, RenderComponent* render) {
    collision->type = 2;
    collision->layer = 1;
    collision->mask = 0;

    collision->capsule = (CapsuleObject*)malloc(sizeof(CapsuleObject));
    if (!collision->capsule) {
        slog("Failed to build player casule");
        return 0;
    }
    collision->capsule[0] = entity_capsule_from_bounds(render->model->mesh->bounds);
    return 1;
}

void setup_kinematics(KinematicsComponent* kinematics) {
    kinematics->vel_max = 6;
    kinematics->acc_max = 1;
}

void setup_transform(TransformComponent* transform) {
    transform->scale = gfc_vector3d(1, 1, 1);
    transform->pos = gfc_vector3d(1, 1, 1);
    transform->rot = gfc_vector3d(0, 0, 0);
}

void setup_resource(ResourceComponent* resource) {
    resource->a_data = gfc_allocate_array(sizeof(EntityResourceA), 3);
    resource->a_data[AXIS_MOVE].min = -1;
    resource->a_data[AXIS_MOVE].max = 1;
    resource->a_data[AXIS_PREVMOVE].min = -1;
    resource->a_data[AXIS_PREVMOVE].max = 1;
    resource->a_data[AXIS_CAMERA].axis.x = 0;
    resource->a_data[AXIS_CAMERA].axis.y = (GFC_PI / 6);

    resource->f_data = gfc_allocate_array(sizeof(EntityResourceF), 6);
    resource->f_data[FLOAT_CAMERADISTANCE].value = 30;

    // setup timers
    resource->f_data[FLOAT_ABILITY_1_TIMER].max = 1;
    resource->f_data[FLOAT_ABILITY_2_TIMER].max = 1;
    resource->f_data[FLOAT_ABILITY_3_TIMER].max = 1;
    resource->f_data[FLOAT_ABILITY_4_TIMER].max = 1;
    resource->f_data[FLOAT_ABILITY_5_TIMER].max = 1;

    resource->f_data[FLOAT_ABILITY_1_TIMER].min = 0;
    resource->f_data[FLOAT_ABILITY_2_TIMER].min = 0;
    resource->f_data[FLOAT_ABILITY_3_TIMER].min = 0;
    resource->f_data[FLOAT_ABILITY_4_TIMER].min = 0;
    resource->f_data[FLOAT_ABILITY_5_TIMER].min = 0;

    resource->f_data[FLOAT_ABILITY_1_TIMER].value = 1;
    resource->f_data[FLOAT_ABILITY_2_TIMER].value = 1;
    resource->f_data[FLOAT_ABILITY_3_TIMER].value = 1;
    resource->f_data[FLOAT_ABILITY_4_TIMER].value = 1;
    resource->f_data[FLOAT_ABILITY_5_TIMER].value = 1;

    resource->i_data = gfc_allocate_array(sizeof(EntityResourceI), 2);
    resource->i_data[INT_PLAYERFLAGS].value = 0;
    resource->i_data[INT_UNLOCKFLAGS].value = 0;
}

void player_free(Entity* self) {
    if (!self)
        return;

    RenderComponent* render = (RenderComponent*)entity_get_own_render(self);
    TransformComponent* transform = (TransformComponent*)entity_get_own_transform(self);
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(self);
    ResourceComponent* resource = (ResourceComponent*)entity_get_own_resource(self);
    CollisionComponent* collision = (CollisionComponent*)entity_get_own_collision(self);

    if (resource->f_data)
        free(resource->f_data);
    if (resource->i_data)
        free(resource->i_data);
    if (resource->a_data)
        free(resource->a_data);
    if (resource->v3_data)
        free(resource->v3_data);

    model_free(render->model);

    memset(render, 0, sizeof(*render));
    memset(transform, 0, sizeof(*transform));
    memset(kinematics, 0, sizeof(*kinematics));
    memset(collision, 0, sizeof(*collision));

    slog("Player Freed!");
    return;
}

void player_think(Entity* self) {
    if (!self)
        return;

    ResourceComponent* resource = entity_get_own_resource(self);
    KinematicsComponent* kinematics = entity_get_own_kinematics(self);
    TransformComponent* transform = entity_get_own_transform(self);

    if (!resource || !kinematics || !transform)
        return;

    resource->a_data[AXIS_MOVE].axis = gfc_vector2d(0, 0);

    if (gfc_input_key_down("UP"))
        resource->a_data[AXIS_CAMERA].axis.y -= 0.05f;

    if (gfc_input_key_down("DOWN"))
        resource->a_data[AXIS_CAMERA].axis.y += 0.05f;

    if (gfc_input_key_down("LEFT"))
        resource->a_data[AXIS_CAMERA].axis.x += 0.05f;

    if (gfc_input_key_down("RIGHT"))
        resource->a_data[AXIS_CAMERA].axis.x -= 0.05f;

    if (gfc_input_key_held("w"))
        resource->a_data[AXIS_MOVE].axis.x = 1;

    if (gfc_input_key_held("s"))
        resource->a_data[AXIS_MOVE].axis.x = -1;

    if (gfc_input_key_held("d"))
        resource->a_data[AXIS_MOVE].axis.y = -1;

    if (gfc_input_key_held("a"))
        resource->a_data[AXIS_MOVE].axis.y = 1;

    kinematics->direction = gfc_vector2d(0, 0);

    float yaw = resource->a_data[AXIS_CAMERA].axis.x;
    float forward = resource->a_data[AXIS_MOVE].axis.x;
    float strafe = resource->a_data[AXIS_MOVE].axis.y;

    kinematics->direction.x = forward * sinf(yaw) + strafe * cosf(yaw);
    kinematics->direction.y = -forward * cosf(yaw) + strafe * sinf(yaw);

    transform->rot.z = resource->a_data[AXIS_CAMERA].axis.x;
}

void player_update(Entity* self) {
    if (!self)
        return;

    // phys
    TransformComponent* transform = (TransformComponent*)entity_get_own_transform(self);
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(self);
    ResourceComponent* resource = (ResourceComponent*)entity_get_own_resource(self);

    kinematics_accelerate(kinematics);
    kinematics_velocity(kinematics);
    gfc_vector3d_add(transform->pos, kinematics->vel, transform->pos);

    level_clamp_bounds(&transform->pos);
    // slog("Direction: %f, %f", kinematics->direction.x,
    // kinematics->direction.y);

    // slog("Velocity: %f, %f", kinematics->vel.x, kinematics->vel.y);

    //  slog("Player Update");
    //  slog("Position: %f, %f, %f", transform->pos.x, transform->pos.y,
    //      transform->pos.z);
    //// slog("camera->rot: %f, %f", resource->a_data[AXIS_CAMERA].axis.x,
    //  resource->a_data[AXIS_CAMERA].axis.y);
    return;
}

void player_draw(Entity* self) {
    if (!self)
        return;

    RenderComponent* render = (RenderComponent*)entity_get_own_render(self);
    TransformComponent* transform = (TransformComponent*)entity_get_own_transform(self);

    gfc_matrix4_from_vectors(render->mat, transform->pos, transform->rot, transform->scale);
    gf3d_model_queue_render(render->model, render->mat, render->color);
    return;
}

void dive(Entity* self) {
    if (!self)
        return;
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(self);
    ResourceComponent* resource = entity_get_own_resource(self);
    if ((resource->i_data[INT_PLAYERFLAGS].value & FLAG_GROUNDED) == 0) {
        resource->i_data[INT_PLAYERFLAGS].value |= FLAG_FASTFALL;
    }
}

void jump(Entity* self) {
    if (!self)
        return;
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(self);
    ResourceComponent* resource = entity_get_own_resource(self);
    if ((resource->i_data[INT_PLAYERFLAGS].value & FLAG_GROUNDED) == 1) {
        kinematics->vel.z = 18;
    } else
        // air njump
        if ((resource->i_data[INT_PLAYERFLAGS].value & FLAG_GROUNDED) == 1 &&
            (player_ability_check(self, FLAG_ABILITY1)) != 0) {
            kinematics->vel.z = 18;
        }
}

void slide(Entity* self) {
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(self);
    ResourceComponent* resource = entity_get_own_resource(self);
    if (kinematics->acc.z < 0 && (resource->i_data[INT_PLAYERFLAGS].value & FLAG_GROUNDED) == 0) {
        if (kinematics->boost <= 0) {
            kinematics->boost = 4;
        }
    }
}

void brake(Entity* self) {
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(self);
    ResourceComponent* resource = entity_get_own_resource(self);
    if (kinematics->acc.z < 0 && (resource->i_data[INT_PLAYERFLAGS].value & FLAG_GROUNDED) == 0) {
    }
}

Uint8 player_ability_check(Entity* self, int flag) {
    if (!self)
        return 0;

    ResourceComponent* resource = entity_get_own_resource(self);
    if ((resource->i_data[INT_UNLOCKFLAGS].value & flag) != 0) {
        return 1;
    }
    return 0;
}

void toggle_ability_flag(Entity* self, int flag) {
    if (!self)
        return;
    ResourceComponent* resource = entity_get_own_resource(self);

    resource->i_data[INT_UNLOCKFLAGS].value ^= flag;
}

/*
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
*/
