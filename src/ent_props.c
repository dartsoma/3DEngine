#include "ent_props.h"
#include "entity_component.h"
#include "entity_core.h"
#include "gfc_types.h"
#include "simple_logger.h"
#include "player.h"
#include <stdint.h>

#define PROP_TAG 0

const char* prop_models[11] = {
    NULL,                    // 0
    "models/map/void.model", // 1
    "models/map/anchor.model",
    "models/map/booster.model",
    "models/map/pile.model",
    "models/map/trash.model",     // 5
    "models/map/boostpack.model", // 6
    "models/map/fakepack.model",
    "models/map/jumppack.model",
    "models/map/abilitypack.model",
    "models/map/gravitypack.model", // 10
};

void prop_try_pickup(Entity* self, int effect);
KinematicsComponent* prop_zone_player(Entity* self);

Entity* prop_create(uint8_t tag) {
    if (tag < 1 || tag > 10) {
        slog("null prop scrapped");
        return NULL;
    }

    Entity* prop = entity_create();
    if (!prop)
        return NULL;

    prop->free = prop_free;
    prop->update = prop_update;
    prop->think = prop_think;
    prop->draw = prop_draw;

    prop->tag |= EntityRender | EntityTransform | EntityCollision | EntityResource;

    RenderComponent* render = entity_get_own_render(prop);
    TransformComponent* transform = entity_get_own_transform(prop);
    ResourceComponent* resource = entity_get_own_resource(prop);
    CollisionComponent* collision = entity_get_own_collision(prop);
    if (!render || !transform || !resource || !collision) {
        slog("prop components failed");
        return NULL;
    }

    transform->scale = gfc_vector3d(1, 1, 1);
    transform->pos = gfc_vector3d(0, 0, 0);

    resource->i_data = gfc_allocate_array(sizeof(EntityResourceI), 1);
    resource->i_data[PROP_TAG].value = tag;

    render->model = model_load(prop_models[tag]);
    if (!render->model) {
        slog("missing model for prop %d, using placeholder", tag);
        render->model = model_load("models/dino/dino.model");
    }

    gfc_matrix4_identity(render->mat);
    render->color = GFC_COLOR_WHITE;

    collision->type = 0;
    collision->layer = 2;
    collision->mask = 0;

    return prop;
}

void prop_anchor(Entity* self) {
    KinematicsComponent* k = prop_zone_player(self);
    if (!k)
        return;
    k->vel.x *= ANCHOR_DRAG;
    k->vel.y *= ANCHOR_DRAG;
    k->vel.z -= ANCHOR_PULL; // kinematics_velocity clamps to terminal velocity
}

void prop_boost(Entity* self) {
    KinematicsComponent* k = prop_zone_player(self);
    if (!k)
        return;

    // one launch per pass: don't re-fire while already flying upward
    if (k->vel.z >= BOOST_LAUNCH_UP * 0.5f)
        return;

    TransformComponent* t = entity_get_own_transform(self);
    float yaw = t->rot.z * GFC_DEGTORAD; // prop rotation from the level file (degrees)

    k->vel.z = BOOST_LAUNCH_UP;
    k->vel.x = cosf(yaw) * BOOST_LAUNCH_FWD;
    k->vel.y = sinf(yaw) * BOOST_LAUNCH_FWD;
}

void prop_pile(Entity* self) {
    KinematicsComponent* k = prop_zone_player(self);
    if (!k)
        return;
    k->vel.x *= PILE_DRAG;
    k->vel.y *= PILE_DRAG;
}

void prop_trash(Entity* self) {
    KinematicsComponent* k = prop_zone_player(self);
    if (!k)
        return;
    k->vel.x = 0;
    k->vel.y = 0;
}

// only for pickups
void prop_try_pickup(Entity* self, int effect) {
    Entity* player = entity_player_reference();
    if (!player)
        return;

    TransformComponent* pt = entity_get_own_transform(player);
    TransformComponent* t = entity_get_own_transform(self);
    if (!pt || !t)
        return;

    GFC_Vector3D diff;
    gfc_vector3d_sub(diff, pt->pos, t->pos);
    if (gfc_vector3d_magnitude(diff) > PROP_PICKUP_RADIUS)
        return;

    player_apply_effect(player, effect);
    entity_free(self);
}

KinematicsComponent* prop_zone_player(Entity* self) {
    Entity* player = entity_player_reference();
    if (!player)
        return NULL;

    TransformComponent* pt = entity_get_own_transform(player);
    TransformComponent* t = entity_get_own_transform(self);
    KinematicsComponent* k = entity_get_own_kinematics(player);
    if (!pt || !t || !k)
        return NULL;

    GFC_Vector3D diff;
    gfc_vector3d_sub(diff, pt->pos, t->pos);
    if (gfc_vector3d_magnitude(diff) > PROP_HIT_RADIUS * t->scale.x)
        return NULL;

    return k;
}

void prop_update(Entity* self) {
    if (!self)
        return;
    ResourceComponent* resource = entity_get_own_resource(self);
    if (!resource)
        return;

    uint8_t tag = (uint8_t)resource->i_data[PROP_TAG].value;

    switch (tag) {
        case 2:
            prop_anchor(self);
            break;
        case 3:
            prop_boost(self);
            break;
        case 4:
            prop_pile(self);
            break;
        case 5:
            prop_trash(self);
            break;
        case 6:
            prop_try_pickup(self, EFFECT_BOOST);
            break;
        case 7:
            prop_try_pickup(self, EFFECT_FAKEBOOST);
            break;
        case 8:
            prop_try_pickup(self, EFFECT_JUMPBOOST);
            break;
        case 9:
            prop_try_pickup(self, EFFECT_ABILITY);
            break;
        case 10:
            prop_try_pickup(self, EFFECT_LOWGRAV);
            break;
        default:
            break;
    }
}

Entity* prop_spawn_at(uint8_t tag, GFC_Vector3D* vec) {
    Entity* prop = prop_create(tag);

    if (!prop)
        return NULL;

    TransformComponent* oldtransform = entity_get_own_transform(prop);

    oldtransform->pos = *vec;
    return prop;
}

void prop_free(Entity* self) {
    if (!self)
        return;
    RenderComponent* render = (RenderComponent*)entity_get_own_render(self);
    TransformComponent* transform = (TransformComponent*)entity_get_own_transform(self);
    ResourceComponent* resource = (ResourceComponent*)entity_get_own_resource(self);
    CollisionComponent* collision = (CollisionComponent*)entity_get_own_collision(self);

    uint8_t tag = (uint8_t)resource->i_data[PROP_TAG].value;

    switch (tag) {
        default:
            break;
    }

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
    memset(collision, 0, sizeof(*collision));

    slog("prop Freed!");
}

void prop_think(Entity* self) {
    if (!self)
        return;

    ResourceComponent* resource = (ResourceComponent*)entity_get_own_resource(self);

    uint8_t tag = (uint8_t)resource->i_data[PROP_TAG].value;

    switch (tag) {
        default:
            break;
    }
}

void prop_draw(Entity* self) {
    if (!self)
        return;

    RenderComponent* render = (RenderComponent*)entity_get_own_render(self);
    TransformComponent* transform = (TransformComponent*)entity_get_own_transform(self);

    gfc_matrix4_from_vectors(render->mat, transform->pos, transform->rot, transform->scale);
    gf3d_model_queue_render(render->model, render->mat, render->color);
    return;
}
