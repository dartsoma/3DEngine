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

void move_left(Entity* self);
void move_right(Entity* self);
void move_forward(Entity* self);
void move_backward(Entity* self);
void setup_resource(ResourceComponent* resource);
void setup_render(RenderComponent* render);
uint8_t setup_collision(CollisionComponent* collision, RenderComponent* render);
void setup_kinematics(KinematicsComponent* kinematics);
void setup_transform(TransformComponent* transform);
Uint8 player_flag_check(Entity* self, int flag);

void dive(Entity* self);

void jump(Entity* self);

void slide(Entity* self);

void brake(Entity* self);

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

float player_get_speed(Entity* self) {
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(self);

    float magnitude = gfc_vector3d_magnitude(kinematics->vel);

    return magnitude;
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
    // Axis
    resource->a_data = gfc_allocate_array(sizeof(EntityResourceA), 3);
    resource->a_data[AXIS_MOVE].min = -1;
    resource->a_data[AXIS_MOVE].max = 1;
    resource->a_data[AXIS_PREVMOVE].min = -1;
    resource->a_data[AXIS_PREVMOVE].max = 1;
    resource->a_data[AXIS_CAMERA].axis.x = 0;
    resource->a_data[AXIS_CAMERA].axis.y = (GFC_PI / 6);

    resource->f_data = gfc_allocate_array(sizeof(EntityResourceF), 10);
    resource->f_data[FLOAT_CAMERADISTANCE].value = 30;

    // setup timers

    resource->f_data[FLOAT_ABILITY_1_TIMER].value = 1;
    resource->f_data[FLOAT_ABILITY_2_TIMER].value = 1;
    resource->f_data[FLOAT_ABILITY_3_TIMER].value = 1;
    resource->f_data[FLOAT_ABILITY_4_TIMER].value = 1;
    resource->f_data[FLOAT_ABILITY_5_TIMER].value = 1;
    resource->f_data[FLOAT_BOOST_TIMER].value = EFFECT_DURATION;
    resource->f_data[FLOAT_SLOW_TIMER].value = EFFECT_DURATION;
    resource->f_data[FLOAT_JUMPBOOST_TIMER].value = EFFECT_DURATION;
    resource->f_data[FLOAT_LOWGRAV_TIMER].value = EFFECT_DURATION;

    resource->i_data = gfc_allocate_array(sizeof(EntityResourceI), 2);
    resource->i_data[INT_PLAYERFLAGS].value = 0;
    resource->i_data[INT_UNLOCKFLAGS].value = 0;

    // Abilities
    resource->i_data[INT_UNLOCKFLAGS].value |= FLAG_ABILITY1;
    resource->i_data[INT_UNLOCKFLAGS].value |= FLAG_ABILITY2;
    resource->i_data[INT_UNLOCKFLAGS].value |= FLAG_ABILITY3;
    resource->i_data[INT_UNLOCKFLAGS].value |= FLAG_ABILITY4;
    resource->i_data[INT_UNLOCKFLAGS].value |= FLAG_ABILITY5;
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

    // jump
    if (player_flag_check(self, FLAG_GROUNDED) && gfc_input_key_held("SPACE")) {
        transform->scale.z = 1; // in the case that char is crouching
        // if (resource->f_data[FLOAT_ABILITY_1_TIMER].value == 0)
        resource->i_data[INT_PLAYERFLAGS].value |= FLAG_JUMPING;
        // resource->f_data[FLOAT_ABILITY_1_TIMER].value = 1;
    }

    if (gfc_input_key_pressed("LCTRL")) {
        resource->i_data[INT_PLAYERFLAGS].value &= FLAG_SLIDING;
        transform->scale.z = 0.5;
    }
    // dive

    if (gfc_input_key_held("LCTRL")) {
        if (player_flag_check(self, FLAG_GROUNDED) == 0) {
            slide(self);
            kinematics->override.x = kinematics->direction.x;
            kinematics->override.y = kinematics->direction.y;
        } else {
            dive(self);
        }
    }

    if (gfc_input_key_released("LCTRL")) {
        resource->i_data[INT_PLAYERFLAGS].value &= ~FLAG_SLIDING;
        transform->scale.z = 1;
        kinematics->override.x = 0;
        kinematics->override.y = 0;
    }

    if (gfc_input_key_held("LSHIFT")) {
        brake(self);
    } else {
        kinematics->brake = 0;
    }

    kinematics->direction = gfc_vector2d(0, 0);

    float yaw = resource->a_data[AXIS_CAMERA].axis.x;
    float forward = resource->a_data[AXIS_MOVE].axis.x;
    float strafe = resource->a_data[AXIS_MOVE].axis.y;

    kinematics->direction.x = forward * sinf(yaw) + strafe * cosf(yaw);
    kinematics->direction.y = -forward * cosf(yaw) + strafe * sinf(yaw);

    if (gfc_vector2d_magnitude(kinematics->override) >= GFC_EPSILON)
        gfc_vector2d_copy(kinematics->direction, kinematics->override);

    transform->rot.z = resource->a_data[AXIS_CAMERA].axis.x;
}

void player_update(Entity* self) {
    if (!self)
        return;
    int i;
    // phys
    TransformComponent* transform = (TransformComponent*)entity_get_own_transform(self);
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(self);
    ResourceComponent* resource = (ResourceComponent*)entity_get_own_resource(self);

    // timer count
    for (int i = FLOAT_ABILITY_1_TIMER; i < 10; i++) {
        if (resource->f_data[i].value > 0)
            resource->f_data[i].value -= 0.016f;
        if (resource->f_data[i].value < 0)
            resource->f_data[i].value = 0;
    }

    // speed effects
    float speed_mult = 1.0f;
    if (resource->f_data[FLOAT_BOOST_TIMER].value > 0)
        speed_mult = BOOST_VEL_MULT;
    if (resource->f_data[FLOAT_SLOW_TIMER].value > 0)
        speed_mult = SLOW_VEL_MULT;
    kinematics->vel_max = BASE_VEL_MAX * speed_mult;

    if (player_flag_check(self, FLAG_GROUNDED)) {
        if (kinematics->vel.z < 0)
            kinematics->vel.z = 0;
        resource->i_data[INT_PLAYERFLAGS].value &= ~FLAG_FASTFALL;

        if (player_flag_check(self, FLAG_JUMPING))
            jump(self);
    }
    resource->i_data[INT_PLAYERFLAGS].value &= ~FLAG_JUMPING;
    float g = FALL_ACC;
    if (player_flag_check(self, FLAG_FASTFALL))
        g *= FASTFALL_MULT;
    if (resource->f_data[FLOAT_LOWGRAV_TIMER].value > 0)
        g *= LOWGRAV_MULT;
    kinematics_fall(kinematics, g);
    kinematics_fall(kinematics, g);
    kinematics_accelerate(kinematics);
    kinematics_velocity(kinematics);

    // collision will re-set this during the steps below
    resource->i_data[INT_PLAYERFLAGS].value &= ~FLAG_GROUNDED;

    // never move more than half a capsule radius per collision check
    CollisionComponent* collision = entity_get_own_collision(self);
    float step_max = 0.5f;
    if (collision && collision->capsule)
        step_max = fmaxf(collision->capsule[0].radius * 0.5f, 0.1f);

    int steps = (int)ceilf(gfc_vector3d_magnitude(kinematics->vel) / step_max);
    if (steps < 1)
        steps = 1;
    if (steps > 16)
        steps = 16;

    for (int i = 0; i < steps; i++) {
        GFC_Vector3D step;
        gfc_vector3d_scale(step, kinematics->vel, 1.0f / steps);
        gfc_vector3d_add(transform->pos, transform->pos, step);
        entity_world_collision(self); // zeroes vel.z when it lands, so later steps stop
    }

    float before = transform->pos.z;
    level_clamp_bounds(&transform->pos);
    if (transform->pos.z < before && kinematics->vel.z > 0)
        kinematics->vel.z = 0;
    // slog("Direction: %f, %f", kinematics->direction.x,
    // kinematics->direction.y);

    // slog("Velocity: %f, %f", kinematics->vel.x, kinematics->vel.y);

    //  slog("Player Update");
    // slog("Position: %f, %f, %f", transform->pos.x, transform->pos.y, transform->pos.z);
    // slog("camera->rot: %f, %f", resource->a_data[AXIS_CAMERA].axis.x,
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

    ResourceComponent* resource = entity_get_own_resource(self);
    if (resource->i_data[INT_UNLOCKFLAGS].value & FLAG_ABILITY2) {
        resource->i_data[INT_PLAYERFLAGS].value |= FLAG_FASTFALL;
    }
}

void jump(Entity* self) {
    if (!self)
        return;
    KinematicsComponent* kinematics = entity_get_own_kinematics(self);
    ResourceComponent* resource = entity_get_own_resource(self);
    if (resource->i_data[INT_UNLOCKFLAGS].value & FLAG_ABILITY1) {
        float speed = JUMP_SPEED;
        if (resource->f_data[FLOAT_JUMPBOOST_TIMER].value > 0)
            speed *= JUMPBOOST_MULT;
        kinematics->vel.z = speed;
    }
}

void slide(Entity* self) {
    TransformComponent* transform = (TransformComponent*)entity_get_own_transform(self);
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(self);
    ResourceComponent* resource = entity_get_own_resource(self);
    if (resource->i_data[INT_UNLOCKFLAGS].value & FLAG_ABILITY3) {
        kinematics->boost = 4;

        resource->i_data[INT_PLAYERFLAGS].value |= FLAG_SLIDING;
    }
}

void brake(Entity* self) {
    KinematicsComponent* kinematics = (KinematicsComponent*)entity_get_own_kinematics(self);
    ResourceComponent* resource = entity_get_own_resource(self);
    if (resource->i_data[INT_UNLOCKFLAGS].value & FLAG_ABILITY3) {
        kinematics->brake = 1;
        resource->i_data[INT_PLAYERFLAGS].value |= FLAG_BRAKING;
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

Uint8 player_flag_check(Entity* self, int flag) {
    if (!self)
        return 0;

    ResourceComponent* resource = entity_get_own_resource(self);
    if ((resource->i_data[INT_PLAYERFLAGS].value & flag) != 0) {
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

void toggle_player_flag(Entity* self, int flag) {
    if (!self)
        return;
    ResourceComponent* resource = entity_get_own_resource(self);

    resource->i_data[INT_PLAYERFLAGS].value ^= flag;
}

void player_apply_effect(Entity* self, int effect) {
    if (!self)
        return;
    ResourceComponent* r = entity_get_own_resource(self);
    KinematicsComponent* k = entity_get_own_kinematics(self);
    if (!r || !k)
        return;

    switch (effect) {
        case EFFECT_BOOST:
            r->f_data[FLOAT_SLOW_TIMER].value = 0;
            r->f_data[FLOAT_BOOST_TIMER].value = EFFECT_DURATION;
            k->boost = 4;
            break;
        case EFFECT_FAKEBOOST:
            r->f_data[FLOAT_BOOST_TIMER].value = 0;
            r->f_data[FLOAT_SLOW_TIMER].value = EFFECT_DURATION;
            break;
        case EFFECT_JUMPBOOST:
            r->f_data[FLOAT_JUMPBOOST_TIMER].value = EFFECT_DURATION;
            break;
        case EFFECT_LOWGRAV:
            r->f_data[FLOAT_LOWGRAV_TIMER].value = EFFECT_DURATION;
            break;
        case EFFECT_ABILITY:
            // unlock the next locked ability (2..5)
            for (int flag = FLAG_ABILITY2; flag <= FLAG_ABILITY5; flag <<= 1) {
                if (!(r->i_data[INT_UNLOCKFLAGS].value & flag)) {
                    r->i_data[INT_UNLOCKFLAGS].value |= flag;
                    slog("Ability unlocked: %d", flag);
                    break;
                }
            }
            break;
    }
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
