#include "simple_logger.h"
#include "gfc_color.h"
#include "gfc_types.h"
#include "gfc_vector.h"
#include "gfc_input.h"
#include "model.h"
#include "entity_component.h"
#include "entity_core.h"

#include "player.h"

#define AXIS_MOVE 0
#define AXIS_CAM 1

#define DAMPENING 10

void move_left(Entity *self);
void move_right(Entity *self);
void move_forward(Entity *self);
void move_backward(Entity *self);

Entity* player_create() {
    Entity *player;

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

    RenderComponent *render = (RenderComponent*) entity_get_own_render(player);
    TransformComponent *transform = (TransformComponent*) entity_get_own_transform(player);
    KinematicsComponent *kinematics = (KinematicsComponent*) entity_get_own_kinematics(player);
    ResourceComponent *resource = (ResourceComponent*) entity_get_own_resource(player);

    resource->a_data = gfc_allocate_array(sizeof(EntityResourceA), 1);
    resource->a_data->min = -1;
    resource->a_data->max = 1;


    transform->scale = gfc_vector3d(1,1,1);

    // remember to divide by 60
    kinematics->vel_max = 10;
    kinematics->acc_max = 5;

    render->model = model_load("models/dino/dino.model");
    gfc_matrix4_identity(render->mat);
    render->color = GFC_COLOR_WHITE;

    slog("Player Spawned!");
    return player;
}

void player_free(Entity *self){

    if(!self) return;

    RenderComponent *render = (RenderComponent*) entity_get_own_render(self);
    ResourceComponent *resource = (ResourceComponent*) entity_get_own_resource(self);

    free(resource->f_data);
    model_free(render->model);

    slog("Player Freed!");
    return;
}

void player_think(Entity *self){
    if(!self) return;
    // controls

    ResourceComponent *resource = entity_get_own_resource(self);

    resource->a_data[AXIS_MOVE].axis.x = 0;
    resource->a_data[AXIS_MOVE].axis.y = 0;
    if(gfc_input_command_down("item_up")){
        move_forward(self);
    }
    if(gfc_input_command_down("item_down")){
        move_backward(self);
    }
    if(gfc_input_command_down("item_left")){
        move_left(self);
    }
    if(gfc_input_command_down("item_right")){
        move_right(self);
    }

    return;
}

void player_update(Entity *self){
    if(!self) return;

    // phys
    TransformComponent *transform = (TransformComponent*) entity_get_own_transform(self);
    KinematicsComponent *kinematics = (KinematicsComponent*) entity_get_own_kinematics(self);
    ResourceComponent *resource = (ResourceComponent*) entity_get_own_resource(self);


    kinematics_accelerate(kinematics, &resource->a_data[AXIS_MOVE].axis);
    // when i get collisions i will make gravity and a on grounded mechanic
    //
    slog("Transform position: x%f y%f z%f", transform->pos.x, transform->pos.y, transform->pos.z);

    kinematics_velocity(kinematics, &resource->a_data[AXIS_MOVE].axis);
    gfc_vector3d_add(transform->pos, kinematics->vel, transform->pos);
    slog("New Transform position: x%f y%f z%f", transform->pos.x, transform->pos.y, transform->pos.z);
    return;
}

void player_draw(Entity *self){
    if(!self) return;

    RenderComponent *render = (RenderComponent*) entity_get_own_render(self);
    TransformComponent *transform = (TransformComponent*) entity_get_own_transform(self);


    gfc_matrix4_from_vectors(render->mat, transform->pos, transform->rot, transform->scale);
    gf3d_model_queue_render(render->model, render->mat, render->color);
    return;
}


void move_left(Entity *self){
    if(!self) return;

    ResourceComponent *resource = (ResourceComponent*) entity_get_own_resource(self);

    resource->a_data[AXIS_MOVE].axis.x -= 1;
}
void move_right(Entity *self){
    if(!self) return;


    ResourceComponent *resource = (ResourceComponent*) entity_get_own_resource(self);

    resource->a_data[AXIS_MOVE].axis.x += 1;

}
void move_forward(Entity *self){
    if(!self) return;

    ResourceComponent *resource = (ResourceComponent*) entity_get_own_resource(self);

    resource->a_data[AXIS_MOVE].axis.y -= 1;

}

void move_backward(Entity *self){
    if(!self) return;

    ResourceComponent *resource = (ResourceComponent*) entity_get_own_resource(self);

    resource->a_data[AXIS_MOVE].axis.y += 1;

}
